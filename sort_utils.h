#ifndef SORT_UTILS
#define SORT_UTILS

/*
Shellsort the numbers using the Marcin Ciura sequence.

http://sun.aei.polsl.pl/~mciura/publikacje/shellsort.pdf

The numbers are sorted in place

Rather than supply a count this specifies a start and end making this suitable
for sorting subsections of an array and also for embedding it in another sort.

An alternative would be to pass a pointer to the offset and a count but I like this better.

This sorts the array numbers in ascending order from [start, end] inclusive. (end being inclusive is not very intuitive though)
*/

typedef int (*comparator)(const void* a, const void* b);

typedef void (*sorter)( void* base, size_t nel, size_t width, comparator compare );

void shellsort( void* base, size_t nel, size_t width, comparator compare );


// read a bunch of integers from a file and returns the count
// exits on failure to malloc, open the file etc.
// should maybe return negative numbers and set errno instead
size_t read_numbers( const char* filename, int** numbers );

int write_numbers( int* numbers, size_t count, const char* filename_out );

int compare_int(const void* a, const void* b);

typedef struct {
	const void* address;
	size_t nel;
} run;

// our stack is a linked list
typedef struct run_node {
	run* item;
	struct run_node* next;
} run_node;


run* new_run( const void* address, const size_t nel );

void push_run( run_node** stack, run* element );

run* pop_run( run_node** stack );

run* peek_run( run_node* stack, size_t index );

void print_stack( run_node* stack );

#endif
