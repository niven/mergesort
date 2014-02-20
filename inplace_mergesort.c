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
		if( compare( first, first + width ) == 1 ) { // if the first is bigger than the second
			memcpy( temp, first + width, width ); // save second
			memcpy( first + width, first, width ); // overwrite second with first
			memcpy( first, temp, width ); // copy second to first	
		}
	}
	free( temp );
	say("Merge step 1 done, all pairs now sorted\n");
	print_array( (widget*)base, 0, nel, 16 );

//--- Second: perform the merge

	size_t merge_width = 2; // start with the pairs that are sorted

	while( merge_width < length ) {
	   
		say("Merge width %zu\n", merge_width);
		
		// merge k pairs of size mergeLength
		for( size_t start=0; start<length; start += 2*merge_width ) {
			// use indices for the Left of the pair and the Right of the pair
			L := start
			L_end := min(start + mergeLength - 1, length - 1)
			R := min(L_end + 1, length - 1)
			R_end := min(R + mergeLength - 1, length - 1)

            // if we're merging chunks of size 8, but we're at the end of the array and have like 5 elements left
            // it means we're done :)
            if L_end >= R {
                continue
            }

            if debug {
    	      fmt.Printf("Merging subarray (%v,%v)-(%v,%v) = %v\n", L, L_end,R, R_end )
            }
    	   // now merge in place, and since we're slicing we need to offset the midpoint
            merge_slice_in_place( n, L, R_end, R )
		}
		
		mergeLength *= 2;
	}


}
