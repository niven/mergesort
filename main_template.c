#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "unistd.h"
#include "sys/stat.h"

#include "mach/mach_time.h"

#include "utils.h"

void sort_function( void* base, size_t nel, size_t width, comparator compare );

int main( int argc, char* argv[] ) {

	if( argc != 3 ) {
		printf("Usage: %s infile outfile\n", argv[0] );
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

	// copy for comparing later
	int* numbers_copy = malloc( sizeof(int)*count );
	if( numbers_copy == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	memcpy( numbers_copy, numbers, sizeof(int)*count );

	unsigned long start, stop;
	start = mach_absolute_time();

	sort_function(numbers, count, sizeof(int), compare_int );

	stop = mach_absolute_time();

	mach_timebase_info_data_t timebase_info;
	mach_timebase_info( &timebase_info );
	
	unsigned long elapsed_nano = (stop-start) * timebase_info.numer / timebase_info.denom;

	// write count,nano
	fprintf(stderr, "%ld,%ld\n", count,elapsed_nano);

	say( "numbers postmerge %p\n", numbers );
	print_array( numbers, 0, count, 8 );

	is_sorted( numbers, 0, count );
	
	contains_same_elements( numbers_copy, numbers, count );
	say( "numbers postmerge contain same elements as premerge.\n" );

	write_numbers( numbers, count, filename_out );
	
	free( numbers );
	free( numbers_copy );
	return 0;
}
