#include <stdint.h>

#define SUPPRESS_STDOUT \
		int SUPPRESS_bak, SUPPRESS_new; \
		fflush(stdout); \
		SUPPRESS_bak = dup(1); \
		SUPPRESS_new = open("/dev/null", O_WRONLY); \
		dup2(SUPPRESS_new, 1); \
		close(SUPPRESS_new);

#define RETURN_STDOUT \
		fflush(stdout); \
		dup2(SUPPRESS_bak, 1); \
		close(SUPPRESS_bak);


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

// just a thing we can set to (almost) any size so we can pick any working set
// we're going to sort these by number, but the padding size is pickable (-DPAD_SIZE=N)
// Well, that would of course be naive :)
// Due to structure alignment (http://c-faq.com/struct/align.esr.html) it's most likely going to be bigger
typedef struct {
	uint32_t number;
	char padding[PAD_SIZE];
} widget;

void shellsort( void* base, size_t nel, size_t width, comparator compare );

// check if a list is sorted in ascending order
void is_sorted( widget* widgets, int from, int to );

// read a bunch of integers from a file and returns the count
// exits on failure to malloc, open the file etc.
// should maybe return negative numbers and set errno instead
size_t read_numbers( const char* filename, int** numbers );

size_t read_widgets( const char* filename, widget** widgets );

int write_numbers( int* numbers, size_t count, const char* filename_out );

int write_widgets( widget* widgets, size_t count, const char* filename_out );

#ifdef VERBOSE
// just printf but not doing anthing if not VERBOSE
void say( const char* format, ... );

void print_array( widget* widgets, int from, int to, int width );

#else

#define say(...) // nop

#define print_array(...) // nop

#endif

void contains_same_elements( widget* a, widget* b, size_t count);

int compare_int(const void* a, const void* b);

int compare_widget(const void* a, const void* b);

// reads the Time Stamp Counter
// for getting CPU cycles
// http://en.wikipedia.org/wiki/Time_Stamp_Counter
// because	
uint64_t read_TSC();


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
