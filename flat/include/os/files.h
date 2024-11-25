#ifndef __FILES_H__
#define __FILES_H__

#include "dfs.h"
#include "files_shared.h"

#define FILE_MAX_OPEN_FILES 15

void FileModuleInit();
uint32 FileOpen(char *filename, char *mode);
int FileClose(uint32 handle);
int FileRead(uint32 handle, void *mem, int num_bytes);
int FileWrite(uint32 handle, void *mem, int num_bytes);
int FileSeek(uint32 handle, int num_bytes, int from_where);
int FileDelete(char *filename);

#endif
