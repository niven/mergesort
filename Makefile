CC=clang
CFLAGS=-Wall -O3 -pedantic 
CMD=${CC} ${CFLAGS}

all: gen_random_ints utils.o sort_functions
	${CMD} utils.o insertionsort.o main_template.c -o bin/insertionsort
	${CMD} utils.o stdlib_qsort.o main_template.c -o bin/stdlib_qsort
	${CMD} utils.o stdlib_mergesort.o main_template.c -o bin/stdlib_mergesort
	${CMD} utils.o stdlib_heapsort.o main_template.c -o bin/stdlib_heapsort
	${CMD} utils.o shellsort.o main_template.c -o bin/shellsort

verbose: *.c
	${CMD} -DVERBOSE -c utils.c
	${CMD} -DVERBOSE -c *sort.c
	${CMD} -DVERBOSE utils.o insertionsort.o main_template.c -o bin/insertionsort
	${CMD} -DVERBOSE utils.o stdlib_qsort.o main_template.c -o bin/stdlib_qsort
	${CMD} -DVERBOSE utils.o stdlib_mergesort.o main_template.c -o bin/stdlib_mergesort
	${CMD} -DVERBOSE utils.o stdlib_heapsort.o main_template.c -o bin/stdlib_heapsort
	${CMD} -DVERBOSE utils.o shellsort.o main_template.c -o bin/shellsort

sort_functions: *sort.c
	${CMD} -c *sort.c

clean:
	rm -f bin/* gen_random_ints *.o

insertionsort: main_template.c utils.o insertionsort.o
	${CMD} utils.o insertionsort.o main_template.c -o bin/insertionsort

shellsort: main_template.c utils.o shellsort.o
	${CMD} utils.o shellsort.o main_template.c -o bin/shellsort

stdlib_qsort: main_template.c utils.o stdlib_qsort.o
	${CMD} utils.o stdlib_qsort.o main_template.c -o bin/stdlib_qsort

stdlib_mergesort: main_template.c utils.o stdlib_mergesort.o
	${CMD} utils.o stdlib_mergesort.o main_template.c -o bin/stdlib_mergesort

stdlib_heapsort: main_template.c utils.o stdlib_heapsort.o
	${CMD} utils.o stdlib_heapsort.o main_template.c -o bin/stdlib_heapsort

utils.o: utils.h utils.c
	${CMD} -c utils.c



