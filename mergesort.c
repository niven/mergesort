#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "assert.h"
#include "time.h"
#include "unistd.h"
#include "sys/stat.h"

#include "mach/mach_time.h"

#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

void merge_sort(void* base, size_t nel, size_t width, comparator compare, size_t inner_sort_width, sorter inner_sorter) {
	

	char* list = (char*) base;
	// arrays of size 0 and size 1 are always sorted
	if( nel < 2 ) {
		return;
	}

	// try to allocate a buffer first, because there is no point in sorting all blocks
	// and then failing because we have no memory to merge anyway
	char* buf = malloc( nel*width );
	if( buf == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	
	// first use the inner sort to sort all chunks of inner_sort_width
	int from = 0;
	while( from + inner_sort_width <= nel ) {
		say( "inner sort [%d - %d]\n", from, from+inner_sort_width-1 );
		inner_sorter( list+(from*width), inner_sort_width, width, compare );
		from += inner_sort_width;
	}
	say("inner sort remainder [%d - %d]\n", from, nel-1 );
	inner_sorter( list+(from*width), nel-from, width, compare );

	print_array( base, 0, nel, 8 );
	
	// now perform the merge


	say( "buf: %p, base: %p\n", buf, base );
	char* swap;
	int m = 0;
	int merge_length = inner_sort_width; // start with the sublists that are sorted
	int merges_done = 0;
	while( merge_length < nel ) {

		// merge k pairs of size merge_length
		// start, to and merge_length are related to the number of elements
		// L, L_end, R, R_end, to are offsets into base/buf
		for(int start_index = 0; start_index < nel; start_index += 2*merge_length) {
			// use indices for the Left of the pair and the Right of the pair
			int end_index = MIN(start_index + (2*merge_length) -1 , nel-1);
			int L = start_index*width;
			int R = (start_index + merge_length)*width;
			int L_end = MIN(start_index+merge_length-1, end_index) * width; // 1 element before R starts, or the end of the array
			int R_end = end_index * width; // 
			int to_index = 0;
			say( "Merging 2x%d [%d - %d] with [%d - %d] indices [%d - %d]\n", merge_length, L, L_end, R, R_end, start_index, end_index );
			print_array( (widget*)list, L/width, L_end/width +1, 8 );
			print_array( (widget*)list, R/width, R_end/width +1, 8 );
			
			while( start_index + to_index <= end_index ) { // we always know how many elements to merge

				// copy as many from the Left as we can
				m = L;
				say("L=%d compare( %d, %d ) < 0\n", L, *(int*)(list+(L)), *(int*)(list+(R)) );
				while( L <= L_end && (R > R_end || compare( list+(L), list+(R) ) <= 0 ) ) {
					L += width;
					say("L=%d compare( %d, %d ) < 0\n", L, *(int*)(list+(L)), *(int*)(list+(R)) );
				}
				say("Copying %d bytes (%d elements) to buf+%d\n", L-m, (L-m)/width );
				memcpy( buf + (start_index+to_index)*width, list + m, L-m );
				to_index += (L-m)/width;
				print_array( (widget*)buf, 0, end_index+1, 8 );
				
				// then copy as many as we can from the right pair
				m = R;
				say("R=%d compare( %d, %d ) < 0\n", R, *(int*)(list+(R)), *(int*)(list+(L)) );
				while( R <= R_end && (L > L_end || compare( list+(R), list+(L) ) <= 0 ) ) {
					R += width;
					say("R=%d compare( %d, %d ) < 0\n", R, *(int*)(list+(R)), *(int*)(list+(L)) );
				}
				say("Copying %d bytes (%d elements) to buf\n", (R-m), (R-m)/width );
				memcpy( buf + (start_index+to_index)*width, list+m, R-m );
				to_index += (R-m)/width;
				print_array( (widget*)buf, 0, end_index+1, 8 );
				
				say("start_index %d, to_index %d, end_index %d\n", start_index, to_index, end_index);
			}

			print_array( (widget*)buf, start_index, end_index+1, 8 );

		}
		// reset the source array for merging
		swap = list;
		list = buf;
		buf = swap;
		
		merge_length *= 2;
		merges_done++;
	}

	say( "merge done (%d) buf: %p, base: %p, list %p\n", merges_done, buf, base, list );

	// unfortunate result of the stdlib sort interface: we might end up with the end result in buf
	// and not in wherever base points to. In that case we copy over everything :(
	if( list != base ) {
		memcpy( base, list, nel*width );
		free( list );
	} else {
		free( buf );
	}

}

void sort_function( void* base, size_t nel, size_t width, comparator compare ) {
	
	int elements_per_block = 4;
	const char* env_block_width = getenv( "SORTER_BLOCK_WIDTH" );
	if( env_block_width != NULL ) {
		elements_per_block = atoi( env_block_width );
	}
	say("Using SORTER_BLOCK_WIDTH %d\n", elements_per_block);
	
	merge_sort( base, nel, width, compare, elements_per_block, shellsort );
	
}
