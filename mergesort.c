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
	
	// first use the inner sort to sort all chunks of inner_sort_width
	int from = 0;
	while( from + (inner_sort_width*width) < nel ) {
		say( "inner sort from %d to %d\n", from/width, (from/width)+inner_sort_width );
		inner_sorter( list+from, inner_sort_width, width, compare );
		from += inner_sort_width*width;
	}
	say("inner sort from %d to %d\n", from/width, nel );
	inner_sorter( list+from, nel, width, compare );

	print_array( base, 0, nel, 8 );
	
	// now perform the merge

	char* buf = malloc( nel*width );
	if( buf == NULL ) {
		perror("malloc()");
		exit(0);
	}

	say( "buf: %p, base: %p\n", buf, base );
	char* swap;
	int m = 0;
	int merge_length = inner_sort_width; // start with the sublists that are sorted
	int merges_done = 0;
	while( merge_length < nel ) {

		// merge k pairs of size merge_length
		// possible optimization: if we have 24 items and merge_length is 4, we merge 4+4, 4+4, 4+4, and then 8+8, 8
		// so that last 8 gets merged agian (but effectively only copied), 
		// we could memcpy those instead of going through the while loops (but we'd have another branch)
		for(int start = 0; start < nel; start += 2*merge_length) {
			// use indices for the Left of the pair and the Right of the pair
			int L = start*width;
			int R = (start + merge_length)*width;
			int L_end = MIN(R-width, (nel-1)*width);
			int R_end = MIN(R+(merge_length*width)-width, (nel-1)*width);
			int to = 0;
			say( "Merging 2x%d [%d to %d]\n", merge_length, L/width, R_end/width );
			print_array( (int*)(buf+L), (R_end-L)/width, nel, 8 );
			
			while( start + to <= R_end ) { // we always know how many elements to merge

				// copy as many from the left pair as we can
				// (short-circuit evaluation means we never acces list[R] if R is out of bounds)
				m = 0;
				while( L <= L_end && (R > R_end || compare( list+L, list+R ) < 0 ) ) {
					L++;
					m += width;
				}
				memcpy( buf+to, list, m );
				to += m;
				
				// then copy as many as we can from the right pair
				m = 0;
				while( R <= R_end && (L > L_end || compare( list+R, list+L ) < 0 ) ) {
					R++;
					m += width;
				}
				memcpy( buf+to, list, m );
				to += m;
				
			}

			say( "buf: %p, base: %p\n", buf, base );
			print_array( (int*)buf, 0, nel, 8 ); // TODO limit to just merged


		}
		// reset the source array for merging
		swap = list;
		list = buf;
		buf = swap;
		
		merge_length *= 2;
		merges_done++;
	}

	say( "merge done (%d) buf: %p, base: %p\n", merges_done, buf, base );

	free(buf);
}

void sort_function( void* base, size_t nel, size_t width, comparator compare ) {
	
	merge_sort( base, nel, width, compare, 4, shellsort );
	
}
