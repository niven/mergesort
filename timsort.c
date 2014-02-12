#include <assert.h>
#include <fcntl.h> /* open */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* dup */

#include "utils.h"

#include "insertionsort.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

#define MIN_GALLOP 4

void merge_lo( run* a, run* b, size_t width, comparator compare );
void merge_hi( run* a, run* b, size_t width, comparator compare );


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

Returns 1 if something was done, 0 otherwise.
This allows one to do while( merge_collapse() ) which I think is nice because we
can't tell how many times we have to do a merge due to having to maintain the invariants.
*/
int merge_collapse( run_node** stack, size_t width, comparator compare ) {
	
	run *A, *B, *C;
	A = peek_run( *stack, 2 );
	B = peek_run( *stack, 1 );
	C = peek_run( *stack, 0 );
	
	say("Merge collapse: A: %zu, B: %zu, C: %zu\n", A==NULL?0:A->nel, B==NULL?0:B->nel, C==NULL?0:C->nel );
	
	if( A == NULL && B == NULL ) { // only single item
		return 0;
	}
	
	// fix invariant violations (A may be empty, but B < C)
	if( B->nel <= C->nel ) { // #2 B > C
		say("Merging B+C\n");
		merge_lo( B, C, width, compare );
		run* top = pop_run( stack );
		free( top );
		say("Post merge B:\n");
		print_array( (widget*)B->address, 0, B->nel, B->nel );
		is_sorted( (widget*)B->address, 0, B->nel );
		return 1;
	}
	
	// TODO: I think we're missing an invariant here
	// and it's probably A=NULL?
	// yes, it is indeed A=NULL :)
	
	if( A == NULL ) {
		return 0;
	}
		
	if( A->nel <= B->nel + C->nel ) {
		if( C->nel <= A->nel ) {
			say("Merging B+C\n");
			// just update the B and pop C
			merge_hi( B, C, width, compare ); // B must be larger than C otherwise we'd have merge_lo()'d above
			run* top = pop_run( stack );
			free( top );
			is_sorted( (widget*)B->address, 0, B->nel );
		} else {
			say("Merging A+B\n");	
			// just update A and pop B,C then push C
			say("NOTHING ACTUALLY HAPPENING HERE YET. MERGE LO/HI, DUNNO\n");
			exit( EXIT_FAILURE );
		}
		return 1;
	}
		
	return 0;
}



/*
	Finds the greatest index N in list so that for all elements list[ < N ] <= value

	Algorithm: don't binary search but compare every element with index (2^n)-1
	Whenever we overshoot we recurse with the previous value etc. This is mathmatically
	the same number of operations (I think but haven't done any math yet). But biases
	towards the beginning of the array. This is good if you are merging random runs
	in timsort where this comes from. the idea is: first find the placement of A[0] in B
	and copy everything in B before that point to save on comparing. This is great when
	you have disjoint ranges of numbers, but awful when they are random. Hence this method.

	Bug: if list approaches size_t max val then the index variable may overflow it
	In practice I hope to never have lists that large.
*/
size_t find_index( const void* in, size_t nel, const void* value, size_t width, comparator compare ) {
	
	say("Find index starting from value %d, %d elements\n", *(int*)in, nel);
	
	if( nel <= 1 ) {
		say("Only 1 element left (%d)\n", *(int*)in);
		return compare( in, value ) <= 0 ? 1 : 0; // equal values come after
	}
	
	char* list = (char*)in;
	// check where value belongs in "steps" of 2^i-1 (the -1 is to ensure we start at 0, the sequence is 0, 1, 3, 7, ...)
	int pow = 0;
	size_t index = (1 << pow) - 1;
	say("About to compare values[%d] = %d with %d\n", index, *(int*) (list + index*width), *(int*)value );
	while( index < nel && compare( list + index*width, value ) <= 0 ) {
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

	// BUG is here somewhere, if the element we're looking for should be placed after the last element I think
	
	pow--;
	size_t min_placement_index = MIN(1 << pow, nel-1); // the previous value of index+1 (since we already compared that one) unless index happened to be the last index of list;
	
	// if we have 10 elements and min=4 we continue searching at list[4], meaning list[4-9] = 6 elements remaining
	size_t elements_remaining = nel - min_placement_index; 
	
	say("Elements remaining %zu, min_placement_index %zu\n", elements_remaining, min_placement_index);
	
	if( elements_remaining == 0 ) {
		return min_placement_index;
	}
	
	if( index >= nel ) { // case #3
		// whatever remains is "the rest"
		say("Recursing until end of list (%d elements)\n", elements_remaining);
	} else { // case #2. list[index] > value
		// we can exclude everything gte index. If we have 10 elements list[4] = 8 and value = 5 everthing in list[4-9] can be left out = 6 = 10-index
		elements_remaining -= nel - index;
	}
	say("Recursing from index %d, elements remaining after removing tail: %d\n", min_placement_index, elements_remaining);
		
	// now find another index starting at previous, add that to previous and that's our index
	return min_placement_index + find_index( list + min_placement_index*width, elements_remaining, value, width, compare ); 

}

/* same as find_index(), but starting from the end and searching towards the beginning. */
size_t find_index_reverse( const void* in, size_t nel, const void* value, size_t width, comparator compare ) {
	
	say("Find reverse index starting from value %d, %d elements\n", *(int*)in, nel);
	
	if( nel <= 1 ) {
		say("Only 1 element left (%d)\n", *(int*)in);
		return compare( in, value ) <= 0 ? 0 : 1; // equal values come after
	}
	
	char* list = (char*)in;
	// check where value belongs in "steps" of 2^i-1 (the -1 is to ensure we start at 0, the sequence is 0, 1, 3, 7, ...)
	int pow = 0;
	size_t index = (1 << pow) - 1;
	say("About to compare values[%d] = %d with %d\n", index, *(int*) (list - index*width), *(int*)value );
	while( index < nel && compare( list - index*width, value ) > 0 ) {
		index = (1 << ++pow) -1;
		say("Going to compare values[-%d] = %d with %d\n", index, *(int*) (list - index*width), *(int*)value ); 
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

	// BUG is here somewhere, if the element we're looking for should be placed after the last element I think
	
	pow--;
	size_t min_placement_index = MIN(1 << pow, nel-1); // the previous value of index+1 (since we already compared that one) unless index happened to be the last index of list;
	
	// if we have 10 elements and min=4 we continue searching at list[4], meaning list[4-9] = 6 elements remaining
	size_t elements_remaining = nel - min_placement_index; 
	
	say("Elements remaining %zu, min_placement_index %zu\n", elements_remaining, min_placement_index);
	
	if( elements_remaining == 0 ) {
		return min_placement_index;
	}
	
	if( index >= nel ) { // case #3
		// whatever remains is "the rest"
		say("Recursing until end of list (%d elements)\n", elements_remaining);
	} else { // case #2. list[index] > value
		// we can exclude everything gte index. If we have 10 elements list[4] = 8 and value = 5 everthing in list[4-9] can be left out = 6 = 10-index
		elements_remaining -= nel - index;
	}
	say("Recursing from index %d, elements remaining after removing tail: %d\n", min_placement_index, elements_remaining);
		
	// now find another index starting at previous, add that to previous and that's our index
	return min_placement_index + find_index_reverse( list - min_placement_index*width, elements_remaining, value, width, compare ); 

}

void gallop_backwards( char* to, char* left, char* left_start, char* right, char* right_start, size_t width, comparator compare ) {
	
}

/*
merge_lo and merge_hi both merge 2 arrays, but to minimize the memory use
we only allocate memory for the smaller run, and then use the original space
for the merge. This unfortunately means merging works differently depending
on where the smaller array is.

I'm actually not convinced we need to have 2 different "symmetrical" functions for this.

so the cases are actually:
[stuff, sub array A, sub array B, maybe more stuff]

and we've decided to merge A+B.
merge_lo: A <= B (create temp space size of A)
merge_hi: A > B (temp space size of B, meaning we have to merge from the end because that is where the free space is)

Important to note A is always "left" in the enclosing array and B is alway "right", so we can't just call
	merge_lo(B,A) instead of merge_lo(A,B)

// assume a is smaller
*/
void merge_lo( run* a, run* b, size_t width, comparator compare ) {

	size_t total_elements = a->nel + b->nel;
	say("Merge lo (%zu elements)\n", total_elements);
	assert( a->nel <= b->nel );

	say("Merging L+R:\n");
	print_array( (widget*)a->address, 0, a->nel, a->nel );
	print_array( (widget*)b->address, 0, b->nel, b->nel);

	// Can we share this between hi/lo? Think so

	const void* merged_array = a->address; // after doung this first_b_in_a business, we might start at an offset from A

	/*
	Imagine merging an array [A,B] where A = [1,3,5,7] and B = [4,6,8,10]
	Finding the first element of B in A means that you can skip 1,3 in the merge.
	Same goes for the last element of A in B, you can skip 8,10

	[1,3,| 4,5,6,7 | ,8,10]

	The actual merge only needs to happen for the elements between the vertical bars

	*/
	say("Finding index of B[0]=%d in A:\n", *(int*)b->address);
	SUPPRESS_STDOUT
	size_t first_b_in_a = find_index( a->address, a->nel, b->address, width, compare );
	RETURN_STDOUT
	say("B[0] (%d) should be placed at index %d in A (A[%d] = %d)\n", *(int*)b->address, first_b_in_a, first_b_in_a, *(int*) ( (char*)a->address + first_b_in_a*width) );
	// this basically means A[0]-A[first_b_in_a] are already sorted, so we adjust a
	a->nel -= first_b_in_a;
	a->address = (char*)a->address + first_b_in_a*width;
	say("A now starts with %d with %d elements:\n", *(int*)a->address, a->nel);
	print_array( (widget*)a->address, 0, a->nel, a->nel);

	say("Finding index of A[%d]=%d in B:\n", a->nel-1, *(int*) ( (char*) a->address + (a->nel-1)*width) );
	// maybe search backwards here?
	SUPPRESS_STDOUT
	size_t last_a_in_b = find_index( b->address, b->nel, (char*)a->address + (a->nel-1)*width, width, compare );
	RETURN_STDOUT
	say("A[%d] (%d) should be placed at index %d in B (B[%d] = %d (could be out of bounds))\n", a->nel-1, *(int*) ( (char*) a->address + (a->nel-1)*width), last_a_in_b, last_a_in_b, *(int*) ( (char*)b->address + last_a_in_b*width) );
	// this basically means B[last_a_in_b]-B[-1] are already sorted so we adjust B
	b->nel -= b->nel - last_a_in_b;
	say("B still starts with %d but now with %d elements:\n", *(int*)b->address, b->nel);
	print_array( (widget*)b->address, 0, b->nel, b->nel);

	// allocate space for the smaller (a) array
	// Hmm, B might now be smaller
	char* to = (char*)a->address;
	char* left = malloc( a->nel * width );
	char* temp = left;
	if( left == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	memcpy( left, a->address, a->nel * width );
	
	char* left_end = left + a->nel*width;
	char* right = (char*)b->address;
	char* right_end = right + b->nel*width;

	memset( to, 0, (a->nel) * width ); // erase, left for debug

	say("Merging bounded arrays:\n");
	print_array( (widget*)left, 0, a->nel, a->nel );
	print_array( (widget*)right, 0, b->nel, b->nel);
	
	size_t same_run_counter = 0;
	int current_run = 1; // 0 is a 1 is b
	while( left < left_end && right < right_end ) {

		say("Comparing %d with %d\n", *(int*)left, *(int*)right );
	
		// too many damn branches (shakes fist)
		// also copying elements 1 by one instead of while ( a<= b ){ count++}; memcpy( to, a, count ); a+=count
		if( compare( left, right ) <= 0 ) {
			memcpy( to, left, width );
			left += width;
			if( current_run == 0 ) {
				same_run_counter++;
			} else {
				same_run_counter = 0;
			}
			current_run = 0;
		} else {
			memcpy( to, right, width );
			right += width;
			if( current_run == 1 ) {
				same_run_counter++;
			} else {
				same_run_counter = 0;
			}
			current_run = 1;
		}
		to += width;

		if( same_run_counter >= MIN_GALLOP ) {
			
			say("Gallop Forwards with left/right:\n");
			print_array( (widget*)left, 0, (left_end-left)/width, (left_end-left)/width );
			print_array( (widget*)right, 0, (right_end-right)/width, (right_end-right)/width );

			say("Dest:\n");
			print_array( (widget*)to, 0, (right_end-right + left_end-left)/width, (right_end-right + left_end-left)/width );
			
		}

	}

	// copy remainders
	say("Merged so far:\n");
	print_array( (widget*)a->address, 0, a->nel+b->nel, a->nel+b->nel );
	// TODO: I don't think both can have remainders
	if( left < left_end ) {
		say("Copying %d remaining elements from left:\n", (left_end-left)/width );
		print_array( (widget*)left, 0, (left_end-left)/width, (left_end-left)/width );
		memcpy( to, left, left_end-left );
	}
	if( right < right_end ) {
		say("Copying %d remaining elements from right:\n", (right_end-right)/width );
		print_array( (widget*)right, 0, (right_end-right)/width, (right_end-right)/width );
		memcpy( to, right, right_end-right );
	}

	say("Merge lo result:\n");
	a->address = merged_array;
	print_array( (widget*)a->address, 0, total_elements, total_elements );

	is_sorted( (widget*)a->address, 0, total_elements );

	free( temp );
	
	a->nel = total_elements; // update the run info
}

void merge_hi( run* a, run* b, size_t width, comparator compare ) {

	size_t total_elements = a->nel + b->nel;
	say("Merge hi (%zu elements)\n", total_elements);
	assert( a->nel > b->nel );

	say("Merging L+R:\n");
	print_array( (widget*)a->address, 0, a->nel, a->nel );
	is_sorted( (widget*)a->address, 0, a->nel );
	print_array( (widget*)b->address, 0, b->nel, b->nel);
	is_sorted( (widget*)b->address, 0, b->nel );

	const void* merged_array = a->address; // after doing this first_b_in_a business, we might start at an offset from A

	// yeah, this is essentially copied from merge_lo, but when compiled non-verbose reduces to 5 lines
	say("Finding index of B[0]=%d in A:\n", *(int*)b->address);
//	print_array( (widget*)a->address, 0, a->nel, a->nel);
	SUPPRESS_STDOUT
	size_t first_b_in_a = find_index( a->address, a->nel, b->address, width, compare );
	RETURN_STDOUT
	say("B[0] (%d) should be placed at index %d in A (A[%d] = %d)\n", *(int*)b->address, first_b_in_a, first_b_in_a, *(int*) ( (char*)a->address + first_b_in_a*width) );
	// this basically means A[0]-A[first_b_in_a] are already sorted, so we adjust a
	a->nel -= first_b_in_a;
	a->address = (char*)a->address + first_b_in_a*width;
	say("A now starts at %d with %d elements:\n", *(int*)a->address, a->nel);
	print_array( (widget*)a->address, 0, a->nel, a->nel);

	say("Finding index of A[%d]=%d in B:\n", a->nel-1, *(int*) ( (char*) a->address + (a->nel-1)*width) );
//	print_array( (widget*)b->address, 0, b->nel, b->nel);
	// maybe search backwards here?
	SUPPRESS_STDOUT
	size_t last_a_in_b = find_index( b->address, b->nel, (char*)a->address + (a->nel-1)*width, width, compare );
	RETURN_STDOUT
	say("A[%d] (%d) should be placed at index %d in B (B[%d] = %d (could be out of bounds))\n", a->nel-1, *(int*) ( (char*) a->address + (a->nel-1)*width), last_a_in_b, last_a_in_b, *(int*) ( (char*)b->address + last_a_in_b*width) );
	// this basically means B[last_a_in_b]-B[-1] are already sorted so we adjust B
	
	b->nel -= b->nel - last_a_in_b;
	say("B still starts at %d but now with %d elements:\n", *(int*)b->address, b->nel);
	print_array( (widget*)b->address, 0, b->nel, b->nel);

	// allocate space for the smaller (b) array
	char* to = (char*)a->address + (a->nel+b->nel) * width; // we merge from the end, a+b elements
	char* right_start = malloc( b->nel * width );
	char* temp = right_start;
	if( right_start == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	memcpy( right_start, b->address, b->nel * width );

	char* left_start = (char*)a->address;
	char* left = (char*)a->address + (a->nel-1)*width;
	char* right = right_start + (b->nel-1)*width;

	memset( (char*)to - (b->nel) * width, 0, b->nel*width ); // erase right for debug (to already points at the end of left_end)

	say("Merging bounded arrays:\n");
	print_array( (widget*)left_start, 0, a->nel, a->nel );
	print_array( (widget*)right_start, 0, b->nel, b->nel );
	say("To target range:\n");
	print_array( (widget*)(to - (a->nel+b->nel)*width), 0, a->nel+b->nel, a->nel+b->nel );

	size_t same_run_counter = 0;
	int current_run = 1; // 0 is a 1 is b
	while( left >= left_start && right >= right_start ) {

		//say("Comparing %d with %d\n", *(int*)left, *(int*)right );
	
		// too many damn branches (shakes fist)
		if( compare( left, right ) > 0 ) {
			memcpy( to, left, width );
			left -= width;
			if( current_run == 0 ) {
				same_run_counter++;
			} else {
				same_run_counter = 0;
			}
			current_run = 0;
		} else {
			memcpy( to, right, width );
			right -= width;
			if( current_run == 1 ) {
				same_run_counter++;
			} else {
				same_run_counter = 0;
			}
			current_run = 1;
		}
		to -= width;
		print_array( (widget*)a->address, 0, a->nel + b->nel + 1, a->nel + b->nel +1 );

		if( same_run_counter >= MIN_GALLOP ) {
			
			gallop_backwards( to, left, left_start, right, right_start, width, compare );
			
		}

	}

	say("Merged runs, may have remainders:\n");
	print_array( (widget*)a->address, 0, a->nel+b->nel +1, a->nel+b->nel +1 );

	// copy remainders, >= because we go RTL
	// TODO: I don't think both could have remainders
	if( left >= left_start ) {
		say("Copying %d elements from left:\n", (left-left_start)/width);
		print_array( (widget*)left_start, 0, (left-left_start)/width, (left-left_start)/width );
		to -= left-left_start;
		memcpy( to, left_start, (left-left_start)+width ); // +1 width since RTL
	}
	if( right >= right_start ) {
		say("Copying %d elements from right:\n", (right-right_start)/width+1);
		print_array( (widget*)right_start, 0, (right-right_start)/width+1, (right-right_start)/width+1 );
		to -= (right - right_start) ;
		memcpy( to, right_start, (right-right_start)+width );
	}
	say("Remainders copied:\n");
	print_array( (widget*)a->address, 0, a->nel+b->nel +1, a->nel+b->nel +1 );
	

	say("Merge hi result:\n");
	a->address = merged_array;

	print_array( (widget*)a->address, 0, total_elements +1, total_elements +1 );

	is_sorted( (widget*)a->address, 0, total_elements );

	free( temp );

	a->nel = total_elements; // update the run info
}

/*
http://bugs.python.org/file4451/timsort.txt
http://infopulseukraine.com/eng/blog/Software-Development/Algorithms/Timsort-Sorting-Algorithm/

Note: keeping track of runs with number of elements might be inelegant. I'm starting to think last_index would
be better

*/
void timsort(void* base, size_t nel, size_t width, comparator compare) {
	
	void* value = malloc( width );
	if( value == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	
	int minrun = calc_minrun( nel );
//	minrun = 1; // for debugging, so we don't have to test arrays of many elements to have minrun << nel
	say("Minrun for %zu elements: %d\n", nel, minrun);
	
	run_node* sorted_runs = NULL; // stack for keeping run indices
	
	char* current = (char*)base;
	size_t reached = 0;
	
	while( reached < nel ) {
		
		say("Current index %zu, finding next run\n", reached);
		
		current = (char*)base + reached*width;
		ssize_t run_length = find_run( current, nel-reached, width, compare );
		say("Found a run with length %zd\n", run_length);
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
		while( merge_collapse( &sorted_runs, width, compare ) ) {
			say("Collapsed, repeating until invariants hold\n");
		}
		
		reached += run_length;
	}

	// now we're almost done, except that we might have some unmerged things on the stack
	say("\nWrapup merges\n");

	while( peek_run( sorted_runs, 1 ) != NULL ) { // keep popping things until there is only 1 remaining
		print_stack( sorted_runs );
		// want to do the same strategy I guess?
		// for now just merge everthing
		run* C = pop_run( &sorted_runs );
		run* B = pop_run( &sorted_runs );
		size_t total = B->nel + C->nel;
		say("Merging 2 runs (B+C)\n");
//		print_array( (widget*)B->address, 0, B->nel, B->nel );
//		print_array( (widget*)C->address, 0, C->nel, C->nel );

		if( B->nel <= C->nel ) {
			merge_lo( B, C, width, compare ); // don't like this name but it's what the jargon is
		} else {
			merge_hi( B, C, width, compare ); // idem
		}
		
		push_run( &sorted_runs, new_run( B->address, total) );

		free( B );
		free( C );

		is_sorted( (widget*)peek_run( sorted_runs, 0 )->address, 0, peek_run( sorted_runs, 0 )->nel );
	}

	// now we have only 1 item on the stack
	run* top = pop_run( &sorted_runs );
	say("Finished with sorted at %p and base at %p\n", top->address, base);
	free( top );
	assert( sorted_runs == NULL );

	say("All done, sorted:\n");
	print_array( (widget*)base, 0, nel, nel );

	free( value ); // maybe merge functions should use their own value? OTOH this saves a ton of mallocs
}
