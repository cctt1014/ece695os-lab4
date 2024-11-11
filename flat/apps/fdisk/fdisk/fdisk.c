#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 freeblockvector[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

void main (int argc, char *argv[])
{
  dfs_block block;
  // STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
  // You need to think of the finer details. You can use bzero() to zero out bytes in memory
  int i, j;
  char * fbv_ptr;

  //1) Initializations and argc check
  if (argc != 1) {
    Printf("Usage: %s\n", argv[0]);
    Exit();
  }

  // 2) Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  sb.valid = 0;

  disksize = disk_size();
  diskblocksize = disk_blocksize();
  sb.total_num_fs_block = DFS_MAX_NUM_BLOCKS;
  sb.inode_fs_blk_idx = FDISK_INODE_BLOCK_START;
  sb.fbv_fs_blk_idx = FDISK_FBV_BLOCK_START;
  sb.fs_blk_size = DFS_BLOCKSIZE;
  sb.num_inodes = DFS_INODE_MAX_NUM;
  
  Printf("fdisk (%d): DBG - disksize=%x, diskblocksize=%d\n", getpid(), disksize, diskblocksize);

  // 3) Make sure the disk exists before doing anything else. You can use disk_create()
  if (disk_create() == DISK_FAIL) {
    Printf("fdisk (%d): ERROR - Failed to create disk\n", getpid());
    return;
  }
 
  // 4) Write all inodes as not in use and empty (Write zeros to all the
  // dfs blocks corresponding to the inodes using FdiskWriteBlock)
  bzero((char *)&block, DFS_BLOCKSIZE);
  for (i = 0; i < FDISK_INODE_NUM_BLOCKS; i++) {
    FdiskWriteBlock(i+FDISK_INODE_BLOCK_START, &block);
  }

  // 5) Next, setup free block vector (fbv) and write free block vector to the disk
  // 5.1) Mark all the blocks used by file system as "Used"
  // 5.2) Mark all other blocks as "Unused"
  // 5.3) Use FdiskWriteBlock to write to disk.
  for (i = 0; i < DFS_FBV_MAX_NUM_WORDS; i++) {
    freeblockvector[i] = 0;
  }
  freeblockvector[0] = 0x03ffffff; // blk 0 - 25 is marked as "Used"

  fbv_ptr = (char *) freeblockvector;
  for (i = 0; i < FDISK_FBV_NUM_BLOCKS; i++) {
    bcopy(fbv_ptr+(DFS_BLOCKSIZE*i), block.data, DFS_BLOCKSIZE);
    // if (i == 0) {
    //   Printf("fdisk (%d): DBG Print a block here =============\n");
    //   for (j = 0; j < DFS_BLOCKSIZE; j++) {
    //     if (j % 32 == 0) {
    //       Printf("Line %d: ", j/32);
    //     }
    //     Printf("%x", block.data[j]);
    //     if (j % 32 == 31) {
    //       Printf("\n");
    //     }
    //   }
    //   Printf("fdisk (%d): DBG Print out ends here =============\n");
    // }
    FdiskWriteBlock(i+FDISK_FBV_BLOCK_START, &block);
  }

  // 6) Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  // 6.1) boot record is all zeros in the first physical block, 
  bzero((char *)&block, DFS_BLOCKSIZE);
  // 6.2) superblock structure goes into the second physical block
  bcopy((char *)&sb ,(char *)&block+diskblocksize, diskblocksize);



  //  6.3) Write superblock to the disk
  FdiskWriteBlock(FDISK_BOOT_FILESYSTEM_BLOCKNUM, &block);
  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {
  // STUDENT: put your code here
  // Remember, the argument blocknum passed is dfs_blocknum not physical block.
  int scaler;
  int i;
  char *disk_b;

  disk_b = (char *) b;

  // since dfs_blocksize = scaler * disk_blocksize
  // disk_blocknum = (dfs_blocknum * scaler) + i
  // where i interate from 0 to scaler-1
  scaler = DFS_BLOCKSIZE/diskblocksize;

  for (i = 0; i < scaler; i++) {
    if (disk_write_block((blocknum*scaler)+i, disk_b+(i*diskblocksize)) == DISK_FAIL) {
      Printf("fdisk (%d): ERROR - disk_write_block failed to write\n", getpid());
      return DFS_FAIL;
    }
    
  }

  // Printf("FdiskWriteBlock (%d): DBG - write to dfs block %d successfully\n", getpid(), blocknum);
  return DFS_SUCCESS;
}
