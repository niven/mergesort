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
	char* offset;

	
	say("Merging %d elements\n", (to-from)/width );
	print_array( (widget*)from, 0, (to-from)/width, (to-from)/width );
	say("Starting merge with L=%d and R=%d\n", ((widget*)left)->number, ((widget*)right)->number );
	while( left < mid && right < to ) {
		
		offset = left;
		while( offset < mid && compare( offset, right ) <= 0 ) {
			offset += width;
		}
		memcpy( b, left, offset-left ); // could be 0 but that is fine
		b += offset - left;
		left = offset;
		
		if( left == mid ) {
			break;
		}
		
		offset = right;
		while( offset < to && compare( left, offset ) > 0 ) {
			offset += width;
		}
		memcpy( b, right, offset-right );
		b += offset - right;
		right = offset;

	}
	
	// copy remainders
	if( left < mid ) {
		memcpy( b, left, mid-left );
	}
	if( right < to ) {
		to -= to-right;
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
