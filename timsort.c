#include <fcntl.h> /* open */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* dup */

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
int merge_collapse( run_node** stack ) {
	
	run *A, *B, *C;
	A = peek_run( *stack, 2 );
	B = peek_run( *stack, 1 );
	C = peek_run( *stack, 0 );
	
	printf("A: %zu, B: %zu, C: %zu\n", A==NULL?0:A->nel, B==NULL?0:B->nel, C==NULL?0:C->nel );
	
	if( B == NULL ) { // only single item
		return 0;
	}
	
	// fix invariant violations (A may be empty, but B < C)
	if( B->nel <= C->nel ) { // #2 B > C
		say("Merging B+C\n");
		B->nel += C->nel;
		run* top = pop_run( stack );
		free( top );
		return 1;
	}
	
	// TODO: I think we're missing an invariant here
	
	if( A==NULL || B==NULL || C==NULL ) {
		return 0;
	}
	
	if( A->nel <= B->nel + C->nel ) {
		if( C->nel <= A->nel ) {
			say("Merging B+C\n");
			// just update the B and pop C
			B->nel += C->nel;
			run* top = pop_run( stack );
			free( top );
		} else {
			say("Merging A+B\n");	
			// just update A and pop B,C then push C
			say("NOTHING ACTUALLY HAPPENING HERE YET\n");
		}
		return 1;
	}
		
	return 0;
}

/*
merge_lo and merge_hi both merge 2 arrays, but to minimize the memory use
we only allocate memory for the smaller run, and then use the original space
for the merge. This unfortunately means merging works differently depending
on where the smaller array is.

I'm actually not convinced we need to have 2 different "symmetrical" functions for this.

*/

// assume a is smaller
void merge_lo( run* a, run* b, size_t width, comparator compare ) {
	
	if( a->nel > b->nel ) {
		printf("a has more elements than b\n");
		exit( EXIT_FAILURE );
	}
	
	size_t first_b_in_a = find_index( a->address, *(b->address), width, compare );
	size_t last_a_in_b = find_index( b->address, *(a->address + a->nelwidth), width, compare );
	
	// allocate space for the smaller (a) array
	char* temp = malloc( a->nel * width );
	if( temp == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	memcpy( temp, a->address, a->nel * width );
	
	
	
	free( temp );
}

void merge_hi( run* a, run* b, size_t width, comparator compare ) {
	
}

size_t find_index( void* in, size_t nel, void* value, size_t width, comparator compare ) {
	
	say("Find index starting from value %d, %d elements\n", *(int*)in, nel);
	
	if( nel <= 1 ) {
		say("Only 1 element left (%d)\n", *(int*)in);
		return compare( in, value ) < 0 ? 0 : 1; // equal values come after to ensure a stable sort 
	}
	
	static int recurse_depth = 0;
	
	char* list = (char*)in;
	// check where value belongs in "steps" of 2^i-1 (the -1 is to ensure we start at 0, the sequence is 0, 1, 3, 7, ...)
	int pow = 0;
	size_t index = (1 << pow) - 1;
	say("About to compare values[%d] = %d with %d\n", index, *(int*) (list + index*width), *(int*)value );
	int sentinel = 0;
	while( sentinel++ < 5 && index < nel && compare( list + index*width, value ) <= 0 ) {
		index = (1 << ++pow) -1;
		say("Going to compare values[%d] = %d with %d\n", index, *(int*) (list + index*width), *(int*)value ); 
	}

	// at this point we can have 3 possible situations:
	// 1. Every element in list is > value
	// 2. value falls somewhere in list
	// 3. value > last index checked, but the next index > nel

	// if we overshot we recurse on the sublist defined by the (previous, index) exclusive range
	// if index == 0 we are smaller than the first element so just return
	// this is #1
	if( index == 0 ) {
		say("Smaller than the first element, returning 0\n");
		return 0;
	}

	pow--;
	int min_placement_index = 0;
	if( index >= nel ) { // case #3
		min_placement_index = 1 << pow; // the previous value of index+1 (since we already compared that one)
	} else { // case #2. list[index] > value
		min_placement_index = 1 << pow; // ehr, same result? Did not expect that :)
	}
	say("Done at %d, recursing from %d\n", index, min_placement_index);
	
	if( recurse_depth++ > 5 ) {
		say("Recurse max hit: %d\n", recurse_depth);
		return 0;
	}
	
	// now find another index starting at previous, add that to previous and that's our index
	size_t elements_left = nel - (min_placement_index) - (nel - MIN(index, nel)) - 1;// all - smaller stuff - larger stuff - for starting at previous+1
	return min_placement_index + find_index( list + min_placement_index*width, elements_left, value, width, compare ); 
	
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
	minrun = 1; // for debugging, so we don't have to test arrays of many elements to have minrun << nel
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
		if( run_length < minrun ) {
			run_length = run_length < minrun ? MIN(minrun, nel-reached) : run_length;
			say("Run length for insertionsort %zu\n", run_length);
			// insertion sort
			print_array( (widget*)current, 0, run_length, minrun );
			SUPPRESS_STDOUT
			insertionsort( current, run_length, width, compare );
			RETURN_STDOUT
			print_array( (widget*)current, 0, run_length, minrun );
		}
		// else: run_length already > minrun, and sorted as well
		
		push_run( &sorted_runs, new_run( current, run_length ) );
		print_stack( sorted_runs );
		while( merge_collapse( &sorted_runs ) ) {
			say("Collapsed, repeating until invariants hold\n");
		}
		
		reached += run_length;
	}

	// now we're almost done, except that we might have some unmerged things on the stack
	say("Wrapup merges\n");

	while( peek_run( sorted_runs, 1 ) != NULL ) {
		print_stack( sorted_runs );
		// want to do the same strategy I guess?
		// for now just merge everthing
		run* B = pop_run( &sorted_runs );
		run* C = pop_run( &sorted_runs );
		size_t total = B->nel + C->nel;
		if( B->nel <= C->nel ) {
			merge_lo( B, C, width, compare ); // don't like this name but it's what the jargon is
		} else {
			merge_hi( B, C, width, compare ); // idem
		}
		push_run( &sorted_runs, new_run( B->address, total) );
		free( B );
		free( C );
	}



	free( value );
}


void sort_function( void* base, size_t nel, size_t width, comparator compare ) {
	
	timsort( base, nel, width, compare );
	
}

size_t working_set_size( size_t element_size, size_t nel ) {
	return 2 * element_size * nel;
}

