CC=clang
CFLAGS=-Wall -O3 -pedantic 
CMD=${CC} ${CFLAGS}

clean:
	rm -f bin/* gen_random_ints utils.o

all: mergesort gen_random_ints pyramid_merge shellsort insertionsort_memcpy insertionsort stdlib_qsort stdlib_heapsort stdlib_mergesort

verbose: mergesort.c pyramid_merge.c utils.c
	${CMD} -DVERBOSE -c utils.c
	${CMD} -DVERBOSE utils.o mergesort.c -o bin/mergesort
	${CMD} -DVERBOSE utils.o pyramid_merge.c -o bin/pyramid_merge
	${CMD} -DVERBOSE utils.o shellsort.c -o bin/shellsort
	${CMD} -DVERBOSE utils.o insertionsort.c -o bin/insertionsort
	${CMD} -DVERBOSE utils.o insertionsort_memcpy.c -o bin/insertionsort_memcpy

mergesort: mergesort.c utils.o
	${CMD} utils.o mergesort.c -o bin/mergesort

pyramid_merge: pyramid_merge.c utils.o
	${CMD} utils.o pyramid_merge.c -o bin/pyramid_merge

shellsort: shellsort.c utils.o
	${CMD} utils.o shellsort.c -o bin/shellsort

insertionsort: insertionsort.c utils.o
	${CMD} utils.o insertionsort.c -o bin/insertionsort

insertionsort_memcpy: insertionsort_memcpy.c utils.o
	${CMD} utils.o insertionsort_memcpy.c -o bin/insertionsort_memcpy

stdlib_qsort: stdlib_qsort.c utils.o
	${CMD} utils.o stdlib_qsort.c -o bin/stdlib_qsort

stdlib_heapsort: stdlib_heapsort.c utils.o
	${CMD} utils.o stdlib_heapsort.c -o bin/stdlib_heapsort

stdlib_mergesort: stdlib_mergesort.c utils.o
	${CMD} utils.o stdlib_mergesort.c -o bin/stdlib_mergesort


utils.o: utils.h utils.c
	${CMD} -c utils.c