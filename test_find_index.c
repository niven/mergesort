#include <stdio.h>
#include <string.h>
#include <time.h>

#include "utils.h"

#include "timsort.c"

// size_t find_index( const void* in, size_t nel, const void* value, size_t width, comparator compare ) {



int main( int argc, char* argv[] ) {

	
	size_t num_tests = atoi( argv[1] );
	size_t max_length = atoi( argv[2] );
	
	say("Running %zu tests of max length %zu\n", num_tests, max_length); 
	
	srand( time( NULL ) );
	
	for(size_t i=0; i<num_tests; i++ ) {
		size_t count = rand() % max_length +1;
		int* array = malloc( count * sizeof(int) );
		for(size_t j=0; j<count; j++ ) {
			array[j] = rand() % 255;
		}
		
		int target = rand() % 255;
		qsort( array, count, sizeof(int), compare_int );
		
		
		say("\n******** Test %d with %zu elements\nTarget %d ********\n[", i, count, target);
		for(int j=0; j<count; j++) {
			say( "%3d ", array[j] );
		}
		say( "]\n[" );	
		for(int j=0; j<count; j++) {
			say( "%3d ", j );
		}
		say( "]\n" );	

		// find the index using linear search
		size_t correct_index;
		for(correct_index=0; correct_index<count && target >= array[correct_index]; correct_index++) {
			// empty
		}
		// could be after the whole thing
		if( correct_index == count-1 && target >= array[count-1] ) {
			correct_index++;
		}
		if( correct_index == count ) {
			say("Correct index is %zu, after last element\n\n", correct_index);
		} else if( correct_index < count ) {
			say("Correct index is %zu, displacing %d\n\n", correct_index, array[correct_index]);
		} else {
			say("CANNOT HAPPEN, correct_index = %d, count = %d\n", correct_index, count);
			exit(1);
		}

		size_t index = find_index( array, count, &target, sizeof(int), compare_int );
		say("\n");
		size_t index_from_end = find_index_reverse( array + count -1, count, &target, sizeof(int), compare_int );
		
		say("\nInsert index = %zu, displacing %d\n", index, array[index]);
		say("Insert index_backwards = %zu, displacing %d\n", index_from_end, array[count-index_from_end]);

		if( index > 0 ) { // check against left 
			say("Check %d <= %d\n", array[index-1], target );
			assert( array[index-1] <= target );
		}

		if( index < count-1 ) { // check against right 
			say("Check %d < %d\n", target, array[index] );
			assert( target < array[index] );
		}

		assert( correct_index == index );
		assert( correct_index == count - index_from_end );
	
		free( array );
	}

	exit( 0 );
}
