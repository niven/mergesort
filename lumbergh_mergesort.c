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
	size_t swaps_up = 0;
	int sentinel = 0;
	while( swaps != 0 && sentinel++<2 ) {
		
		swaps = 0;
		swaps_up = 0;
		// merge halves, quarters, eights etc "down"
		char* copy = malloc( nel*width );
		if( copy == NULL ) {
			perror("malloc()");
			exit( EXIT_FAILURE );
		}
		memcpy( copy, in, nel*width );
		swaps += merge_down( in, buf, nel, width, compare );
		say("Merged down with %d swaps\n", swaps);
		print_array( (widget*)buf, 0, nel, nel );
		contains_same_elements( (widget*)copy, (widget*)buf, nel );
		
		// now we don't know where the final result was in since in/buf swap
		// so we figure it out with math. Well, later on. Now we have merge_down() just copy stuff
		
		// merge "up"
		memcpy( copy, buf, nel*width );
		swaps_up += merge_up( buf, in, nel, width, compare );
		say("Merged up with %d swaps\n", swaps_up);
		swaps += swaps_up;
		print_array( (widget*)in, 0, nel, nel );
		contains_same_elements( (widget*)copy, (widget*)in, nel );

	}
	say("sentinal %d\n", sentinel);
}

size_t merge_down( char* in, char* buf, size_t nel, size_t width, comparator compare ) {
	
	say("Merge Down\n");
	int swaps = 0;
	
	int merge_length = nel / 2;
	char* temp = NULL;
	int offset_left = 0;
	while( merge_length >= 1 ) {

		char* copy = malloc( nel*width );
		if( copy == NULL ) {
			perror("malloc()");
			exit( EXIT_FAILURE );
		}
		memcpy( copy, in, nel*width );
	
		say("Merging length %d from source (%d)\n", merge_length, nel);
		print_array( (widget*)in, 0, nel, nel );
		memset( buf, 0, nel*width ); // for debug
		for(offset_left=0; offset_left<nel-merge_length+1; offset_left+=2*merge_length) {
			char* left_start = in+(offset_left*width);
			char* right_start = in + ( MIN(offset_left+merge_length, nel-1)*width );
			char* end = in + (MIN(offset_left+merge_length+merge_length-1, nel-1)*width);
			say("rest: %d \n", nel - (end-in)/width -1 );
			if( nel - (end-in)/width -1 < merge_length ) {
				end = in + (nel-1)*width;
				say("Taking remainder, end points to %d\n", *(int*)end);
			} 
			say("Merging interval [%d-%d] with [%d-%d]\n", (left_start-in)/width, (right_start-in)/width-1, (right_start-in)/width, (end-in)/width );
			print_array( (widget*)left_start, 0, (right_start-left_start)/width, merge_length );
			print_array( (widget*)left_start, (right_start-left_start)/width, (end-in)/width+1, (end-in)/width+1 );
			swaps += merge_range( left_start, right_start, end, buf + (offset_left*width), width, compare );
			print_array( (widget*)buf, 0, nel, nel );
		
			// as soon as merge_length==1 we could special case and just perform a swap if needed
		}

		contains_same_elements( (widget*)copy, (widget*)buf, nel );

		temp = in;
		in = buf;
		buf = temp;
		
		merge_length /= 2;
	}

	return swaps;
}

size_t merge_up( char* in, char* buf, size_t nel, size_t width, comparator compare ) {
	
	say("Merge Up\n");
	int swaps = 0;
	
	int merge_length = 2;
	char* temp = NULL;
	int offset_left = 0;
	
	while( merge_length <= nel ) {

		char* copy = malloc( nel*width );
		if( copy == NULL ) {
			perror("malloc()");
			exit( EXIT_FAILURE );
		}
		memcpy( copy, in, nel*width );
	
		say("Merging length %d from source\n", merge_length);
		print_array( (widget*)in, 0, nel, nel );
		memset( buf, 0, nel*width ); // for debug
		for(offset_left=0; offset_left<nel-merge_length+1; offset_left+=2*merge_length) {
			char* left_start = in+(offset_left*width);
			char* right_start = in + ( MIN(offset_left+merge_length, nel-1)*width );
			char* end = in + (MIN(offset_left+merge_length+merge_length-1, nel-1)*width);
			// dislike this special case, but when we have a leftover piece, just take it with the last merge
			say("rest: %d \n", (end-right_start)/width ); 
			if( nel - (end-in)/width -1 < merge_length ) {
				end = in + (nel-1)*width;
				say("Taking remainder, end points to %d\n", *(int*)end);
			} 
			
			say("Merging interval [%d-%d] with [%d-%d]\n", (left_start-in)/width, (right_start-in)/width-1, (right_start-in)/width, (end-in)/width );
			print_array( (widget*)left_start, 0, (right_start-left_start)/width, merge_length );
			print_array( (widget*)left_start, (right_start-left_start)/width, (end-left_start)/width+1, (end-right_start)/width+1 );
			swaps += merge_range( left_start, right_start, end, buf + (offset_left*width), width, compare );
			print_array( (widget*)buf, 0, nel, nel );
			say("Swaps: %d\n", swaps);
		}

		contains_same_elements( (widget*)copy, (widget*)buf, nel );

		temp = in;
		in = buf;
		buf = temp;
		
		merge_length *= 2;
	}
	
	return swaps;
}

size_t merge_range(char* left, char* right, char* end, char* to, size_t width, comparator compare) {
	int swaps = 0;

	if( left == end ) { // for small arrays like length=3 we merge_down (1+1) and the the remaining 1 with itself
		say("Merging single element, just copying\n");
		memcpy( to, left, width ); //copy the 1 element
		return 0;
	}

	char* start = left;
	char* start_right = right;
	// clear so debug output is nicer
	memset( to, 0, end-left );

	say("Merging %d + %d elements to %p\n", (right-left)/width, (end-right)/width, to);
	print_array( (widget*)left, 0, (right-left)/width, 20 );
	print_array( (widget*)right, 0, (end-right)/width+1, 20 );
	while( right <= end ) {
//		say("comparing left %d with right %d -> %d\n", *(int*)left, *(int*)right, compare( left, right ));
		if( compare( left, right ) <= 0 ) {
			memcpy( to, left, width );
			left += width;

			// special case: left might run out, then we bump right by 1 element
			if( left >= start_right ) {
				left = right;
				right += width; // this could make right point past end, but the while will terminate and then we copy only from left
//				say("Ran out of left array, set left to %d and bumped right to %d\n", *(int*)left, *(int*)right);
			}

		} else {
			memcpy( to, right, width );
			right += width;
			swaps++;
		}
		to += width;
	
//		print_array( (widget*)to_start, 0, merge_length*2, merge_length );
//		say("swaps done %d\n", swaps);
	}
	// now we ran out of right, copy over any left if remaining
	if( left < start_right ) { // stuff from left
		memcpy( to, left, start_right - left );
		swaps += (start_right - left)/width;
	} else { // we bumped right, so there is only one remaining
		memcpy( to, left, width );
	}
//	print_array( (widget*)to_start, 0, merge_length*2, merge_length );
	
	return swaps;
	
}

void sort_function( void* base, size_t nel, size_t width, comparator compare ) {
	
	lumbergh_merge_sort( base, nel, width, compare );
	
}

size_t working_set_size( size_t element_size, size_t nel ) {
	return 2 * element_size * nel;
}

