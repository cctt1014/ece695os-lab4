#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

//dfs_superblock sb;
//dfs_inode inodes[DFS_INODE_MAX_NUM];
//uint32 freeblockvector[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

void main (int argc, char *argv[])
{
  dfs_block block;
  // STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
  // You need to think of the finer details. You can use bzero() to zero out bytes in memory

  //1) Initializations and argc check
  if (argc != 1) {
    Printf("Usage: %s\n", argv[0]);
    Exit();
  }

  // 2) Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  // sb.valid = 0

  //disksize = 
  //diskblocksize = 
  //num_filesystem_blocks = 

  // 3) Make sure the disk exists before doing anything else. You can use disk_create()
 
  // 4) Write all inodes as not in use and empty (Write zeros to all the
  // dfs blocks corresponding to the inodes using FdiskWriteBlock)

  // 5) Next, setup free block vector (fbv) and write free block vector to the disk
  // 5.1) Mark all the blocks used by file system as "Used"
  // 5.2) Mark all other blocks as "Unused"
  // 5.3) Use FdiskWriteBlock to write to disk.

  // 6) Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  // 6.1) boot record is all zeros in the first physical block, 
  bzero((char *)&block, DFS_BLOCKSIZE);
  // 6.2) superblock structure goes into the second physical block
  // bcopy ....

  //  6.3) Write superblock to the disk
  // FdiskWriteBlock(FDISK_BOOT_FILESYSTEM_BLOCKNUM, &block);
  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {
  // STUDENT: put your code here
  // Remember, the argument blocknum passed is dfs_blocknum not physical block.
}
