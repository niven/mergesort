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
	
	widget* widgets = NULL;
	size_t count = read_widgets( filename_in, &widgets );
	say( "Read %zu things\n", count);
	say("widgets premerge %p\n", widgets);
	print_array( widgets, 0, count, 8 );

	// copy for comparing later
	widget* widgets_copy = malloc( sizeof(widget)*count );
	if( widgets_copy == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	memcpy( widgets_copy, widgets, sizeof(widget)*count );

	unsigned long start, ticks;
	start = mach_absolute_time(); // returns ticks since last reboot

	sort_function(widgets, count, sizeof(int), compare_widget );

	ticks = mach_absolute_time() - start;

	// write count, ticks
	fprintf( stderr, "%ld,%ld\n", count, ticks );

	say( "widgets postmerge %p\n", widgets );
	print_array( widgets, 0, count, 8 );

	is_sorted( widgets, 0, count );
	
	contains_same_elements( widgets_copy, widgets, count );
	say( "widgets postmerge contain same elements as premerge.\n" );

	//write_numbers( numbers, count, filename_out );
	
	free( widgets );
	free( widgets_copy );
	
	exit( EXIT_SUCCESS );
}
