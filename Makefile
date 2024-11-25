PS=0
PE=52

.PHONY: fdisk ostests fileio build_os clean print

fdisk: clean build_os
	cd flat/apps/fdisk; make; make run

ostests: clean build_os
	cd flat/apps/ostests; make; make run 

fileio: clean build_os
	cd flat/apps/fileio; make; make run 

build_os:
	cd flat/os; make

print:
	cd flat/scripts; ./blockprint ${PS} ${PE} /tmp/ece695tc.img

# run_fork:
# 	cd fork; make run TEST_IDX=${TEST_IDX}

clean:
	cd flat/apps/fdisk; make clean
	cd flat/apps/ostests; make clean
	cd flat/apps/fileio; make clean
	cd flat/os; make clean
