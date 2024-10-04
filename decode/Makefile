# GIT_VERSION := "$(shell git describe --abbrev=4 --dirty --always --tags)"
GIT_VERSION := "$(shell git describe --long --dirty --always --tags)"

BIN=$(HOME)/bin
PGM=decode
# CFLAGS = -Wall -Werror -g # -O2 # -fPIC
CFLAGS = -Wall -g -O2 # -fPIC

# For use with valgrind
# CFLAGS = -Wall -Werror -g -O0

VALGRIND=valgrind -s --leak-check=yes --leak-check=full --show-leak-kinds=all

# VALGRIND=valgrind -s --leak-check=yes --leak-check=full --show-leak-kinds=all --tool=memcheck --tool=exp-sgcheck
# VALGRIND=valgrind \
   --tool=memcheck --tool=exp-sgcheck\

#   --show-error-list=yes --leak-check=full --show-leak-kinds=all

# VALGRIND=valgrind -s --leak-check=yes --leak-check=full --show-leak-kinds=all
# VALGRIND=valgrind -s

SRC = $(PGM).c levenshtein-new.c sensors-new.c cqueue.c test-cqueue.c test-levenshtein.c ack-stats.c regexes.c mpdusize-variation.c window.c filter.c durations.c
HFILES = levenshtein.h cqueue.h 
OTHERSRC = README
TARSRC = $(README) $(SRC) $(HFILES) Makefile
LEV=levenshtein-new.o
OBJS= sensors-new.o cqueue.o regexes.o ack-stats.o $(PGM).o getopt/getopt.o \
      window.o filter.o durations.o utils.o \
      globals.o options.o process-options.o decode-algs.o expected.o \
      interval-stats.o

all: test-cqueue test-levenshtein test-regexes test-ack-stats mpdusize-variation $(PGM)

git-version.h:
	/bin/echo "// DO NOT add this to the git repo. It is generated on each build" > git-version.h
	/bin/echo "#define GIT_VERSION \"$(GIT_VERSION)\"" >> git-version.h

$(PGM): git-version.h $(OBJS) $(LEV) *.h
	gcc $(CFLAGS) $(LEV) $(OBJS) -lm -o $(PGM)

test-cqueue: test-cqueue.c cqueue.h cqueue.c
	gcc $(CFLAGS) test-cqueue.c cqueue.c -o test-cqueue

test-levenshtein: utils.o test-levenshtein.c levenshtein.h $(LEV)
	gcc $(CFLAGS) utils.o test-levenshtein.c $(LEV) -o test-levenshtein

test-regexes: test-regexes.c regexes.h regexes.c
	gcc $(CFLAGS) test-regexes.c regexes.c -o test-regexes

test-ack-stats: test-ack-stats.c ack-stats.h ack-stats.c cqueue.c cqueue.h
	gcc $(CFLAGS) test-ack-stats.c ack-stats.c cqueue.c -o test-ack-stats

mpdusize-variation: mpdusize-variation.c 
	gcc $(CFLAGS) mpdusize-variation.c -o mpdusize-variation  -lm

# test-try:
#	./$(PGM) -v -L -P 3 -B 3 -M 3000.dat

test-consec:
	./$(PGM) -n 150 -L -P 3 -B 3 -M 3 data/000.dat

test-demo:
	./$(PGM) -e -L -P 3 -B 5 -M 3 data/00000.acks

test1-basic:
	./$(PGM) -e -B 1 -M 6 -P 3 -v data/b1-m6-p3.ack

bug:
	./$(PGM) -e -E 3 -L -P 3 -B 5 -M 3 ../../../kamran/Dumps/2022-05-01/1m/1m_p3b5m03-00011.txt

most-freq:
	./$(PGM) -n 800 -v -e -E 3 -L -P 3 -B 5 -M 3 ../../../kamran/Dumps/2022-05-01/1m/1m_p3b5m03-00011.txt


# NOTE: current file also contains 1's
test1-expected:
	./$(PGM) -e -b 0 -c 1 -B 1 -M 6 -P 3 -v -E 0 -R 10 -i data/b1-m6-p3.ack

test3-5:
	./$(PGM) -b 0 -c 1 -B 3 -M 3 -P 3 -v data/101.dat

test3-7:
	./$(PGM) -b 1 -c 1 -B 3 -M 3 -P 3 -v data/111.dat


test5-0:
	./$(PGM) -b 0 -c 1 -B 5 -M 3 -P 3 -v data/00000.dat

test5-24:
	./$(PGM) -b 0 -c 1 -B 5 -M 3 -P 3 -v data/11000.dat

test5-window:
	./$(PGM) -b 0 -c 1 -B 5 -M 3 -P 3 -v -w 100 data/11000.dat

test5-packet-window-b-c:
	./$(PGM) -b 0 -c 1 -B 5 -M 3 -P 3 -v -l 10 data/11000.dat

test5-packet-window:
	./$(PGM) -l 10 -B 5 -M 3 -P 3 -v -N 32 data/11000.dat

test5-packet-window-15:
	./$(PGM) -b 0 -c 1 -B 5 -M 3 -P 3 -v -l 15 data/11000.dat

test5-packet-window-20:
	./$(PGM) -b 0 -c 1 -B 5 -M 3 -P 3 -v -l 20 data/11000.dat





tar:
	tar -czf src.tgz $(TARSRC)

REMOTEDIR=192.168.86.1:/home/root/scripts/witag/fordemo/decode
push:
	scp $(TARSRC) $(REMOTEDIR)

valgrind:
	$(VALGRIND) ./$(PGM) -e -N 64 -B 10 -M 3 -P 3 -v data/b10m3p3_60sec_4ms.txt

bug2:
	./$(PGM) -e -N 64 -B 10 -M 3 -P 3 -v data/b10m3p3_60sec_4ms.txt

bug3:
	./$(PGM) -R 10 -e -N 64 -B 10 -M 3 -P 3 -v data/b10m3p3_60sec_4ms.txt

bug4:
	./$(PGM) -e -N 64 -B 10 -M 3 -P 3 -v data/b10m3p3_60sec_4ms.txt

bug5:
	./$(PGM) -e -N 64 -B 10 -M 3 -P 3 -v temp


oldinput:
	./$(PGM) -b 1 -c 4 -z 5 -Z 24 -e -E 0 -L -B 5 -M 3 -P 3 -N 32 -v data/00000-old.txt




#	$(VALGRIND) ./$(PGM) -B 5 -M 3 -P 3 -v -w 100 11000.dat

ctags:
	ctags *.[ch]

wc:
	wc *.[ch]

clean:
	-/bin/rm -rf $(PGM).dSYM $(PGM) test-cqueue test-cqueue.dSYM
	-/bin/rm -rf test-levenshtein.dSYM test-levenshtein
	-/bin/rm -rf test-regexes.dSYM test-regexes 
	-/bin/rm -rf test-ack-stats.dSYM test-ack-stats
	-/bin/rm -rf mpdusize-variation.dSYM mpdusize-variation
	-/bin/rm -rf $(OBJS) $(LEV)
	-/bin/rm -rf git-version.h

vclean: clean
	-/bin/rm -rf tags
	-/bin/rm -rf cscope.out

install:
	cp decode $(BIN)
	
