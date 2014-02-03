CC=clang
PAD_SIZE=4
CFLAGS=-Wall -O3 -pedantic -DPAD_SIZE=${PAD_SIZE}
CMD=${CC} ${CFLAGS}

all: clean tools sort_functions
	${CMD} -DSORT_FUNCTION=insertionsort utils.o insertionsort.o main_template.c -o bin/insertionsort
	${CMD} -DSORT_FUNCTION=qsort utils.o main_template.c -o bin/stdlib_qsort
	${CMD} -DSORT_FUNCTION=mergesort utils.o main_template.c -o bin/stdlib_mergesort
	${CMD} -DSORT_FUNCTION=heapsort utils.o main_template.c -o bin/stdlib_heapsort
	${CMD} -DSORT_FUNCTION=shellsort utils.o shellsort.o main_template.c -o bin/shellsort
	${CMD} -DSORT_FUNCTION=merge_sort_wrapper utils.o mergesort.o main_template.c -o bin/mergesort
	${CMD} -DSORT_FUNCTION=pyramid_mergesort_wrapper utils.o pyramid_mergesort.o main_template.c -o bin/pyramid_mergesort
	${CMD} -DSORT_FUNCTION=timsort utils.o insertionsort.o timsort.o main_template.c -o bin/timsort

verbose: clean *.c
	${CMD} -DVERBOSE -c utils.c
	${CMD} -DVERBOSE -c ziggurat.c
	${CMD} -DVERBOSE -c *sort.c
	${CMD} -DVERBOSE utils.o ziggurat.o gen_random_structs.c -o gen_random_structs
	${CMD} -DVERBOSE -DSORT_FUNCTION=insertionsort utils.o insertionsort.o main_template.c -o bin/insertionsort
	${CMD} -DVERBOSE -DSORT_FUNCTION=qsort utils.o main_template.c -o bin/stdlib_qsort
	${CMD} -DVERBOSE -DSORT_FUNCTION=mergesort utils.o main_template.c -o bin/stdlib_mergesort
	${CMD} -DVERBOSE -DSORT_FUNCTION=heapsort utils.o main_template.c -o bin/stdlib_heapsort
	${CMD} -DVERBOSE -DSORT_FUNCTION=shellsort utils.o shellsort.o main_template.c -o bin/shellsort
	${CMD} -DVERBOSE -DSORT_FUNCTION=merge_sort_wrapper utils.o mergesort.o main_template.c -o bin/mergesort
	${CMD} -DVERBOSE -DSORT_FUNCTION=pyramid_mergesort_wrapper utils.o pyramid_mergesort.o main_template.c -o bin/pyramid_mergesort
	${CMD} -DVERBOSE -DSORT_FUNCTION=timsort utils.o insertionsort.o timsort.o main_template.c -o bin/timsort

sort_functions: *sort.c
	${CMD} -c *sort.c
	
clean:
	rm -rf bin/* testdata/ gen_random_ints gen_random_structs ziggurat_test test_find_index *.o

tools: utils.* gen_random_ints.c gen_random_structs.c ziggurat.*
	${CMD} -c utils.c
	${CMD} -c ziggurat.c
	${CMD} ziggurat_test.c ziggurat.o -o ziggurat_test
	${CMD} gen_random_ints.c -o gen_random_ints
	${CMD} gen_random_structs.c ziggurat.o -o gen_random_structs

insertionsort: main_template.c utils.o insertionsort.o
	${CMD} -DSORT_FUNCTION=insertionsort utils.o insertionsort.o main_template.c -o bin/insertionsort

shellsort: main_template.c utils.o shellsort.o
	${CMD} -DSORT_FUNCTION=shellsort utils.o shellsort.o main_template.c -o bin/shellsort

mergesort: main_template.c utils.o mergesort.o
	${CMD} -DSORT_FUNCTION=merge_sort_wrapper utils.o mergesort.o main_template.c -o bin/mergesort

pyramid_mergesort: main_template.c utils.o pyramid_mergesort.o
	${CMD} -DSORT_FUNCTION=pyramid_mergesort_wrapper utils.o pyramid_mergesort.o main_template.c -o bin/pyramid_mergesort

timsort: main_template.c utils.o timsort.o insertionsort
	${CMD} -DSORT_FUNCTION=timsort utils.o insertionsort.o timsort.o main_template.c -o bin/timsort

stdlib_qsort: main_template.c utils.o sort_functions
	${CMD} -DSORT_FUNCTION=qsort utils.o main_template.c -o bin/stdlib_qsort

stdlib_mergesort: main_template.c utils.o sort_functions
	${CMD} -DSORT_FUNCTION=mergesort utils.o main_template.c -o bin/stdlib_mergesort

stdlib_heapsort: main_template.c utils.o sort_functions
	${CMD} -DSORT_FUNCTION=heapsort utils.o main_template.c -o bin/stdlib_heapsort
