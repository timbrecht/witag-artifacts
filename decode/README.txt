Fri Oct  4 10:28:05 EDT 2024
This version is used to succesfully filter and decode messages
It is much more general now. See all of the options available.

-------
Thu Jan 20 19:52:23 EST 2022

NOTE: Right now everything is set up for 5 bits of data.

./decode -h
To see usage and option descriptions

Note that the -v (verbose) option will give more detailed
information when running.

Updated example use cases.
# If the input contains two columns the block ack and expected string 
# E.g.,
# 10000001111111111100000000000000 11000
# 11100000000000001111111111111111 11000
# 00000000000111111111111111111111 11000
# 11100000000000000000000000000000 11000
./decode 11000.dat

# To check what happens when using 'n' block acks
./decode -n 100 11000.dat
# To monitor changes over the entire data 
./decode -n 100 -v 11000.dat


# If the input doesn't contain the expected string (only block acks)
# -e  means the input does not contain expected strings
# E.g.,
# 10000001111111111100000000000000
# 11100000000000001111111111111111
# 00000000000111111111111111111111
./decode -e 11000.acks


-------
Wed 15 Dec 2021 08:20:24 PM EST

- Added support for -I inputfile option

- Added support for reading input from stdin so you can do
cat 00000.txt | ./decode -e -v
and see the detected output live as the packets arrive.

You should now be able to run the code that pulls out
just the block acks and run it something like
getblockacks | ./decode -e -v
and see what values are getting decoded live.

./decode -e 11000.acks

-------
Wed 15 Dec 2021 08:20:24 PM EST

- Added support for -I inputfile option

- Added support for reading input from stdin so you can do
cat 00000.txt | ./decode -e -v
and see the detected output live as the packets arrive.

You should now be able to run the code that pulls out
just the block acks and run it something like
getblockacks | ./decode -e -v
and see what values are getting decoded live.

-------
Mon Dec 13 17:36:43 EST 2021

Moved the old version of the code that was used for demos to
decode-demo.c

Reworked decode.c to use the getopt code provided
to handle the command line options.

Factored out the LUX conversion code to another file (sensors.c).

Added command line option to specify the sensor type.
./decode -h 
to see the options supported now

To run for demo code probably use these options (with the light sensor)
./decode -q -S -A -e -s 2 -f 31.0 -o 0.0 -O outputfile inputfile
./decode -h to see what the options are.

Remove quite options (-q) and add verbose option (-v) if you need
more useful information about what is going on.


-------
Thu Nov 18 13:29:02 EST 2021
This code now seems to be getting used in our demo system.

Note for running the code for a demo enable 
DEMO_MODE in try-regex.c and figure out if
you want SILENT_MODE also enabled.

./try-regex -h
For usage.

-------
Thu Nov  4 14:51:47 EDT 2021
This is code developed to play around with packet recognition.
