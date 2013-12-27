CC=clang
CFLAGS=-Wall -O3 -pedantic 
CMD=${CC} ${CFLAGS}

clean:
	rm -f bin/* gen_random_ints *.o

all: gen_random_ints utils.o insertionsort.o stdlib_qsort.o 
	${CMD} utils.o insertionsort.o main_template.c -o bin/insertionsort
	${CMD} utils.o stdlib_qsort.o main_template.c -o bin/stdlib_qsort

verbose: *.c
	${CMD} -DVERBOSE -c utils.c
	${CMD} -DVERBOSE -c insertionsort.c
	${CMD} -DVERBOSE utils.o insertionsort.o main_template.c -o bin/insertionsort
	${CMD} -DVERBOSE utils.o stdlib_qsort.o main_template.c -o bin/stdlib_qsort

insertionsort: main_template.c utils.o insertionsort.o
	${CMD} utils.o insertionsort.o main_template.c -o bin/insertionsort

stdlib_qsort: main_template.c utils.o stdlib_qsort.o
	${CMD} utils.o stdlib_qsort.o main_template.c -o bin/stdlib_qsort

utils.o: utils.h utils.c
	${CMD} -c utils.c
	
insertionsort.o: insertionsort.c
	${CMD} -c insertionsort.c

stdlib_qsort.o: stdlib_qsort.c
	${CMD} -c stdlib_qsort.c

