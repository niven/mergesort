#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "assert.h"
#include "time.h"
#include "unistd.h"
#include "sys/stat.h"

#include "mach/mach_time.h"

typedef void (*sorter)(int* numbers, int start, int end);

void is_sorted(int* numbers, int from, int to) {

	for( int i=from; i<to-1; i++ ) {
		if( numbers[i] > numbers[i+1] ) {
			printf("Fail at %d/%d (%d/%d)\n", i, i+1, numbers[i], numbers[i+1]);
			exit(0);
		}
	}

}



void merge_sort(int** list, unsigned int count, unsigned int inner_sort_width, sorter inner_sorter) {
	
	int* numbers = *list;
	// arrays of size 0 and size 1 are always sorted
	if( numbers == NULL || count < 2 ) {
		return;
	}
	
	// first use the inner sort to sort all chunks of inner_sort_width
	int from = 0;
	while( from + inner_sort_width < count ) {
//		printf("is from %d to %d\n", from, from+inner_sort_width-1);
		inner_sorter( numbers, from, from+inner_sort_width-1);
		is_sorted( numbers, from, from+inner_sort_width);
		from += inner_sort_width;
	}
//	printf("is from %d to %d\n", from, count-1);
	inner_sorter( numbers, from, count-1);
	is_sorted( numbers, from, count);

	for(int i=0; i<count; i++ ) {
//		printf( "is[%02d] = %d\n", i, numbers[i] );
	}

	
	// now perform the merge

	int* buf = malloc( sizeof(int)*count );
	if( buf == NULL ) {
		perror("malloc()");
		exit(0);
	}

//	printf("buf: %p, num: %p\n", buf, numbers);
	int* swap;
	int merge_length = inner_sort_width; // start with the sublists that are sorted
	int merges_done = 0;
	while( merge_length < count ) {

		// merge k pairs of size merge_length
		// possible optimization: if we have 24 items and merge_length is 4, we merge 4+4, 4+4, 4+4, and then 8+8, 8
		// so that last 8 gets merged agian (but effectively only copied), 
		// we could memcpy those instead of going through the while loops (but we'd have another branch)
		for(int start = 0; start < count; start += 2*merge_length) {
			// use indices for the Left of the pair and the Right of the pair
			int L = start;
			int R = start + merge_length;
			int L_end = R-1 < count-1 ? R-1 : count-1;
			int R_end = R+merge_length-1 < count-1 ? R+merge_length-1 : count-1;
			int to = 0;
//			printf("Merging %d to %d\n", L, R_end);
			
			while( start + to <= R_end ) { // we always know how many elements to merge

				// copy as many from the left pair as we can
				// (short-circuit evaluation means we never acces list[R] if R is out of bounds)
				while( L <= L_end && (R > R_end || numbers[L] <= numbers[R] ) ) {
					buf[start + to] = numbers[L];
					to++;
					L++;
				}
				
				// then copy as many as we can from the right pair
				while( R <= R_end && (L > L_end || numbers[R] < numbers[L] ) ) {
					buf[start + to] = numbers[R];
					to++;
					R++;
				}

			}

			is_sorted( buf, L, R_end-1);

//			printf("buf: %p, num: %p\n", buf, numbers);
			for(int i=0; i<R_end; i++ ) {
//				printf( "m[%02d] = %d\n", i, buf[i] );
			}


		}
		// reset the source array for merging
		swap = numbers;
		numbers = buf;
		buf = swap;
		
		merge_length *= 2;
		merges_done++;
	}

//	printf("MERGE DONE(%d) buf: %p, num: %p\n", merges_done, buf, numbers);

	*list = numbers;	
	free(buf);

//	printf("numbers @ %p\n", *list);
	for(int i=0; i<count; i++ ) {
//		printf( "D[%02d] = %d\n", i, (*list)[i] );
	}

}

void swap2( int* numbers, int start, int end ) {
	assert( start+1 == end );
	printf("Inner sort [%d,%d] = (%d,%d)\n", start, end, numbers[start], numbers[end]);
	if( numbers[start] > numbers[end] ) {
		int temp = numbers[start];
		numbers[start] = numbers[end];
		numbers[end] = temp;
	}
	printf("Done  sort [%d,%d] = (%d,%d)\n", start, end, numbers[start], numbers[end]);
}

const int gaps[8] = {701, 301, 132, 57, 23, 10, 4, 1};

// Sort an array a[start...end].
// end is inclusive
void shellsort_marcin( int* numbers, int start, int end ) {

//	printf("shellsort [%d,%d]\n", start, end);
	for(int s=start; s<=end; s++){
//		printf( "\tn[%02d] = %d\n", s, numbers[s] );
	}
	int i, j, g;

		// Do an insertion sort for each gap size.
	for( g=0; g<8; g++ ) {
		int gap = gaps[g];
//		printf("\tGapsize %d\n", gap);
		// i iterates over a virtual array defined by gapsize
		// so if gap=3 then i iterates n[3], n[6], n[9] (starting at element 2)
		if( start+gap <= end ) {
//			printf("\tStarting at n[%02d] = %d\n", start+gap, numbers[start+gap]);
		}
		for (i = start+gap; i <= end; i += gap) { 
		    int number_to_place = numbers[i];
//			printf("\tcurrent number to place n[%02d] = %d\n",i, number_to_place);
			// now iterate over every "gapth" element back to the left, moving items until
			// we find one that is lower/equal than number_to_place
			// For the first iteration, either the second number is larger and we're done, or move it to the position of the first element
			// then we try element 3, move that left to its spot (since 1 and 2 are now sorted)
//			printf("\tnow iterate from %d to %d\n", i-gap, start);
		    for (j = i-gap; j >= start && numbers[j] > number_to_place; j -= gap) {
//				printf("\tSince n[%02d] > n[%02d] (%d > %d = %d), moving n[%02d] right\n", j, i, numbers[j], number_to_place, numbers[j] > number_to_place, j );
		        numbers[j+gap] = numbers[j];
		    }
//			printf("\tHole left at n[%02d], putting %d there\n", j+gap, number_to_place);
		    numbers[j+gap] = number_to_place;
		}
		
	}
//	printf("\tss result [%d,%d]\n", start, end);
	for(int s=start; s<=end; s++){
//		printf( "\tn[%02d] = %d\n", s, numbers[s] );
	}

	
}

// read all numbers (space separated)
// (random.org output)
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
	printf("Number of integers of size %ld bytes in file: %ld\n", sizeof(int), count);

/*	
	// we don't know how many, so start with space for 16, then realloc and double
	int current_size = 256;
	int* target = malloc( sizeof(int)*current_size );
	if( target == NULL ) {
		perror("malloc()");
		exit(0);
	}
	printf("Starting target at %p\n", target);

	int buf[256];
	int read = 0;
	int count = 0;
	while( ( read = fread( buf, sizeof(int), 256, in ) ) > 0 ) {
		printf("Read %d bytes, copying to %p\n", read, &target[count]);
		memcpy( &target[count], buf, read*sizeof(int) );
		count += read;
		if( count >= current_size ) {
			int* temp = realloc( target, sizeof(int)*current_size*2 );
			if( temp == NULL ) {
				perror("realloc()");
				free(target);
				exit(0);
			}
			target = temp;
			printf("Realloc'd target to %p\n", target);
		}
	}
	*/
	
	int* target = malloc( sizeof(int)*count );
	if( target ==  NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	size_t read = fread( target, sizeof(int), count, in );
	if( read != count ) {
		printf("Read %ld ints, expected %ld\n", read, count);
		free( target );
		exit( EXIT_FAILURE );
	}	
	fclose( in );
	*numbers = target;
	
	return count;
}

int compare_int(const void* a, const void* b) {

	if ( *(int*)a == *(int*)b )
		return 0;

	if (*(int*)a < *(int*)b)
		return -1;

	return 1;
}

int main(int argc, char *argv[]) {  
	
	if( argc != 3 ) {
		puts("Usage: mergesort infile outfile");
		exit(0);
	}
	
	const char* filename_in = argv[1];
	const char* filename_out = argv[2];
	printf("Sorting file %s, writing to %s\n", filename_in, filename_out);
	
	int* numbers = NULL;
	size_t count = read_numbers( filename_in, &numbers );
//	printf( "Read %d numbers\n", count);
	for(int i=0; i<count; i++ ) {
//		printf( "N[%02d] = %d\n", i, numbers[i] );
	}
//	printf("numbers premerge %p\n", numbers);
	unsigned long start, stop;
	start = mach_absolute_time();

	merge_sort( &numbers, count, 701, shellsort_marcin );

// use builtin sort
//	mergesort( numbers, count, sizeof(int), compare_int);

	stop = mach_absolute_time();

	mach_timebase_info_data_t timebase_info;
	mach_timebase_info( &timebase_info );
	
	unsigned long elapsed_nano = (stop-start) * timebase_info.numer / timebase_info.denom;

//	printf("Diff %ld %ld  = %ld -> %ld nanos = %ld micros = %ld millis = %ld sec\n", start, stop, stop-start, elapsed_nano, elapsed_nano/1000, elapsed_nano/(1000*1000), elapsed_nano/(1000*1000*1000));

	// write count,nano, micro, milli, sec
	fprintf(stderr, "%ld,%ld,%ld,%ld,%ld\n", count,elapsed_nano, elapsed_nano/1000, elapsed_nano/(1000*1000), elapsed_nano/(1000*1000*1000));

//	printf("numbers postmerge %p\n", numbers);
	
	for( int i=0; i<count; i++ ) {
//		printf("n[%02d] = %d\n", i, numbers[i] );
	}
	
	is_sorted( numbers, 0, count );
	
	// write to out
	FILE* out = fopen( filename_out, "wb" );
	if( out == NULL ) {
		perror("fopen()");
		free(numbers);
		exit( EXIT_FAILURE );
	}
	size_t written = fwrite( numbers, sizeof(int), count, out );
	if( written != count ) {
		perror("frwrite()");
		exit( EXIT_FAILURE );
	}	
	fclose( out );
	
//	printf("Freeing %p\n", numbers);
	free( numbers );
	return 0;
}
