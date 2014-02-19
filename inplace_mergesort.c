#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

/*
Non-recursive mergesort without inner sort that merges in place.
*/
void inplace_mergesort(void* base, size_t nel, size_t width, comparator compare) {

	if( nel <= 1 ) {
		return;
	}

//--- First: divide the array in pairs and sort those

	char* list = (char*)base;
// (instead of pairs you could pick a larger number and use a different sorting algorithm, but we're doing that elsewhere)

// for a list with an odd number of elements we don't need to sort the remaining single element.
	size_t pairs = nel / 2; // (implicit floor)
	char* temp = malloc( width );
	if( temp == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	char* first;
	for( size_t i=0; i<2*pairs; i+=2 ) {
		first = list + i*width; // first element of the pair
		if( compare( first + width, first ) == 1 ) { // if the second is bigger than the first
			memcpy( temp, first + width, width ); // save second
			memcpy( first + width, first, width ); // overwrite second with first
			memcpy( first, first + width, width ); // copy second to first	
		}
	}
	free( temp );
	say("Merge step 1 done, all pairs now sorted\n");
	print_array( (widget*)base, 0, nel, 16 );


}
