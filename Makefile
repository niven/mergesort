CC=clang
CFLAGS=-Wall -O3 -pedantic 
CMD=${CC} ${CFLAGS}

clean:
	rm -f bin/mergesort gen_random_ints bin/pyramid_merge bin/shellsort bin/insertionsort bin/stdlib_qsort bin/stdlib_heapsort utils.o

all: mergesort gen_random_ints pyramid_merge shellsort insertionsort stdlib_qsort stdlib_heapsort

verbose: mergesort.c pyramid_merge.c utils.c
	${CMD} -DVERBOSE -c utils.c
	${CMD} -DVERBOSE utils.o mergesort.c -o bin/mergesort
	${CMD} -DVERBOSE utils.o pyramid_merge.c -o bin/pyramid_merge
	${CMD} -DVERBOSE utils.o shellsort.c -o bin/shellsort
	${CMD} -DVERBOSE utils.o insertionsort.c -o bin/insertionsort

mergesort: mergesort.c utils.o
	${CMD} utils.o mergesort.c -o bin/mergesort

pyramid_merge: pyramid_merge.c utils.o
	${CMD} utils.o pyramid_merge.c -o bin/pyramid_merge

shellsort: shellsort.c utils.o
	${CMD} utils.o shellsort.c -o bin/shellsort

insertionsort: insertionsort.c utils.o
	${CMD} utils.o insertionsort.c -o bin/insertionsort

stdlib_qsort: stdlib_qsort.c utils.o
	${CMD} utils.o stdlib_qsort.c -o bin/stdlib_qsort

stdlib_heapsort: stdlib_heapsort.c utils.o
	${CMD} utils.o stdlib_heapsort.c -o bin/stdlib_heapsort


utils.o: utils.h utils.c
	${CMD} -c utils.c