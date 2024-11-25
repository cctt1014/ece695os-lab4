#ifndef __DFS_H__
#define __DFS_H__

#include "dfs_shared.h"

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
