#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )


// sequence by Marcin Ciura
const int gaps[8] = {701, 301, 132, 57, 23, 10, 4, 1};

// Sort an array a[start...end].
// end is inclusive (seems like a bad idea)
void shellsort( int* numbers, int start, int end ) {

	int i, j, g;

	say( "Shellsort " );
	print_array( numbers, start, end+1, end-start+1 ); // +1 silliness because end is inclusive
	
	// Do an insertion sort for each gap size.
	for( g=0; g<8; g++ ) {
		int gap = gaps[g];

		if( gap <= end-start+1 ){
			say("Shellsort gap %d (%d,%d)\n", gap, start+gap, end );
		}
		// i iterates over a virtual array defined by gapsize
		// so if gap=3 then i iterates n[3], n[6], n[9] (starting at element 2)
		// But we increment i by 1, so what gives?
		// Well, the sequence 3,6,9 + 4,7,10 + 5,8,11 is the same 
		// as 3,4,5,6,7,8,9,10,11 just in a different order and that saves a for loop
		// j iterates backwards from i in gapsize steps so the end result is the same 
		for (i = start+gap; i <= end; i++ ) { 
		    int number_to_place = numbers[i];

			say("Shellsort n[%d] = %d\n", i, number_to_place );

			// now iterate over every "gapth" element back to the left, moving items until
			// we find one that is lower/equal than number_to_place
			// For the first iteration, either the second number is larger and we're done, or move it to the position of the first element
			// then we try element 3, move that left to its spot (since 1 and 2 are now sorted)
		    for (j = i-gap; j >= start && numbers[j] > number_to_place; j -= gap) {
		        numbers[j+gap] = numbers[j];
		    }
		    numbers[j+gap] = number_to_place;

			say( "Shellsort " );
			print_array( numbers, start, end+1, end-start+1 ); // +1 silliness because end is inclusive
		}
		
	}
	
}

void is_sorted(int* numbers, int from, int to) {

	for( int i=from; i<to-1; i++ ) {
		if( numbers[i] > numbers[i+1] ) {
			say("Fail at %d/%d (%d/%d)\n", i, i+1, numbers[i], numbers[i+1]);
			exit(0);
		}
	}

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

void say( const char* format, ... ) {
#ifdef VERBOSE	
	va_list arglist;
	va_start( arglist, format );
	vprintf( format, arglist );
	va_end( arglist );
#endif	
}

void print_array( int* numbers, int from, int to, int width ) {
#ifdef VERBOSE	
	say( "[%d - %d] [ ", from, MIN(from+width-1, to-1) );
	for(int i=from; i<to; i++) {
		say( "%3d ", numbers[i] );
		if( i<to-1 && (i+1)%width==0 ) {
			say( "]\n[%d - %d] [ ", i+1, MIN(i+width,to-1) );
		}
	}
	say( "]\n" );
#endif	
}


int compare_int(const void* a, const void* b) {

	if ( *(int*)a == *(int*)b )
		return 0;

	if (*(int*)a < *(int*)b)
		return -1;

	return 1;
}

