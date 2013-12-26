#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "unistd.h"
#include "sys/stat.h"

#include "mach/mach_time.h"

#include "utils.h"

void insertionsort( void* base, size_t nel, size_t width, comparator compare ) {
	
	char* list = (char*)base;
	void* value = malloc( width );
	if( value == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	
	int hole_index;
	for(int i=0; i<nel*width; i+=width ) {
		memcpy( value, list+i, width ); // take value and keep it safe
		hole_index = i; // this is where we took the value from, it's vacant
		
		say( "Moving value %d and creating a hole at %d\n", *(int*)value, hole_index );
		
		while( hole_index > 0 && compare( list+hole_index-width, value ) == 1 ) { // if elements are higher, we shift them 
			memcpy( list+hole_index, list+hole_index-width, width ); // move the element right
			hole_index -= width; // move the hole left
		}
		memcpy( list+hole_index, value, width );
		
		print_array( (int*)base, 0, nel, 8 );
	}
}


int main( int argc, char* argv[] ) {

	if( argc != 3 ) {
		puts("Usage: shellsort infile outfile");
		exit(0);
	}
	
	const char* filename_in = argv[1];
	const char* filename_out = argv[2];
	say("Sorting file %s, writing to %s\n", filename_in, filename_out);
	
	int* numbers = NULL;
	size_t count = read_numbers( filename_in, &numbers );
	say( "Read %zu numbers\n", count);
	say("numbers premerge %p\n", numbers);
	print_array( numbers, 0, count, 8 );

	unsigned long start, stop;
	start = mach_absolute_time();

	insertionsort( numbers, count, sizeof(int), compare_int );

	stop = mach_absolute_time();

	mach_timebase_info_data_t timebase_info;
	mach_timebase_info( &timebase_info );
	
	unsigned long elapsed_nano = (stop-start) * timebase_info.numer / timebase_info.denom;

	// write count,nano
	fprintf(stderr, "%ld,%ld\n", count,elapsed_nano);

	say( "numbers postmerge %p\n", numbers );
	print_array( numbers, 0, count, 8 );

	is_sorted( numbers, 0, count );
	
	write_numbers( numbers, count, filename_out );
	
	free( numbers );
	return 0;
}
