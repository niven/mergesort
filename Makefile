CC=clang
CFLAGS=-Wall -O3 -pedantic 
CMD=${CC} ${CFLAGS}

clean:
	rm -f mergesort gen_random_ints pyramid_merge utils.o

all: mergesort gen_random_ints pyramid_merge

verbose: mergesort.c pyramid_merge.c utils.c
	${CMD} -DVERBOSE -c utils.c
	${CMD} -DVERBOSE utils.o mergesort.c -o mergesort
	${CMD} -DVERBOSE utils.o pyramid_merge.c -o pyramid_merge

mergesort: mergesort.c utils.o
	${CMD} utils.o mergesort.c -o mergesort

pyramid_merge: pyramid_merge.c utils.o
	${CMD} utils.o pyramid_merge.c -o pyramid_merge

utils.o: utils.h utils.c
	${CMD} -c utils.c