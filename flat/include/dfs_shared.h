#ifndef __DFS_SHARED__
#define __DFS_SHARED__

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  int valid; // valid indicator for the file system
  int fs_blk_size;
  int disk_blk_size;
  int total_num_fs_block;
  int inode_fs_blk_idx;
  int num_inodes;
  int fbv_fs_blk_idx;
} dfs_superblock;

#define DFS_BLOCKSIZE 1024  // Must be an integer multiple of the disk blocksize

typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;



#define DFS_INODE_MAX_FILENAME_LENGTH 72
#define DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS 10
#define DFS_INODE_NUM_INDIRECT_ADDRESSED_BLOCKS 1
#define DFS_INODE_MAX_VIRTUAL_BLOCKNUM 128

typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 128 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 128 bytes.
  int inuse; // 4B
  int file_size; // 4B
  char fname[DFS_INODE_MAX_FILENAME_LENGTH];
  uint32* direct[DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS]; // 4*10 = 40B
  uint32* indirect; // 4B
  uint32* double_indirect; // 4B
} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x4000000  // 64MB 
#define DFS_MAX_NUM_BLOCKS (DFS_MAX_FILESYSTEM_SIZE / DFS_BLOCKSIZE)


#define DFS_BITS_PER_BYTE 8
#define DFS_FBV_BITS_PER_ENTRY 32
#define DFS_FBV_MAX_NUM_WORDS (DFS_MAX_NUM_BLOCKS / DFS_FBV_BITS_PER_ENTRY)

// #define DFS_INODE_MAX_NUM_FILESYSTEM_BLOCKS 
#define DFS_INODE_MAX_NUM 128

#define DFS_SUPERBLOCK_PHYSICAL_BLOCKNUM 1 // Where to write superblock on the disk

#define DFS_FAIL -1
#define DFS_SUCCESS 1



#endif
