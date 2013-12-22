CC=clang
CFLAGS=-Wall -O3 -pedantic 

clean:
	rm -f mergesort gen_random_ints pyramid_merge utils.o

all: mergesort gen_random_ints pyramid_merge

mergesort: mergesort.c utils.o
	clang utils.o mergesort.c -o mergesort

pyramid_merge: pyramid_merge.c utils.o
	clang utils.o pyramid_merge.c -o pyramid_merge

utils.o: utils.h utils.c
	clang -c utils.c