# Steps to Run
Note that clean and build need to be run in /tmp due to the bug mentioned in lab1 instructions. 
## To clean:
```shell
make clean
```

## To build:
```shell
make <target>
```

\<target\> should be replaced by the target you would like to build. Here is the list of target available:
* one
* two
* fork

For instance, when we would like to build fork:
```shell
make fork
```



## To run the test program:
### Run with fork built

Run with default input values:
```shell
make run_fork
```
Input arguments TEST_IDX is used to select the test. For instance, if you would like to run fork test in part 5:
```shell
make run_fork TEST_IDX=6
```

TEST_IDX options: 
* 0: Hello World
* 1: Beyond max virtual address
* 2: Beyond allocated pages
* 3: Grow user stack to more than 1 page
* 4: Create 100 processes sequentially to print Hello World
* 5: Spawn 30 processes to run in parallel
* 6: Fork test for part 5 (for fork built ONLY)


# Potential Issues
1.  The error below might be encountered if using mainframer to run this program. But the error will be gone if run on remote machine directly.
```shell
    stty: standard input: Inappropriate ioctl for device
    make: *** [run] Error 1
```

2. The "Usage" error below usually shows up when there is a input argument mismatch. To get rid of this error, ensure that "make clean" is run before building and running a new target.
```shell
    Usage: makeprocs.dlx.obj <number of processes to create>
    No runnable processes - exiting!
```

# References
1. https://github.com/sgianelli/DLXOS-Virtual-Memory-Project
2. http://electro.fisica.unlp.edu.ar/arq/downloads/Software/WinDLX/dlxarch.shtml
