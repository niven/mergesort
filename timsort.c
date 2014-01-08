#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

#include "insertionsort.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

int calc_minrun(size_t nel) {

	int r = 0;  /* becomes 1 if the least significant bits contain at least one off bit */

	while (nel >= 64) {
		r |= nel & 1;
	    nel >>= 1;
	}

	return nel + r;
}

ssize_t find_run( char* base, size_t nel, size_t width, comparator compare ) {
	
	if( nel < 2 ) {
		return nel;
	}
	
	size_t run_length = 2;
	int asc = 1; 
	if( compare( base, base+width ) > 0 ) { // base[0] > base[1]
		asc = 0;
	}
	base += width;
	
	// too tired to figure out the elegant way to compa() >= asc or something.
	int order = compare( base, base+width );
	while( run_length < nel && ( (asc && order <= 0) || (!asc && order > 0 ) ) ) {
		run_length++;

		base += width;
		order = compare( base, base+width );
	}
	
	return asc ? run_length : -run_length;
}

// reverse, though I'm sure there is a faster way.
static inline void reverse_run( char* current, const size_t run_length, size_t width, void* temp_space ) {

	char* left = current;
	char* right = current + (run_length-1)*width;

	say("Reversing from %d to %d\n", *(int*)left, *(int*)right);

	while( left < right ) {
		memcpy( temp_space, left, width );
		memcpy( left, right, width );
		memcpy( right, temp_space, width );
		left += width;
		right -= width;
	}

	say("Reversed descending run\n");
	print_array( (widget*)current, 0, run_length, run_length );
	
}

/*
From http://bugs.python.org/file4451/timsort.txt

What turned out to be a good compromise maintains two invariants on the
stack entries, where A, B and C are the lengths of the three righmost not-yet
merged slices:

1.  A > B+C
2.  B > C

*/
void merge_collapse( run_node** stack ) {
	
	run *A, *B, *C;
	A = peek_run( *stack, 0 );
	B = peek_run( *stack, 1 );
	C = peek_run( *stack, 2 );
	
	printf("A: %zu, B: %zu, C: %zu\n", A==NULL?0:A->nel, B==NULL?0:B->nel, C==NULL?0:C->nel );
	
}

/*
http://bugs.python.org/file4451/timsort.txt
http://infopulseukraine.com/eng/blog/Software-Development/Algorithms/Timsort-Sorting-Algorithm/
*/
void timsort(void* base, size_t nel, size_t width, comparator compare) {
	
	void* value = malloc( width );
	if( value == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	
	int minrun = calc_minrun( nel );
	minrun /= 4; // for debugging, so we don't have to test arrays of many elements to have minrun << nel
	say("Minrun for %zu elements: %d\n", nel, minrun);
	
	run_node* sorted_runs = NULL; // stack for keeping run indices
	
	char* current = (char*)base;
	size_t reached = 0;
	
	while( reached < nel ) {
		
		say("Current index %zu\n", reached);
		
		current = (char*)base + reached*width;
		ssize_t run_length = find_run( current, nel-reached, width, compare );
		say("Run length %zd\n", run_length);
		print_array( (widget*)current, 0, abs(run_length), minrun );
		if( run_length < 0 ) { // was descending
			run_length = abs( run_length );
			reverse_run( current, run_length, width, value );
		}
		
		// extend to minrun if needed
		run_length = run_length < minrun ? MIN(minrun, nel-reached) : run_length;
		say("Run length for insertionsort %zu\n", run_length);
		
		// insertion sort
		print_array( (widget*)current, 0, run_length, minrun );
		insertionsort( current, run_length, width, compare );
		print_array( (widget*)current, 0, run_length, minrun );
		
		push_run( &sorted_runs, new_run( current, run_length ) );
		print_stack( sorted_runs );
		merge_collapse( &sorted_runs );
		
		reached += run_length;
	}

	free( value );
}


void sort_function( void* base, size_t nel, size_t width, comparator compare ) {
	
	timsort( base, nel, width, compare );
	
}

size_t working_set_size( size_t element_size, size_t nel ) {
	return 2 * element_size * nel;
}

