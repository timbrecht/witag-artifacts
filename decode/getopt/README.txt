We are using getopt.c 

To compile and test run:

make test

You should see output like this:

./getopt -2 -a -b -c 10 file1 file2
option 2
option a
option b
option c with value `10'
non-option ARGV-elements: file1 file2
