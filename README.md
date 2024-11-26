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

For instance, when we would like to build and run ostests:
```shell
make ostests
```



# Potential Issues
1.  The error below might be encountered if using mainframer to run this program. But the error will be gone if run on remote machine directly.
```shell
    stty: standard input: Inappropriate ioctl for device
    make: *** [run] Error 1
```

# Files Modified for Each Questions
The file list may not be accurate enough, for details please check out the .git folder.
1. Q1:
    1. flat/apps/fdisk/fdisk/fdisk.c
    2. flat/apps/fdisk/include/fdisk.h
    3. flat/include/dfs_shared.h
    4. flat/include/os/disk.h
    5. flat/os/dfs.c



# References
1. https://github.com/ckv123/Operating-System/tree/master/lab5
