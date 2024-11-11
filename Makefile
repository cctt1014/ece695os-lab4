
.PHONY: fdisk run_fdisk build_os clean print

fdisk: build_os
	cd flat/apps/fdisk; make

run_fdisk: 
	cd flat/apps/fdisk; make run 

build_os:
	cd flat/os; make

print:
	cd flat/scripts; ./blockprint 0 52 /tmp/ece695tc.img

# run_fork:
# 	cd fork; make run TEST_IDX=${TEST_IDX}

clean:
	cd flat/apps/fdisk; make clean
	cd flat/os; make clean
