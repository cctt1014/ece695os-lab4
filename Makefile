# Input arguments for one-level
TEST_IDX=0


.PHONY: one two fork run_one run_two run_fork clean

one:
	cd one-level; make

two:
	cd two-level; make

fork:
	cd fork; make

run_one:
	cd one-level; make run TEST_IDX=${TEST_IDX}

run_two:
	cd two-level; make run TEST_IDX=${TEST_IDX}

run_fork:
	cd fork; make run TEST_IDX=${TEST_IDX}

clean:
	cd one-level; make clean
	cd two-level; make clean
	cd fork; make clean
