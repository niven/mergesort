#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "utils.h"


const int gaps[8] = {701, 301, 132, 57, 23, 10, 4, 1};


// Sort an array a[start...end].
// end is inclusive (seems like a bad idea)
void shellsort( int* numbers, int start, int end ) {

	int i, j, g;

#ifdef VERBOSE
	printf("Shellsort [%d - %d] [", start, end );
	for(int i=start; i<=end; i++) {
		printf("%3d ", numbers[i]);
		if( i< end-1 &&  (i+1)%((end-start)+1)==0 ) {
			printf("]\n[ ");
		}
	}
	printf("]\n");
#endif
	
	// Do an insertion sort for each gap size.
	for( g=0; g<8; g++ ) {
		int gap = gaps[g];
#ifdef VERBOSE
		printf("Shellsort Gap %d (%d,%d,%d)\n", gap, start+gap, end, gap );
#endif
		// i iterates over a virtual array defined by gapsize
		// so if gap=3 then i iterates n[3], n[6], n[9] (starting at element 2)
		// But we increment i by 1, so what gives?
		// Well, the sequence 3,6,9 + 4,7,10 + 5,8,11 is the same 
		// as 3,4,5,6,7,8,9,10,11 just in a different order and that saves a for loop
		// j iterates backwards from i in gapsize steps so the end result is the same 
		for (i = start+gap; i <= end; i++ ) { 
		    int number_to_place = numbers[i];
#ifdef VERBOSE
			printf("Shellsort n[%d] = %d\n", i, number_to_place );
#endif
			// now iterate over every "gapth" element back to the left, moving items until
			// we find one that is lower/equal than number_to_place
			// For the first iteration, either the second number is larger and we're done, or move it to the position of the first element
			// then we try element 3, move that left to its spot (since 1 and 2 are now sorted)
		    for (j = i-gap; j >= start && numbers[j] > number_to_place; j -= gap) {
		        numbers[j+gap] = numbers[j];
		    }
		    numbers[j+gap] = number_to_place;
#ifdef VERBOSE
			printf("Shellsort [%d - %d] [", start, end );
			for(int i=start; i<=end; i++) {
				printf("%3d ", numbers[i]);
				if( i< end-1 &&  (i+1)%(end-start+1)==0 ) {
					printf("]\n[ ");
				}
			}
			printf("]\n");
#endif
		}
		
	}
	
}

void is_sorted(int* numbers, int from, int to) {

	for( int i=from; i<to-1; i++ ) {
		if( numbers[i] > numbers[i+1] ) {
			printf("Fail at %d/%d (%d/%d)\n", i, i+1, numbers[i], numbers[i+1]);
			exit(0);
		}
	}

}

size_t read_numbers( const char* filename, int** numbers ) {
	
	FILE* in = fopen( filename, "rb" );
	if( in == NULL ) {
		perror("open()");
		exit(0);
	}
	
	// find out how many 4 byte ints
	struct stat st;
	int result = fstat( fileno(in), &st );
	if( result == -1 ) {
		perror("fstat()");
		fclose( in );
		exit(0);
	}
	size_t count = st.st_size / sizeof(int);
	printf("Number of integers of size %ld bytes in file: %zu\n", sizeof(int), count);
	
	int* target = malloc( sizeof(int)*count );
	if( target ==  NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	size_t read = fread( target, sizeof(int), count, in );
	if( read != count ) {
		printf("Read %zu ints, expected %zu\n", read, count);
		free( target );
		exit( EXIT_FAILURE );
	}	
	fclose( in );
	*numbers = target;
	
	return count;
}

int write_numbers( int* numbers, size_t count, const char* filename_out ) {

	FILE* out = fopen( filename_out, "wb" );
	if( out == NULL ) {
		perror("fopen()");
		return -1;
	}
	
	size_t written = fwrite( numbers, sizeof(int), count, out );
	if( written != count ) {
		perror("frwrite()");
		return -1;
	}	
	
	fclose( out );

	return 0;
}

