#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )


// sequence by Marcin Ciura
const int gaps[8] = {701, 301, 132, 57, 23, 10, 4, 1};

void shellsort( void* base, size_t nel, size_t width, comparator compare ) {


	char* list = (char*)base;
	void* value = malloc( width );
	if( value == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	int i, j, g;

	say( "Shellsorting " );
	print_array( base, 0, nel, nel ); // +1 silliness because end is inclusive
	
	// Do an insertion sort for each gap size.
	for( g=0; g<8; g++ ) {
		int gap = gaps[g];

		if( gap < nel ){
			say("Shellsort gap %d nel %d\n", gap, nel );
		}
		// i iterates over a virtual array defined by gapsize
		// so if gap=3 then i iterates n[3], n[6], n[9] (starting at element 2)
		// But we increment i by 1, so what gives?
		// Well, the sequence 3,6,9 + 4,7,10 + 5,8,11 is the same 
		// as 3,4,5,6,7,8,9,10,11 just in a different order and that saves a for loop
		// j iterates backwards from i in gapsize steps so the end result is the same 
		for (i = gap*width; i < nel*width; i += width ) { 
		    memcpy( value, list+i, width);
			
			say("Shellsort n[%d] = %d\n", i/width, *(int*)value );

			// now iterate over every "gapth" element back to the left, moving items until
			// we find one that is lower/equal than number_to_place
			// For the first iteration, either the second number is larger and we're done, or move it to the position of the first element
			// then we try element 3, move that left to its spot (since 1 and 2 are now sorted)
		    for (j = i-(gap*width); j >= 0 && compare( list+j, value ) == 1; j -= gap*width) {
		        memcpy( list+j+(gap*width), list+j, width );
		    }
	        memcpy( list+j+(gap*width), value, width );

			say( "Shellsort " );
			print_array( base, 0, nel, nel ); // +1 silliness because end is inclusive
		}
		
	}	
	
	free( value );
}

size_t read_numbers( const char* filename, int** numbers ) {
	
	FILE* in = fopen( filename, "rb" );
	if( in == NULL ) {
		perror("open()");
		exit(0);
	}
	
	// find out how many 4 byte ints
	struct stat st;
	int result = fstat( fileno(in), &st );
	if( result == -1 ) {
		perror("fstat()");
		fclose( in );
		exit(0);
	}
	size_t count = st.st_size / sizeof(int);
	say("Number of integers of size %ld bytes in file: %zu\n", sizeof(int), count);
	
	int* target = malloc( sizeof(int)*count );
	if( target ==  NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	size_t read = fread( target, sizeof(int), count, in );
	if( read != count ) {
		say("Read %zu ints, expected %zu\n", read, count);
		free( target );
		exit( EXIT_FAILURE );
	}	
	fclose( in );
	*numbers = target;
	
	return count;
}

size_t read_widgets( const char* filename, widget** widgets ) {
	
	FILE* in = fopen( filename, "rb" );
	if( in == NULL ) {
		perror("open()");
		exit(0);
	}
	
	// find out how many 4+PAD_SIZE byte widgets
	struct stat st;
	int result = fstat( fileno(in), &st );
	if( result == -1 ) {
		perror("fstat()");
		fclose( in );
		exit(0);
	}
	// finding out the count by dividing the size by sizeof(widget) won't work as that micht be larger than the members due to struct alignment
	size_t count = st.st_size / sizeof(widget);
	say("Size of file: %d bytes. Number of widgets of size %ld bytes in file: %zu\n", st.st_size, sizeof(widget), count);
	
	widget* target = malloc( sizeof(widget)*count );
	if( target ==  NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	size_t read = fread( target, sizeof(widget), count, in );
	if( read != count ) {
		say("Read %zu widget, expected %zu\n", read, count);
		free( target );
		exit( EXIT_FAILURE );
	}
	fclose( in );
	*widgets = target;
	
	return count;
}


int write_numbers( int* numbers, size_t count, const char* filename_out ) {

	FILE* out = fopen( filename_out, "wb" );
	if( out == NULL ) {
		perror("fopen()");
		return -1;
	}
	
	size_t written = fwrite( numbers, sizeof(int), count, out );
	if( written != count ) {
		perror("frwrite()");
		return -1;
	}	
	
	fclose( out );

	return 0;
}

int write_widgets( widget* widgets, size_t count, const char* filename_out ) {

	FILE* out = fopen( filename_out, "wb" );
	if( out == NULL ) {
		perror("fopen()");
		return -1;
	}
	
	size_t written = fwrite( widgets, sizeof(widget), count, out );
	if( written != count ) {
		perror("frwrite()");
		return -1;
	}	
	
	fclose( out );

	return 0;
}



#ifdef VERBOSE	

void say( const char* format, ... ) {
	va_list arglist;
	va_start( arglist, format );
	vprintf( format, arglist );
	va_end( arglist );
}

void print_array( widget* widgets, int from, int to, int width ) {

	if( to <= from ) {
		say("[nothing]\n");
		return;
	}
	say( "[%d - %d] [ ", from, MIN(from+width-1, to-1) );
	for(int i=from; i<to; i++) {
		say( "%3d:%.1s ", widgets[i].number, widgets[i].padding[0] == '\0' ? "___" : widgets[i].padding );
		if( i<to-1 && (i+1)%width==0 ) {
			say( "]\n[%d - %d] [ ", i+1, MIN(i+width,to-1) );
		}
	}
	say( "]\n" );

}

void is_sorted(widget* widgets, int from, int to) {

	for( int i=from; i<to-1; i++ ) {
		if( widgets[i].number > widgets[i+1].number ) {
			say("Not sorted at %d/%d (%d/%d)\n", i, i+1, widgets[i].number, widgets[i+1].number);
			exit( EXIT_FAILURE );
		}
	}

}

#endif	

// as a check to see if the original numbers array contains the same elements
// as the sorted one
void contains_same_elements( widget* a, widget* b, size_t count) {

	int xored_a = 0, xored_b = 0;
	size_t sum_a = 0, sum_b = 0;
	for(size_t i=0; i<count; i++) {
		sum_a += a[i].number;
		sum_b += b[i].number;

		xored_a ^= a[i].number;
		xored_b ^= b[i].number;
	}

	// so in theory the sort function could have a bug that results
	// in different values that sum and xor to the same.

	if( sum_a != sum_b || xored_a != xored_b ) {
		puts( "Arrays don't contain the same numbers" );
		print_array( a, 0, count, 8 );
		print_array( b, 0, count, 8 );
		exit( EXIT_FAILURE );
	}
}


/*
Compare 2 integers
	a > b: 1
	a == b: 0
	a < b: -1
*/
int compare_int(const void* a, const void* b) {

	if ( *(int*)a == *(int*)b )
		return 0;

	if (*(int*)a < *(int*)b)
		return -1;

	return 1;
}
/*
Compare 2 integers
	a > b: 1
	a == b: 0
	a < b: -1
*/

int compare_double(const void* a, const void* b) {

	if ( *(double*)a == *(double*)b )
		return 0;

	if (*(double*)a > *(double*)b)
		return -1;

	return 1;
}

/*
Compare 2 structs of type widget
	a > b: 1
	a == b: 0
	a < b: -1
*/
int compare_widget(const void* a, const void* b) {

	widget* wa = (widget*)a;
	widget* wb = (widget*)b;	

	if ( wa->number == wb->number )
		return 0;

	if ( wa->number < wb->number )
		return -1;

	return 1;
}

/*
 assembly code to read the TSC 
 (Time Stamp Counter)
*/
uint64_t read_TSC() {
  unsigned int hi, lo;
  __asm__ ("cpuid");
  __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}

// timsort needs a stack to keep the runs on
// (potentially nel/minrun items maybe? In practice less)


run* new_run( const void* address, const size_t nel ) {
	run* new_run = (run*)malloc( sizeof(run) );
	if( new_run == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	
	new_run->address = address;
	new_run->nel = nel;
	
	return new_run;
}

void push_run( run_node** stack, run* element ) {
	run_node* new_node = (run_node*)malloc( sizeof(run_node) );
	if( new_node == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	
	new_node->item = element;
	new_node->next = *stack;

	*stack = new_node;
}

run* pop_run( run_node** stack ) {
	
	if( stack == NULL ) {
		return NULL;
	}
	
	run_node* top = *stack;
	run* top_item = top->item;
	say("pop_run top_item %p / %zu\n", top_item->address, top_item->nel);
	*stack = (*stack)->next;

	free( top );
	return top_item;
}

// returns but not removes
run* peek_run( run_node* stack, size_t index ) {
	
	size_t i = 0;
	run_node* current = stack;
	while( i < index && current != NULL ) {
		current = current->next;
		i++;
	}
	
	if( current == NULL ) {
		return NULL;
	}
	
	return current->item;
}

void print_stack( run_node* stack ) {
	
	run_node *current = stack;
#ifdef VERBOSE	
	size_t count = 0;
#endif
	say("Run Stack:\n");
	while( current != NULL ) {
		say("[%zu] %zu elements starting at %p\n", count++, current->item->nel, current->item->address );
		current = current->next;
	}
	
}

char* append_str( char* prefix, const char* format, ... ) {
	
	char* formatted_string;
	
	va_list argslist;
	va_start( argslist, format );
	vasprintf( &formatted_string, format, argslist );
	va_end( argslist );
	
	size_t len = strlen( formatted_string );
	char* temp = realloc( prefix, strlen(prefix) + len + 1 ); // length of both + NUL
	if( temp == NULL ) {
		free( prefix );
		perror("realloc()");
		exit( EXIT_FAILURE );
	}
	
	temp = strcat( temp, formatted_string );
	free( formatted_string );
	
	return temp;
}

