#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )


void _mergesort( char* from, char* to, char* buf, size_t width, comparator compare ) {

	say("\nSorting %d elements\n", (to-from)/width );
	print_array( (widget*)from, 0, (to-from)/width, (to-from)/width );

	char* mid = from + ((to-from) / (2*width))*width; // ugly
	say("from-mid-to = 0 - %d - %d\n", (mid-from)/width, (to-from)/width );

	// only sort the smaller chunks if there is at least 1 element
	if( mid - width > from ) {
		_mergesort( from, mid, buf, width, compare );
	}

	if( mid + width < to ) {
		_mergesort( mid, to, buf, width, compare );
	}

	// left and right ranges are now sorted, merge them
	char* left = from;
	char* right = mid;
	char* b = buf;
	say("Merging %d elements\n", (to-from)/width );
	print_array( (widget*)from, 0, (to-from)/width, (to-from)/width );
	say("Starting merge with L=%d and R=%d\n", ((widget*)left)->number, ((widget*)right)->number );
	while( left < mid && right < to ) {
		
		if( compare( left, right ) <= 0 ) {
			memcpy( b, left, width );
			left += width;
		} else {
			memcpy( b, right, width );
			right += width;
		}
		
		b += width;
	}
	
	// copy remainders
	if( left < mid ) {
		memcpy( b, left, mid-left );
	}
	if( right < to ) {
		memcpy( b, right, to-right );
	}
	
	// copy from buf back to base
	memcpy( from, buf, to-from );
	say("Result:\n");
	print_array( (widget*)from, 0, (to-from)/width, (to-from)/width );
	
	
}

void recursive_mergesort( void* base, size_t nel, size_t width, comparator compare ) {

	char* buf = malloc( nel*width );
	if( buf == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}

	_mergesort( (char*)base, (char*)base + nel*width, buf, width, compare );

	free( buf );
}
