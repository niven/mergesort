/*
Shellsort the numbers using the Marcin Ciura sequence.

http://sun.aei.polsl.pl/~mciura/publikacje/shellsort.pdf

The numbers are sorted in place

Rather than supply a count this specifies a start and end making this suitable
for sorting subsections of an array and also for embedding it in another sort.

An alternative would be to pass a pointer to the offset and a count but I like this better.

This sorts the array numbers in ascending order from [start, end] inclusive. (end being inclusive is not very intuitive though)
*/

typedef void (*sorter)(int* numbers, int start, int end);

typedef int (*comparator)(const void* a, const void* b);

void shellsort( int* numbers, int start, int end );

// check if a list is sorted in ascending order
void is_sorted( int* numbers, int from, int to );

// read a bunch of integers from a file and returns the count
// exits on failure to malloc, open the file etc.
// should maybe return negative numbers and set errno instead
size_t read_numbers( const char* filename, int** numbers );

int write_numbers( int* numbers, size_t count, const char* filename_out );

#ifdef VERBOSE
// just printf but not doing anthing if not VERBOSE
void say( const char* format, ... );

void print_array( int* numbers, int from, int to, int width );

#endif

#ifndef VERBOSE

#define say(...) // nop

#define print_array(...) // nop

#endif

void contains_same_elements( int* a, int* b, size_t count);


int compare_int(const void* a, const void* b);
