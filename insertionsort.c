#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "utils.h"

void sort_function( void* base, size_t nel, size_t width, comparator compare ) {
	
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
	
	free( value );
}
