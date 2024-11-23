#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

static dfs_inode inodes[DFS_INODE_MAX_VIRTUAL_BLOCKNUM]; // all inodes
static dfs_superblock sb; // superblock
static uint32 fbv[DFS_FBV_MAX_NUM_WORDS]; // Free block vector

static lock_t inode_lock;
static lock_t fbv_lock;

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your DFS level functions below.
// Some skeletons are provided. You can implement additional functions.


//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

void DfsModuleInit() {
  inode_lock = LockCreate();
  fbv_lock = LockCreate();

  // You essentially set the file system as invalid and then open 
  // using DfsOpenFileSystem().
	sb.valid = 0;
	DfsOpenFileSystem();

  // later initialize buffer cache-here.

}

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------

void DfsInvalidate() {
// This is just a one-line function which sets the valid bit of the 
// superblock to 0.
  sb.valid = 0;
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------

int DfsOpenFileSystem() {
  int i;
  disk_block dblk;
  dfs_block  fblk;
  // int num_fbv_fblks;

  //Basic steps:
  // 1) Check that filesystem is not already open
  if (sb.valid == 1) {
    printf("DfsOpenFileSystem (%d): ERROR - filesystem has been opened already\n", GetCurrentPid());
    return DFS_FAIL;
  }

  // 2) Read superblock from disk.  Note this is using the disk read rather 
  // than the DFS read function because the DFS read requires a valid 
  // filesystem in memory already, and the filesystem cannot be valid 
  // until we read the superblock. Also, we don't know the block size 
  // until we read the superblock, either. Use (DiskReadBlock function).
  if (DiskReadBlock(1, &dblk) == DISK_FAIL) {
    printf("DfsOpenFileSystem (%d): ERROR - DiskReadBlock failed to read\n", GetCurrentPid());
    return DFS_FAIL;
  }

  // 3) Copy the data from the block we just read into the superblock in
  // memory. Use bcopy
  bcopy((char*) &dblk, (char*) &sb, sizeof(sb));

  // 4) All other blocks are sized by virtual block size:
  // 4.1) Read inodes (Use DfsReadContiguousBytes)
  // 4.2) Read free block vector
  DfsReadContiguousBytes(sb.inode_fs_blk_idx, (char*)&inodes, sizeof(inodes));
  DfsReadContiguousBytes(sb.fbv_fs_blk_idx, (char*)&fbv, sizeof(fbv));

  // num_fbv_fblks = (sb.total_num_fs_block / DFS_BITS_PER_BYTE) / sb.fs_blk_size;
  // for (i = 0; i < num_fbv_fblks; i++) {
  //   DfsReadBlock(i+sb.fbv_fs_blk_idx, &fblk);
  //   bcopy((char*)&fblk, ((char*)fbv)+(i*sb.fs_blk_size), sb.fs_blk_size);
  // }

  // 5) (5.1) Change superblock to be invalid, (5.2) write back to disk, then
  // (5.3) change it back to be valid in memory (mysuperblock.valid = VALID)
  sb.valid = 0;
  bzero((char*) &dblk, sb.disk_blk_size);
  bcopy((char*) &sb, (char*) &dblk, sizeof(sb));
  if (DiskWriteBlock(1, &dblk) == DISK_FAIL) {
    printf("DfsOpenFileSystem (%d): ERROR - DiskWriteBlock failed to write\n", GetCurrentPid());
    return DFS_FAIL;
  }
  sb.valid = 1;

  return DFS_SUCCESS;
}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {
  int i;
  disk_block dblk;
  dfs_block  fblk;
  int num_inode_fblks, num_fbv_fblks;

  if (sb.valid == 0) {
    printf("DfsCloseFileSystem (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  // Write inode to disk
  num_inode_fblks = sb.num_inodes * 128 / sb.fs_blk_size;
  for (i = 0; i < num_inode_fblks; i++) {
    bcopy(((char*)inodes)+(i*sb.fs_blk_size), fblk.data, sb.fs_blk_size);
    DfsWriteBlock(i+sb.inode_fs_blk_idx, &fblk);
  }

  // Write fbv to disk
  num_fbv_fblks = (sb.total_num_fs_block / DFS_BITS_PER_BYTE) / sb.fs_blk_size;
  for (i = 0; i < num_fbv_fblks; i++) {
    bcopy(((char*)fbv)+(i*sb.fs_blk_size), fblk.data, sb.fs_blk_size);
    DfsWriteBlock(i+sb.fbv_fs_blk_idx, &fblk);
  }

  // Write superblock with set valid bit to disk
  sb.valid = 1;
  bzero((char*) dblk.data, sb.disk_blk_size);
  bcopy((char*) &sb, dblk.data, sizeof(sb));
  if (DiskWriteBlock(1, &dblk) == DISK_FAIL) {
    printf("DfsOpenFileSystem (%d): ERROR - DiskWriteBlock failed to write\n", GetCurrentPid());
    return DFS_FAIL;
  }

  // Invalidate memory copy of superblock
  DfsInvalidate();

  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

uint32 DfsAllocateBlock() {
  // Check that file system has been validly loaded into memory
  // Find the first free block using the free block vector (FBV), mark it in use
  // Return handle to block
  int i, j;

  if (sb.valid == 0) {
    printf("DfsAllocateBlock (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  for (i = 0; i < DFS_FBV_MAX_NUM_WORDS; i++) {
    for (j = 0; j < 32; j++) {
      LockHandleAcquire(fbv_lock);
      if ((j == 0) && (fbv[i] == 0xffffffff)) {
        LockHandleRelease(fbv_lock);
        break;
      }

      if ((fbv[i] & (1 << j)) == 0) {
        fbv[i] |= (1 << j);
        LockHandleRelease(fbv_lock);
        return (i*32 + j);
      }

      LockHandleRelease(fbv_lock);
    }
  }

  printf("DfsAllocateBlock (%d): ERROR - Failed to find an available DFS block in FBV\n", GetCurrentPid());
  return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(uint32 blocknum) {
  int i, j;
  if (sb.valid == 0) {
    printf("DfsFreeBlock (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  i = blocknum / 32;
  j = blocknum % 32;

  LockHandleAcquire(fbv_lock);
  fbv[i] &= invert(1 << j);
  LockHandleRelease(fbv_lock);

  return DFS_SUCCESS;
}

//-----------------------------------------------------------------
// DfsReadContiguousBytes reads contiguous bytes from the disk
// (which could span multiple physical disk blocks). The block
// needs not to be allocated, it is used when FBV is not ready.  
// Returns DFS_FAIL on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadContiguousBytes(int start_blocknum, char *dest, int num_bytes) {
  int i;
  int start_dblk, num_dblk;
  int ratio;
  disk_block dblk;

  ratio = sb.fs_blk_size / sb.disk_blk_size;
  start_dblk = start_blocknum * ratio;
  num_dblk = num_bytes / sb.disk_blk_size;

  for (i = 0; i < num_dblk; i++) {
    if (DiskReadBlock(i+start_dblk, &dblk) == DISK_FAIL) {
      printf("DfsReadContiguousBytes (%d): ERROR - DiskReadBlock failed to read\n", GetCurrentPid());
      return DFS_FAIL;
    }
    bcopy(dblk.data, dest+(i*sb.disk_blk_size), sb.disk_blk_size);
  }

  return num_bytes;
}



//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {
  int i;
  disk_block dblk;
  int ratio = sb.fs_blk_size/sb.disk_blk_size;

  // check whether this dfs block is allocated
  if ((fbv[blocknum/32] & (1 << blocknum%32)) == 0) {
    printf("[DBG] fbv[%d] = %d\n", blocknum/32, fbv[blocknum/32]);
    printf("DfsReadBlock (%d): ERROR -  Input DFS block num %d has not been allocated\n", GetCurrentPid(), blocknum);
    return DFS_FAIL;
  }

  for (i = 0; i < ratio; i += 1) {
    if (DiskReadBlock(blocknum*ratio+i, &dblk) == DISK_FAIL) {
      printf("DfsReadBlock (%d): ERROR -  DiskReadBlock failed\n", GetCurrentPid());
      return DFS_FAIL;
    }
    bcopy((char*) &dblk, ((char*) b)+(i*sb.disk_blk_size), sb.disk_blk_size);
  }

  return sb.fs_blk_size;
}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b){
  int i;
  disk_block dblk;
  int ratio = sb.fs_blk_size/sb.disk_blk_size;

  // check whether this dfs block is allocated
  if ((fbv[blocknum/32] & (1 << blocknum%32)) == 0) {
    printf("DfsWriteBlock (%d): ERROR -  Input DFS indexed block has not been allocated\n", GetCurrentPid());
    return DFS_FAIL;
  }

  for (i = 0; i < ratio; i += 1) {
    bcopy(((char*) b)+(i*sb.disk_blk_size), (char*) &dblk, sb.disk_blk_size);
    if (DiskWriteBlock(blocknum*ratio+i, &dblk) == DISK_FAIL) {
      printf("DfsWriteBlock (%d): ERROR -  DiskWriteBlock failed\n", GetCurrentPid());
      return DFS_FAIL;
    } 
  }

  return sb.fs_blk_size;
}


////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------
// CompareTwoString compares 2 char strings. It returns DFS_SUCCESS  
// if they are identical, returns DFS_FAIL if they are not.
//-----------------------------------------------------------------

int DfsCompareTwoString(char* str1, char* str2) {
  int i;

  i = 0;
  while (str1[i] == str2[i]) {
    if (str1[i] == '\0') {
      return DFS_SUCCESS;
    }
    i++;
  }

  return DFS_FAIL;
}

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------

uint32 DfsInodeFilenameExists(char *filename) {
  int i;

  if (sb.valid == 0) {
    printf("DfsInodeFilenameExists (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  for (i = 0; i < DFS_INODE_MAX_VIRTUAL_BLOCKNUM; i++) {
    LockHandleAcquire(inode_lock);
    if ((inodes[i].inuse == 1) && (DfsCompareTwoString(inodes[i].fname, filename) == DFS_SUCCESS)) {
      LockHandleRelease(inode_lock);
      return (uint32) i;
    }
    LockHandleRelease(inode_lock);
  }

  // printf("DfsInodeFilenameExists (%d): Input filename %s does not exist\n", GetCurrentPid(), filename);
  return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

uint32 DfsInodeOpen(char *filename) {
  int i;
  uint32 inode_idx;

  if (sb.valid == 0) {
    printf("DfsInodeOpen (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  inode_idx = DfsInodeFilenameExists(filename);

  // return handle if the filename exists
  if (inode_idx != DFS_FAIL)
    return inode_idx;

  // find available inode
  for (i = 0; i < DFS_INODE_MAX_VIRTUAL_BLOCKNUM; i++) {
    LockHandleAcquire(inode_lock);
    if (inodes[i].inuse == 0) {
      dstrncpy(inodes[i].fname, filename, sizeof(inodes[i].fname));
      inodes[i].inuse = 1;
      LockHandleRelease(inode_lock);
      return i;
    }
    LockHandleRelease(inode_lock);
  }

  printf("DfsInodeOpen (%d): ERROR - Failed to allocate an inode\n", GetCurrentPid());
  return DFS_FAIL;
}





//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {
  int i, j;
  dfs_block fblk_lvl1, fblk_lvl2;
  uint32 *blknum_lvl1_ptr, *blknum_lvl2_ptr;

  if (sb.valid == 0) {
    printf("DfsInodeDelete (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  LockHandleAcquire(inode_lock);
  if (inodes[handle].inuse == 0) {
    printf("DfsInodeDelete (%d): ERROR - This node is currently not in use\n", GetCurrentPid());
    LockHandleRelease(inode_lock);
    return DFS_FAIL;
  } else {
    // Free direct blocks
    for (i = 0; i < DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS; i++) {
      if (inodes[handle].direct[i] != 0) {
        DfsFreeBlock(inodes[handle].direct[i]);
      }
    }

    // Free indirect blocks
    if (inodes[handle].indirect != 0) {
      DfsReadBlock(inodes[handle].indirect, &fblk_lvl1);
      blknum_lvl1_ptr = (uint32*) fblk_lvl1.data;
      for (i = 0; i < (sb.fs_blk_size/4); i++) {
        if (*(blknum_lvl1_ptr+i) != 0) {
          DfsFreeBlock(*(blknum_lvl1_ptr+i));
        }
      }
    }

    // Free double-indirect blocks
    if (inodes[handle].double_indirect != 0) {
      DfsReadBlock(inodes[handle].double_indirect, &fblk_lvl1); // 1st lvl address blk
      blknum_lvl1_ptr = (uint32*) fblk_lvl1.data;
      for (i = 0; i < (sb.fs_blk_size/4); i++) {
        if (*(blknum_lvl1_ptr+i) != 0) {
          DfsReadBlock(*(blknum_lvl1_ptr+i), &fblk_lvl2); // 2nd lvl address blk
          blknum_lvl2_ptr = (uint32*) fblk_lvl2.data;
          for (j = 0; j < (sb.fs_blk_size/4); j++) {
            if (*(blknum_lvl2_ptr+j) != 0) {
              DfsFreeBlock(*(blknum_lvl2_ptr+j));
            }
          }
        }
      }
    }

    // Clear the inode
    bzero((char*)&inodes[handle], sizeof(inodes[handle]));
  }
  LockHandleRelease(inode_lock);

  return DFS_SUCCESS;

}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  int i;
  uint32 start_blk, end_blk; // virtual blk index
  uint32 start_offset, end_offset;
  uint32 fsblknum;
  uint32 curr_total_bytes;
  dfs_block fsblk;

  if (sb.valid == 0) {
    printf("DfsInodeReadBytes (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  start_blk = start_byte / sb.fs_blk_size;
  start_offset = start_byte % sb.fs_blk_size;
  end_blk = (start_byte+num_bytes) / sb.fs_blk_size;
  end_offset = (start_byte+num_bytes) % sb.fs_blk_size;

  if (end_offset != 0) {
    end_blk++;
  }

  curr_total_bytes = 0;
  for (i = start_blk; i < end_blk; i++) {
    fsblknum = DfsInodeTranslateVirtualToFilesys(handle, start_blk+i);
    DfsReadBlock(fsblknum, &fsblk);
    
    if (i == start_blk) {
      bcopy(&fsblk.data[start_offset], (char*) mem, (sb.fs_blk_size - start_offset));
      curr_total_bytes += sb.fs_blk_size - start_offset;

    } else if (i == (end_blk-1)) {
      if (end_offset == 0) {
        end_offset = sb.fs_blk_size;
      }
      bcopy(fsblk.data, ((char*)mem) + curr_total_bytes, end_offset);
      curr_total_bytes += end_offset;

    } else {
      bcopy(fsblk.data, ((char*)mem) + curr_total_bytes, sb.fs_blk_size);
      curr_total_bytes += sb.fs_blk_size;

    }
  }

  if (curr_total_bytes != num_bytes) {
    printf("DfsInodeReadBytes (%d): ERROR - Requested %d bytes but actually read %d bytes\n", GetCurrentPid(), num_bytes, curr_total_bytes);
    return DFS_FAIL;
  }
  return curr_total_bytes;
}


//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------

int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  int i;
  uint32 start_blk, end_blk; // virtual blk index
  uint32 start_offset, end_offset;
  uint32 fsblknum;
  uint32 curr_total_bytes;
  dfs_block fsblk;

  if (sb.valid == 0) {
    printf("DfsInodeWriteBytes (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  start_blk = start_byte / sb.fs_blk_size;
  start_offset = start_byte % sb.fs_blk_size;
  end_blk = (start_byte+num_bytes) / sb.fs_blk_size;
  end_offset = (start_byte+num_bytes) % sb.fs_blk_size;
  
  if (end_offset != 0) {
    end_blk++;
  }

  curr_total_bytes = 0;
  for (i = start_blk; i < end_blk; i++) {
    
    fsblknum = DfsInodeTranslateVirtualToFilesys(handle, i);
    if (fsblknum == DFS_FAIL) { 
      fsblknum = DfsInodeAllocateVirtualBlock(handle, i);
    }
    
    if (i == start_blk) {
      if (start_offset != 0) {
        DfsReadBlock(fsblknum, &fsblk);
      }
      bcopy((char*) mem, &fsblk.data[start_offset], (sb.fs_blk_size - start_offset));
      curr_total_bytes += sb.fs_blk_size - start_offset;

    } else if (i == (end_blk-1)) {
      if (end_offset != 0) {
        DfsReadBlock(fsblknum, &fsblk);
      } else {
        end_offset = sb.fs_blk_size;
      }
      bcopy(((char*)mem) + curr_total_bytes, fsblk.data, end_offset);
      curr_total_bytes += end_offset;

    } else {
      bcopy(((char*)mem) + curr_total_bytes, fsblk.data, sb.fs_blk_size);
      curr_total_bytes += sb.fs_blk_size;

    }

    DfsWriteBlock(fsblknum, &fsblk);
  }

  if (curr_total_bytes != num_bytes) {
    printf("DfsInodeWriteBytes (%d): ERROR - Requested %d bytes but actually write %d bytes\n", GetCurrentPid(), num_bytes, curr_total_bytes);
    return DFS_FAIL;
  }

  LockHandleAcquire(inode_lock);
  inodes[handle].file_size += curr_total_bytes; // DBG
  LockHandleRelease(inode_lock);  
  return curr_total_bytes;
}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeFilesize(uint32 handle) {
  int filesize;

  if (sb.valid == 0) {
    printf("DfsInodeFilesize (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  LockHandleAcquire(inode_lock);
  filesize = inodes[handle].file_size;
  LockHandleRelease(inode_lock);

  return filesize;
}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------

uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {
  uint32 fs_blknum;
  dfs_block fsblk_lvl1, fsblk_lvl2;
  uint32 *ptr_lvl1, *ptr_lvl2;
  uint32 offset_lvl1, offset_lvl2;


  if (sb.valid == 0) {
    printf("DfsInodeAllocateVirtualBlock (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  if (virtual_blocknum >= DFS_INODE_MAX_VIRTUAL_BLOCKNUM) {
    printf("DfsInodeAllocateVirtualBlock (%d): ERROR - Virtual block number %d is out-of-bound.\n", GetCurrentPid(), virtual_blocknum);
    printf("DfsInodeAllocateVirtualBlock (%d): INFO - Max virtual block number: %d.\n", GetCurrentPid(), DFS_INODE_MAX_VIRTUAL_BLOCKNUM);
    return DFS_FAIL;
  }

  LockHandleAcquire(inode_lock);
  if (virtual_blocknum < DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS) {
    // direct
    fs_blknum = DfsAllocateBlock();
    inodes[handle].direct[virtual_blocknum] = fs_blknum;

  } else if (virtual_blocknum < (DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS + (sb.fs_blk_size/4))) {
    // indirect
    if (inodes[handle].indirect == 0) { // indirect blk allocation needed
      fs_blknum = DfsAllocateBlock();
      inodes[handle].indirect = fs_blknum;
    }

    DfsReadBlock(inodes[handle].indirect, &fsblk_lvl1);
    ptr_lvl1 = (uint32*) fsblk_lvl1.data;
    offset_lvl1 = (virtual_blocknum-DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS) % (sb.fs_blk_size/4);
    fs_blknum = DfsAllocateBlock();
    ptr_lvl1[offset_lvl1] = fs_blknum;
    DfsWriteBlock(inodes[handle].indirect, &fsblk_lvl1);

  } else {
    // double indirect - assume MAX virtual block number is within the range of double indirect
    if (inodes[handle].double_indirect == 0) { // double indirect lvl1 blk allocation needed
      fs_blknum = DfsAllocateBlock();
      inodes[handle].double_indirect = fs_blknum;
    }

    DfsReadBlock(inodes[handle].double_indirect, &fsblk_lvl1);
    ptr_lvl1 = (uint32*) fsblk_lvl1.data;
    offset_lvl1 = (virtual_blocknum-DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS-(sb.fs_blk_size/4)) / (sb.fs_blk_size/4);

    if (ptr_lvl1[offset_lvl1] == 0) { // double indirect lvl2 blk allocation needed
      fs_blknum = DfsAllocateBlock();
      ptr_lvl1[offset_lvl1] = fs_blknum;
      DfsWriteBlock(inodes[handle].double_indirect, &fsblk_lvl1);
    }

    DfsReadBlock(ptr_lvl1[offset_lvl1], &fsblk_lvl2);
    ptr_lvl2 = (uint32*) fsblk_lvl2.data;
    offset_lvl2 = (virtual_blocknum-DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS-(sb.fs_blk_size/4)) % (sb.fs_blk_size/4);
    fs_blknum = DfsAllocateBlock();
    ptr_lvl2[offset_lvl2] = fs_blknum;
    DfsWriteBlock(inodes[handle].indirect, &fsblk_lvl2);

  }
  LockHandleRelease(inode_lock);

  return fs_blknum;

}



//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 vblknum) {
  int i, j;
  dfs_block fsblk_lvl1, fsblk_lvl2;
  uint32 *lvl1_ptr, *lvl2_ptr;
  uint32 lvl1_offset, lvl2_offset;
  uint32 dfs_blknum;

  if (sb.valid == 0) {
    printf("DfsInodeTranslateVirtualToFilesys (%d): ERROR - filesystem has NOT been opened yet\n", GetCurrentPid());
    return DFS_FAIL;
  }

  if (vblknum >= DFS_INODE_MAX_VIRTUAL_BLOCKNUM) {
    printf("DfsInodeTranslateVirtualToFilesys (%d): ERROR - Virtual block number %d is out-of-bound.\n", GetCurrentPid(), vblknum);
    printf("DfsInodeTranslateVirtualToFilesys (%d): INFO - Max virtual block number: %d.\n", GetCurrentPid(), DFS_INODE_MAX_VIRTUAL_BLOCKNUM);
    return DFS_FAIL;
  }

  LockHandleAcquire(inode_lock);
  if (inodes[handle].inuse == 0) {
    printf("DfsInodeTranslateVirtualToFilesys (%d): ERROR - This node is currently not in use\n", GetCurrentPid());
    LockHandleRelease(inode_lock);
    return DFS_FAIL;
  } else {
    if (vblknum < DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS) {
      // direct
      if (inodes[handle].direct[vblknum] == 0) {
        // printf("DfsInodeTranslateVirtualToFilesys (%d): ERROR - Direct pointer is not allocated\n", GetCurrentPid());
        return DFS_FAIL;
      }

      dfs_blknum = inodes[handle].direct[vblknum];

    } else if (vblknum < (DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS + (sb.fs_blk_size/4))) {
      // indirect
      if (inodes[handle].indirect == 0) {
        // printf("DfsInodeTranslateVirtualToFilesys (%d): ERROR - Indirect pointer is not allocated\n", GetCurrentPid());
        return DFS_FAIL;
      }

      DfsReadBlock(inodes[handle].indirect, &fsblk_lvl1);
      lvl1_ptr = (uint32*) fsblk_lvl1.data;
      lvl1_offset = vblknum - DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS;
      dfs_blknum = *(lvl1_ptr+lvl1_offset);
      
    } else {
      // double indirect - assume MAX virtual block number is within the range of double indirect
      if (inodes[handle].double_indirect == 0) {
        // printf("DfsInodeTranslateVirtualToFilesys (%d): ERROR - Double indirect lvl1 pointer is not allocated\n", GetCurrentPid());
        return DFS_FAIL;
      }

      DfsReadBlock(inodes[handle].double_indirect, &fsblk_lvl1);
      lvl1_ptr = (uint32*) fsblk_lvl1.data;
      lvl1_offset = vblknum - (DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS + (sb.fs_blk_size/4)); // find the relative block index
      lvl1_offset /= (sb.fs_blk_size/4); // find the pointer pointing to the specific lvl2 block

      if (*(lvl1_ptr+lvl1_offset) == 0) {
        // printf("DfsInodeTranslateVirtualToFilesys (%d): ERROR - Double indirect lvl2 pointer is not allocated\n", GetCurrentPid());
        return DFS_FAIL;
      }

      DfsReadBlock(*(lvl1_ptr+lvl1_offset), &fsblk_lvl2);
      lvl2_ptr = (uint32*) fsblk_lvl2.data;
      lvl2_offset = vblknum - (DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS + (sb.fs_blk_size/4)); // find the relative block index
      lvl2_offset %= (sb.fs_blk_size/4); // find the block num position in lvl2 block

      dfs_blknum = *(lvl2_ptr+lvl2_offset);
    }

  }
  LockHandleRelease(inode_lock);

  return dfs_blknum;
}
