#ifndef __DISK_H__
#define __DISK_H__

// Student: Change the filename to ensure it does not get rewritten by
// someone else.

// Name of file which represents the "hard disk".
#define DISK_FILENAME "/tmp/ece695tc.img"

// Number of bytes in one physical disk block
#define DISK_BLOCKSIZE 512 

typedef struct disk_block {
  char data[DISK_BLOCKSIZE]; // This assumes that DISK_BLOCKSIZE is a multiple of 4 (for byte alignment)
} disk_block;


// Total size of this disk (64MB), in units of 512-byte blocks
#define DISK_NUMBLOCKS 0x20000 // STUDENT: Calculate the correct no of blocks and write. This value is wrong

#define DISK_SUCCESS 1
#define DISK_FAIL -1

// this function returns the number of bytes in one physical block.
int DiskBytesPerBlock();
// this function returns the number of bytes in the entire disk.
int DiskSize();
// this function creates the Linux file that will hold the file system.
int DiskCreate();
// this function writes one physical block of data to the file at the specified block number.
int DiskWriteBlock (uint32 blocknum, disk_block *b);
// this function reads one physical block of data from the file at the specified block number.
int DiskReadBlock (uint32 blocknum, disk_block *b);

#endif
