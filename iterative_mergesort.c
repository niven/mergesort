#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

void iterative_mergesort( void* base, size_t nel, size_t width, comparator compare ) {

	char* temp = malloc( width );
	if( temp == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	char* buf = malloc( nel*width );
	if( buf == NULL ) {
		perror("malloc()");
		free( temp );
		exit( EXIT_FAILURE );
	}


	char* from = base;
	char* to = from + nel*width;
	
	say("\nSorting %d elements\n", nel );
	print_array( (widget*)from, 0, nel, nel );

	// sort all pairs in place
	for( from=base; from < to-width; from += 2*width ) {
		say("Comparing %d with %d = %d\n", *(int*)from, *(int*)(from+width), compare( from, from+width ) );
		if( compare( from, from+width) > 0 ) {
			memcpy( temp, from, width );
			memcpy( from, from+width, width );
			memcpy( from+width, temp, width );
		}
	}
	from = base;

	say("All pairs sorted.\n");
	print_array( (widget*)from, 0, nel, nel );

	char* mid;
	char* end;
	char* offset;
	char* left;
	char* right;
	char* b;
	for( size_t merge_range = 2; merge_range < nel; merge_range *= 2 ) { // merge 2,4,8,.. ranges

		say("Merging all chunks of size %zu\n", merge_range);
	
		for( from = base; from < to; from += 2*merge_range*width ) { // go over the entire array with a window of 2*merge_range
			
			end = MIN(from + 2*merge_range*width, to);
			mid = from + ((end-from) / (2*width))*width ; // ugly
			right = mid;
			left = from;
			b = buf;
			say("start-mid/right-end = %d - %d/%d - %d = [%d, %d, %d->]\n", 
				(from-(char*)base)/width, (mid-(char*)base)/width, (right-(char*)base)/width, (end-(char*)base)/width, 
				*(int*)left, *(int*)mid, *(int*)(end-width)
			);
			print_array( (widget*)from, 0, (end-from)/width, merge_range );
			memset( buf, 0, end-left );
			while( left < mid && right < end ) {

				offset = left;
				while( offset < mid && compare( offset, right ) <= 0 ) {
					offset += width;
				}
				memcpy( b, left, offset-left ); // could be 0 but that is fine
				b += offset - left;
				left = offset;

				say("\nLC l=[%s] m=[%s] r=[%s] e=[%s]\n", sfw(left), sfw(mid), sfw(right), sfw(end));
				print_array( (widget*)buf, 0, (end-from)/width, merge_range );
		
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

				say("\nRC l:%p m:%p r:%p e:%p\n", left, mid, right, end);
				print_array( (widget*)buf, 0, (end-from)/width, merge_range );

			}
			say("\nXW l:%p m:%p r:%p e:%p\n", left, mid, right, end);
			// copy remainders
			if( left < mid ) {
				memcpy( b, left, mid-left );
			}
			// instead of copying the trailing right hand array to the buffer and then copying everything back
			// we can just shorten the entire array, saving on copying to and then back.
			if( right < end ) {
				end -= end-right;
			}
			
			// copy the buffer back to the original array
			memcpy( from, buf, end-from );
			say("Result:\n");
			print_array( (widget*)buf, 0, (end-from)/width, merge_range );
			
		}
	
						
	
	}

	/*
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
	// instead of copying the trailing right hand array to the buffer and then copying everything back
	// we can just shorten the entire array, saving on copying to and then back.
	if( right < to ) {
		to -= to-right;
	}
	
	// copy from buf back to base
	memcpy( from, buf, to-from );
	say("Result:\n");
	print_array( (widget*)from, 0, (to-from)/width, (to-from)/width );
*/	
	free( temp );
	free( buf );

	free_allocated_strings(); // free sfw() strings
	
}

