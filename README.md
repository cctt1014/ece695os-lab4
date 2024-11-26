# Steps to Run
Note that clean and build need to be run in /tmp due to the bug mentioned in lab1 instructions. 
## To clean:
```shell
make clean
```

## To build and run:
```shell
make <target>
```

\<target\> should be replaced by the target you would like to build and run. Here is the list of target available:
* fdisk -- format disk
* ostests -- run os tests to verify Inode APIs at OS level
* fileio -- run user level tests to verify File APIs

For instance, when we would like to build and run fileio which is user level file I/O test:
```shell
make fileio
```



# Potential Issues
1.  The error below might be encountered if using mainframer to run this program. But the error will be gone if run on remote machine directly.
```shell
    stty: standard input: Inappropriate ioctl for device
    make: *** [run] Error 1
```

# Files Modified for Each Questions
Some files like Makefile and Makerules are skipped here.
1. Q1:
    1. flat/apps/fdisk/fdisk/fdisk.c
    2. flat/apps/fdisk/include/fdisk.h
    3. flat/include/dfs_shared.h
    4. flat/include/os/disk.h
    5. flat/os/dfs.c
2. Q2:
    1. flat/apps/fdisk/fdisk/fdisk.c
    2. flat/include/dfs_shared.h
    3. flat/include/os/dfs.h
    4. flat/os/dfs.c
3. Q3:
    1. flat/include/dfs_shared.h
    2. flat/include/os/dfs.h
    3. flat/os/dfs.c
4. Q4:
    1. flat/os/dfs.c
    2. flat/os/ostests.c
    3. flat/include/dfs_shared.h
5. Q5:
    1. flat/include/files_shared.h
    2. flat/include/os/dfs.h
    3. flat/include/os/files.h
    4. flat/os/dfs.c
    5. flat/os/files.c
    6. flat/os/ostests.c
    7. flat/os/process.c
6. Q6:
    1. flat/apps/fileio/fileio/fileio.c
    2. flat/apps/fileio/include/fileio.h
    3. flat/include/os/files.h
    4. flat/os/dfs.c
    5. flat/os/files.c
7. Q7:
    1. flat/include/os/dfs.h
    2. flat/os/dfs.c





# References
1. https://github.com/ckv123/Operating-System/tree/master/lab5
