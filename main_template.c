#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "unistd.h"
#include "sys/stat.h"

#ifdef __APPLE__
#include "mach/mach_time.h"
#endif

#include "utils.h"

#include "insertionsort.h"
#include "mergesort.h"
#include "pyramid_mergesort.h"
#include "timsort.h"
#include "inplace_mergesort.h"
#include "recursive_mergesort.h"
#include "jsf_mergesort.h"


size_t working_set_size( size_t element_size, size_t nel );

int main( int argc, char* argv[] ) {

	if( argc != 3 ) {
		printf("Usage: %s infile outfile\n", argv[0] );
		exit(0);
	}
	
	const char* filename_in = argv[1];
	const char* filename_out = argv[2];
	say("Sorting file %s, writing to %s\n", filename_in, filename_out);
	
	widget* widgets = NULL;
	size_t count = read_widgets( filename_in, &widgets );
	say( "Read %zu things\n", count);
	say("widgets premerge %p\n", widgets);
	print_array( widgets, 0, count, PAD_SIZE );

#if defined(TESTING) || defined (VERBOSE)
	// copy for comparing later
	widget* widgets_copy = malloc( sizeof(widget)*count );
	if( widgets_copy == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	memcpy( widgets_copy, widgets, sizeof(widget)*count );
#endif

#ifdef __APPLE__
	uint64_t start, ticks;
	start = mach_absolute_time(); // returns ticks since last reboot

	SORT_FUNCTION( widgets, count, sizeof(widget), compare_widget );
// ## (widgets, count, sizeof(widget), compare_widget );

	ticks = mach_absolute_time() - start;

	// write count, ticks
	fprintf( stderr, "%zu,%llu\n", count, ticks );

#elif
	
	timespec requestStart, requestEnd;
	clock_gettime(CLOCK_REALTIME, &requestStart);

	SORT_FUNCTION(widgets, count, sizeof(widget), compare_widget );

	clock_gettime(CLOCK_REALTIME, &requestEnd);
	uint64_t nanos = ( requestEnd.tv_sec - requestStart.tv_sec ) * 1E9 + ( requestEnd.tv_nsec - requestStart.tv_nsec );

	// write count, nanos
	fprintf( stderr, "%zu,%llu\n", count, nanos );
#endif
	
	say( "widgets postmerge %p\n", widgets );
	print_array( widgets, 0, count, PAD_SIZE );

#ifdef TESTING

	for( int i=0; i<count-1; i++ ) {
		if( widgets[i].number > widgets[i+1].number ) {
			printf("Not sorted at %d/%d (%d/%d)\n", i, i+1, widgets[i].number, widgets[i+1].number);
			exit( EXIT_FAILURE );
		}
	}

#endif

#ifdef VERBOSE

	is_sorted( widgets, 0, count );

#endif
	
#if defined(TESTING) || defined(VERBOSE)

	contains_same_elements( widgets_copy, widgets, count );
	say( "widgets postmerge contain same elements as premerge.\n" );
	free( widgets_copy );

#endif

	write_widgets( widgets, count, filename_out );
	
	free( widgets );
	
	exit( EXIT_SUCCESS );
}
