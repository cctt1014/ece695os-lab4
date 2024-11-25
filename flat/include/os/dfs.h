#ifndef __DFS_H__
#define __DFS_H__

#include "dfs_shared.h"
#include "queue.h"

#define DFS_NUM_BUFFER_CACHE_ENTRY 16

typedef struct buffer_cache_slot {
  int inuse;
	int dirty; // To indicate the block is dirty and must be written to disk during eviction
	uint32 fblknum; // DFS block number cached by this buffer cache entry
	int count; // Number of file operations
	Link *l; // To move this node around empty and full queue.
} buffer_cache_slot;

void DfsBufferCacheInit();

int DfsReadContiguousBytes(int start_blocknum, char *dest, int num_bytes);
int DfsCompareTwoString(char* str1, char* str2);

// Inode APIs
uint32 DfsInodeFilenameExists(char *filename);
uint32 DfsInodeOpen(char *filename);
int DfsInodeDelete(uint32 handle);
int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes);
int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes);
uint32 DfsInodeFilesize(uint32 handle);
uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum);
uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 vblknum);

#endif
