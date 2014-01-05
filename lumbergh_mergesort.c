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

size_t merge_down( char* in, char* buf, size_t nel, size_t width, comparator compare );
size_t merge_up( char* in, char* buf, size_t nel, size_t width, comparator compare );

size_t merge_range(char* left, char* right, char* end, char* to, size_t width, comparator compare);

void lumbergh_merge_sort(void* base, size_t nel, size_t width, comparator compare) {
	
	char* in = (char*) base;
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

	say( "buf: %p, base: %p\n", buf, base );

	size_t swaps = 1;
	char *to;
	int sentinel = 0;
	while( swaps != 0 && sentinel++ < 1 ) {
		
		swaps = 0;
		
		// merge halves, quarters, eights etc "down"
		char* copy = malloc( nel*width );
		if( copy == NULL ) {
			perror("malloc()");
			exit( EXIT_FAILURE );
		}
		memcpy( copy, in, nel*width );
		swaps += merge_down( in, buf, nel, width, compare );
		say("Merged down with %d swaps\n", swaps);
		print_array( (widget*)buf, 0, nel, 8 );
		contains_same_elements( (widget*)copy, (widget*)buf, nel );
		
		// now we don't know where the final result was in since in/buf swap
		// so we figure it out with math. Well, later on. Now we have merge_down() just copy stuff
		
		// merge "up"
		memcpy( copy, buf, nel*width );
		swaps += merge_up( buf, in, nel, width, compare );
		say("Merged down with %d swaps\n", swaps);
		print_array( (widget*)in, 0, nel, 8 );
		contains_same_elements( (widget*)copy, (widget*)in, nel );


//		to = in;
	}

}

size_t merge_down( char* in, char* buf, size_t nel, size_t width, comparator compare ) {
	
	say("Merge Down\n");
	int swaps = 0;
	
	int merge_length = nel / 2;
	char* temp = NULL;
	char* original_in = in;
	while( merge_length >= 1 ) {

		char* copy = malloc( nel*width );
		if( copy == NULL ) {
			perror("malloc()");
			exit( EXIT_FAILURE );
		}
		memcpy( copy, in, nel*width );
	
		say("Merging from source\n");
		print_array( (widget*)in, 0, nel, merge_length*2*2 );
		for(int offset_left=0; offset_left<nel-merge_length+1; offset_left+=2*merge_length) {
			say("\tmerging interval [%d-%d] with [%d-%d]\n", offset_left, offset_left+merge_length-1, MIN(offset_left+merge_length, nel-1), MIN(offset_left+merge_length+merge_length-1, nel-1) );
			char* left_start = in+(offset_left*width);
			char* right_start = in + ( MIN(offset_left+merge_length, nel-1)*width );
			char* end = in + (MIN(offset_left+merge_length+merge_length-1, nel-1)*width);
			swaps += merge_range( left_start, right_start, end, buf + (offset_left*width), width, compare );
			say("Merge step done, buf:\n");
			print_array( (widget*)buf, 0, nel, merge_length*2*2 );
		
			// as soon as merge_length==1 we could special case and just perform a swap if needed
		}
		say("After merging ranges in/buf\n");
		print_array( (widget*)in, 0, nel, merge_length*2*2 );
		print_array( (widget*)buf, 0, nel, merge_length*2*2 );

		contains_same_elements( (widget*)copy, (widget*)buf, nel );

		temp = in;
		in = buf;
		buf = temp;
		
		merge_length /= 2;
	}

	say("Merge down result\n");
	print_array( (widget*)in, 0, nel, 8 );

	
	return swaps;
}

size_t merge_up( char* in, char* buf, size_t nel, size_t width, comparator compare ) {
	
	say("Merge Up\n");
	int swaps = 0;
	
	int merge_length = 2;
	char* temp = NULL;
	char* original_in = in;
	while( merge_length < nel ) {

		char* copy = malloc( nel*width );
		if( copy == NULL ) {
			perror("malloc()");
			exit( EXIT_FAILURE );
		}
		memcpy( copy, in, nel*width );
	
		say("Merging from source\n");
		print_array( (widget*)in, 0, nel, merge_length*2*2 );
		for(int offset_left=0; offset_left<nel-merge_length+1; offset_left+=2*merge_length) {
			say("\tmerging interval [%d-%d] with [%d-%d]\n", offset_left, offset_left+merge_length-1, MIN(offset_left+merge_length, nel-1), MIN(offset_left+merge_length+merge_length-1, nel-1) );
			char* left_start = in+(offset_left*width);
			char* right_start = in + ( MIN(offset_left+merge_length, nel-1)*width );
			char* end = in + (MIN(offset_left+merge_length+merge_length-1, nel-1)*width);
			swaps += merge_range( left_start, right_start, end, buf + (offset_left*width), width, compare );
			say("Merge step done, buf:\n");
			print_array( (widget*)buf, 0, nel, merge_length*2*2 );
		
			// as soon as merge_length==1 we could special case and just perform a swap if needed
		}
		say("After merging ranges in/buf\n");
		print_array( (widget*)in, 0, nel, merge_length*2*2 );
		print_array( (widget*)buf, 0, nel, merge_length*2*2 );

		contains_same_elements( (widget*)copy, (widget*)buf, nel );

		temp = in;
		in = buf;
		buf = temp;
		
		merge_length *= 2;
	}

	say("Merge down result\n");
	print_array( (widget*)in, 0, nel, 8 );

	
	return swaps;
}

size_t merge_range(char* left, char* right, char* end, char* to, size_t width, comparator compare) {
	int swaps = 0;


	char* start = left;
	char* to_start = to;
	int merge_length = (end-left)/(width*2)+1;
	// clear so debug output is nicer
	memset( to, 0, 2*merge_length*width );

	say("Merging 2x%d to %p\n", merge_length, to);
	print_array( (widget*)left, 0, merge_length, merge_length );
	print_array( (widget*)right, 0, merge_length, merge_length );
	print_array( (widget*)to, 0, merge_length*2, merge_length );
	while( right <= end ) {
		say("comparing left %d with right %d -> %d\n", *(int*)left, *(int*)right, compare( left, right ));
		if( compare( left, right ) <= 0 ) {
			memcpy( to, left, width );
			left += width;

			// special case: left might run out, then we bump right by 1 element
			if( left >= start + merge_length*width ) {
				left = right;
				right += width; // this could make right point past end, but the while will terminate and then we copy only from left
				say("Ran out of left array, set left to %d and bumped right to %d\n", *(int*)left, *(int*)right);
			}

		} else {
			memcpy( to, right, width );
			right += width;
			swaps++;
		}
		to += width;
	
		print_array( (widget*)to_start, 0, merge_length*2, merge_length );
//		say("swaps done %d\n", swaps);
	}
	// now we ran out of right, copy over any left if remaining
	if( left < start + merge_length*width ) { // stuff from left
		memcpy( to, left, (start + merge_length*width) - left );
		swaps += ((start + merge_length*width) - left)/width;
	} else { // we bumped right, so there is only one remaining
		memcpy( to, left, width );
	}
	print_array( (widget*)to_start, 0, merge_length*2, merge_length );
	
	return swaps;
	
}

void sort_function( void* base, size_t nel, size_t width, comparator compare ) {
	
	lumbergh_merge_sort( base, nel, width, compare );
	
}

size_t working_set_size( size_t element_size, size_t nel ) {
	return 2 * element_size * nel;
}

