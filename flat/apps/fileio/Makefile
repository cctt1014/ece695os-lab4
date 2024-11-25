default:
	cd fileio; make

clean:
	cd fileio; make clean

run:
	cd ../../os; make; cd ../bin; dlxsim -x os.dlx.obj -a -D F -u fileio.dlx.obj; ee469_fixterminal
