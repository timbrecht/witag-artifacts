CFLAGS= -Wall -Werror -DTEST

getopt: getopt.c getopt.h
	gcc $(CFLAGS) getopt.c -o getopt

getopt-long: getopt1.c getopt.h
	gcc $(CFLAGS) getopt1.c -o getopt-long

test: getopt
	./getopt -2 -a -b -c 10 file1 file2

test-long: getopt-long
	./getopt-long -2 -a -b -c 10 file1 file2 --add --append --file file3

clean:
	-/bin/rm getopt

