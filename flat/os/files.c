#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// STUDENT: put your file-level functions here
file_descriptor fd[FILE_MAX_OPEN_FILES];
static lock_t fd_lock;

//-----------------------------------------------------------------
// FileModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

void FileModuleInit() {
  int i;

  fd_lock = LockCreate();

  LockHandleAcquire(fd_lock);
  for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    fd[i].inuse = 0;
  }
  LockHandleRelease(fd_lock);

}


//-----------------------------------------------------------------
// FileOpen open the given filename with one of two possible modes: 
// "r", "w". If opening the file in "w" mode, and the file already
// exists, the inode should first be deleted and then reopened.
// Return FILE_FAIL on failure, and the handle of a file 
// descriptor on success. Remember to use locks whenever you
// allocate a new file descriptor. You can use dstrncmp function 
// (misc.c) to compare strings.
//-----------------------------------------------------------------

uint32 FileOpen(char *filename, char *mode) {
  int i;
  uint32 inode_handle;
  uint32 fd_handle;

  if (mode[0] != 'r' && mode[0] != 'w') {
    printf("FileOpen (%d): ERROR - Invalid file mode %s! File mode needs to be r or w\n", GetCurrentPid(), mode);
    return FILE_FAIL;
  }

  // 1. inode-related check
  inode_handle = DfsInodeFilenameExists(filename);
  if (inode_handle != DFS_FAIL) { // file exists
    for (i = 0; i < FILE_MAX_OPEN_FILES; i++) { // check whether this inode is used by other fd
      LockHandleAcquire(fd_lock);
      if ((fd[i].inuse == 1) && (fd[i].inode_handle == inode_handle)) {
        printf("FileOpen (%d): ERROR - inode %d is used by another fd!\n", GetCurrentPid(), inode_handle);
        LockHandleRelease(fd_lock);
        return FILE_FAIL;
      }
      LockHandleRelease(fd_lock);
    }

    if (mode[0] == 'w') { 
      // existed file in write mode
      DfsInodeDelete(inode_handle);
      inode_handle = DfsInodeOpen(filename);
    }
    
  } else { // file does NOT exist
    if (mode[0] == 'r') {
      printf("FileOpen (%d): ERROR - File %s does not exist!\n", GetCurrentPid(), filename);
      return FILE_FAIL;
    }

    inode_handle = DfsInodeOpen(filename);
  }

  // 2. Find an available fd
  fd_handle = FILE_MAX_OPEN_FILES;
  for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    LockHandleAcquire(fd_lock);
    if (fd[i].inuse == 0) {
      fd[i].inuse = 1;
      fd_handle = i;
      i = FILE_MAX_OPEN_FILES;
    }
    LockHandleRelease(fd_lock);
  }

  if (fd_handle == FILE_MAX_OPEN_FILES) {
    printf("FileOpen (%d): ERROR - No more available file descriptor. The number of opened file hits the limit.\n", GetCurrentPid());
    return FILE_FAIL;
  }

  // 3. Configure the fd
  LockHandleAcquire(fd_lock);
  dstrncpy(fd[fd_handle].filename, filename, FILE_MAX_FILENAME_LENGTH);
  fd[fd_handle].inode_handle = inode_handle;
  fd[fd_handle].eof = 0;
  dstrncpy(fd[fd_handle].mode, mode, 5);
  fd[fd_handle].curr_pos = 0;
  fd[fd_handle].pid = GetCurrentPid();
  LockHandleRelease(fd_lock);

  return fd_handle;
}

int FileClose(uint32 handle) {
  LockHandleAcquire(fd_lock);
  fd[handle].inuse = 0;
  LockHandleRelease(fd_lock);

  return FILE_SUCCESS;
}

//-----------------------------------------------------------------
// FileRead reads num_bytes from the open file descriptor
// identified by handle. Return FILE_FAIL on failure or if the 
// end-of-file flag is already set, and the number of bytes read
// on success. If end of file is reached, the end-of-file flag in
// the file descriptor should be set.
//-----------------------------------------------------------------

int FileRead(uint32 handle, void *mem, int num_bytes) {
  int total_bytes_read;
  int eof;
  uint32 inode_handle;
  uint32 curr_pos;
  uint32 filesize;

  // 1. Exception handling
  LockHandleAcquire(fd_lock);
  if (fd[handle].inuse == 0) {
    printf("FileRead (%d): ERROR - Fd %d has not been opened.\n", GetCurrentPid(), handle);
    LockHandleRelease(fd_lock);
    return FILE_FAIL;
  } else if (fd[handle].eof == 1) {
    printf("FileRead (%d): ERROR - Fd %d has reached EoF.\n", GetCurrentPid(), handle);
    LockHandleRelease(fd_lock);
    return FILE_FAIL;
  } else if (fd[handle].mode[0] != 'r') {
    printf("FileRead (%d): ERROR - Fd %d is NOT in read mode.\n", GetCurrentPid(), handle);
    LockHandleRelease(fd_lock);
    return FILE_FAIL;
  }
  LockHandleRelease(fd_lock);

  // 2. Start to read
  LockHandleAcquire(fd_lock);
  inode_handle = fd[handle].inode_handle;
  curr_pos = fd[handle].curr_pos;
  LockHandleRelease(fd_lock);

  filesize = DfsInodeFilesize(inode_handle);
  if ((curr_pos + num_bytes) >= filesize) { // read out-of-bound
    num_bytes = filesize - curr_pos;
    eof = 1;
  } else {
    eof = 0;
  }

  total_bytes_read = DfsInodeReadBytes(inode_handle, mem, curr_pos, num_bytes);
  if (total_bytes_read == DFS_FAIL) {
    printf("FileRead (%d): ERROR - DfsInodeReadBytes Failed to read.\n", GetCurrentPid());
    return FILE_FAIL;
  }
  
  // 3. Update fd info
  LockHandleAcquire(fd_lock);
  fd[handle].curr_pos = curr_pos + total_bytes_read;
  fd[handle].eof = eof;
  LockHandleRelease(fd_lock);

  return total_bytes_read;
}

//-----------------------------------------------------------------
// FileWrite write num_bytes to the open file descriptor identified
// by handle. If the file is opened with mode="r", then return
// failure. Return FILE_FAIL on failure, and the number of bytes
// written on success.
//-----------------------------------------------------------------

int FileWrite(uint32 handle, void *mem, int num_bytes) {
  int total_bytes_written;
  uint32 inode_handle;
  uint32 curr_pos;

  LockHandleAcquire(fd_lock);
  if (fd[handle].inuse = 0) {
    printf("FileWrite (%d): ERROR - Input fd %d is not in use.\n", GetCurrentPid(), handle);
    LockHandleRelease(fd_lock);
    return FILE_FAIL;
  } else if (fd[handle].mode[0] != 'w') {
    printf("FileRead (%d): ERROR - Fd %d is NOT in write mode.\n", GetCurrentPid(), handle);
    LockHandleRelease(fd_lock);
    return FILE_FAIL;
  }
  LockHandleRelease(fd_lock);

  // Start to write
  LockHandleAcquire(fd_lock);
  inode_handle = fd[handle].inode_handle;
  curr_pos = fd[handle].curr_pos;
  LockHandleRelease(fd_lock);

  total_bytes_written = DfsInodeWriteBytes(handle, mem, curr_pos, num_bytes);
  if (total_bytes_written == DFS_FAIL) {
    printf("FileRead (%d): ERROR - DfsInodeWriteBytes failed to write bytes.\n", GetCurrentPid(), handle);
    return FILE_FAIL;
  }

  // Shift current position
  LockHandleAcquire(fd_lock);
  fd[handle].curr_pos = curr_pos + total_bytes_written;
  LockHandleRelease(fd_lock);

  return total_bytes_written;
}

int FileSeek(uint32 handle, int num_bytes, int from_where) {
  LockHandleAcquire(fd_lock);
  if (fd[handle].inuse = 0) {
    printf("FileSeek (%d): ERROR - Input fd %d is not in use.\n", GetCurrentPid(), handle);
    LockHandleRelease(fd_lock);
    return FILE_FAIL;
  }

  if (from_where == FILE_SEEK_SET) {
    fd[handle].curr_pos = num_bytes;
  } else if (from_where == FILE_SEEK_END) {
    fd[handle].curr_pos = DfsInodeFilesize(fd[handle].inode_handle) + num_bytes;
  } else if (from_where == FILE_SEEK_CUR) {
    fd[handle].curr_pos += num_bytes;
  } else {
    printf("FileSeek (%d): ERROR - Input from_where %d is not valid.\n", GetCurrentPid(), from_where);
    LockHandleRelease(fd_lock);
    return FILE_FAIL;
  }

  fd[handle].eof = 0;

  LockHandleRelease(fd_lock);
  return FILE_SUCCESS;
}

int FileDelete(char *filename) {
  int i;
  uint32 inode_handle;

  // remove it from fd
  for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    if (fd[i].inuse == 1 && DfsCompareTwoString(fd[i].filename, filename) == DFS_SUCCESS) {
      fd[i].inuse = 0;
      bzero(fd[i].filename, FILE_MAX_FILENAME_LENGTH);
    }
  }

  inode_handle = DfsInodeFilenameExists(filename);
  if (inode_handle == DFS_FAIL) {
    printf("FileDelete (%d): ERROR - filename %s does NOT exist in inodes.\n", GetCurrentPid(), filename);
    return FILE_FAIL;
  } else {
    if (DfsInodeDelete(inode_handle) == DFS_FAIL) {
      printf("FileDelete (%d): ERROR - DfsInodeDelete failed to delete inode[%d].\n", GetCurrentPid(), inode_handle);
      return FILE_FAIL;
    }
  }

  return FILE_SUCCESS;
}
