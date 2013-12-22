#include "utils.h"

// Sort an array a[start...end].
// end is inclusive (seems like a bad idea)
void shellsort( int* numbers, int start, int end ) {

	int i, j, g;

	printf("Shellsort [%d - %d] [", start, end );
	for(int i=start; i<=end; i++) {
		printf("%3d ", numbers[i]);
		if( i< end-1 &&  (i+1)%(end-start+1)==0 ) {
			printf("]\n[ ");
		}
	}
	printf("]\n");

	// Do an insertion sort for each gap size.
	for( g=0; g<8; g++ ) {
		int gap = gaps[g];
		printf("Shellsort Gap %d (%d,%d,%d)\n", gap, start+gap, end, gap );
		// i iterates over a virtual array defined by gapsize
		// so if gap=3 then i iterates n[3], n[6], n[9] (starting at element 2)
		// But we increment i by 1, so what gives?
		// Well, the sequence 3,6,9 + 4,7,10 + 5,8,11 is the same 
		// as 3,4,5,6,7,8,9,10,11 just in a different order and that saves a for loop
		// j iterates backwards from i in gapsize steps so the end result is the same 
		for (i = start+gap; i <= end; i++ ) { 
		    int number_to_place = numbers[i];
			printf("Shellsort n[%d] = %d\n", i, number_to_place );
			// now iterate over every "gapth" element back to the left, moving items until
			// we find one that is lower/equal than number_to_place
			// For the first iteration, either the second number is larger and we're done, or move it to the position of the first element
			// then we try element 3, move that left to its spot (since 1 and 2 are now sorted)
		    for (j = i-gap; j >= start && numbers[j] > number_to_place; j -= gap) {
		        numbers[j+gap] = numbers[j];
		    }
		    numbers[j+gap] = number_to_place;
			printf("Shellsort [%d - %d] [", start, end );
			for(int i=start; i<=end; i++) {
				printf("%3d ", numbers[i]);
				if( i< end-1 &&  (i+1)%(end-start+1)==0 ) {
					printf("]\n[ ");
				}
			}
			printf("]\n");
		}
		
	}
	
}
