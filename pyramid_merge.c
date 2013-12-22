#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "unistd.h"

#include "mach/mach_time.h"

#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

struct statictics {
	int comparisons;
	int moves;
} stats;

int block_width = 8;

void mergesort_pyramid( int** numbers, int count ) {

	int* in = *numbers;
int* buf = malloc( count * sizeof(int) );
if( buf == NULL ) {
	perror("malloc()");
	exit(0);
}

// a block is a subarray with width equal to block_width
// so if count=100 and block_width = 20 our array consists of 5 blocks
int blocks_done = 0;

/*
Some examples:

Just sorted block:
	

	1: do nothing
	
	2: merge 1+2
	
	3: do nothing
	
	4: merge 3+4, then (1+2) + (3+4)
	
	6: merge 5+6, then do nothing. we now have (1+2+3+4)(5+6)
	
	7: do nothing
	
	8: merge 7+8, then (5+6)+(7+8), then ((1+2)+(3+4)) + ((5+6)+(7+8))
	
	10: merge 9+10, then do nothing, we have (1+2+3+4+5+6+7+8),(9+10)
	
	12: merge 11+12, then (9+10)+(11+12), then do nothing, we have (1+2+3+4+5+6+7+8),(9+10+11+12)

	13: do nothing, we have (1+2+3+4+5+6+7+8),(9+10+11+12), 13
	
	14: merge 13+14, then done. we should have  (1+2+3+4+5+6+7+8),(9+10+11+12),(13+14)
	
	So every time we merge upto what we just sorted (the endpoint is always the same),
	but how far we look back doubles and so does the block size
*/

int from = 0;

// for the first merge (blocks_done==2) we have to read from in
int* left = in;
int* right = in;
int* to = buf;

#ifdef VERBOSE
printf("in array: %p, buf %p\n", in, buf);
#endif
while( from < count ) {

	int from_to = MIN(from + block_width, count);// temp name, too manythings called "to"
	
	// shellsort will inplace sort a block of the in array
#ifdef VERBOSE
	printf( "\nSorting [%d - %d]\n[ ", from, from_to-1 );
	for(int i=from; i<from_to; i++) {
		printf("%3d ", in[i]);
	}
	printf("]\n");
#endif

	shellsort( in, from, from_to-1 ); // maybe not range but start and count would be better
	blocks_done++;
#ifdef VERBOSE
	printf("Blocks done %d\n[ ", blocks_done);
	for(int i=from; i<from_to; i++) {
		printf("%3d ", in[i]);
	}
	printf("]\n");
#endif	
	
	int mergecounter = blocks_done;
	int merge_width = block_width;

	// now merge 2 subsections of the array, and if the number of blocks is even, it means we can do more merges

	// always read from in and merge to buf for the first merge pass
	to = buf;
	left = right = in;
	while( mergecounter % 2 == 0 ) {
#ifdef VERBOSE
		printf("Mergecounter at %d\n", mergecounter );
#endif		
		// how much we would have done if each block were block_width (every single one is except maybe the last one)
		int start = (blocks_done*block_width) - 2*merge_width; 
		int L = start; // start of "left" array
		int R = start + merge_width; // start of "right" array

		int L_end = MIN( R-1, count-1 );
		int R_end = MIN( R+merge_width-1, count-1 ); 
		int sent=0;
		int t = start; // where we start writing to the target array (corresponds to a)
#ifdef VERBOSE
		printf( "Merging %d elements: [%d - %d] (%p) with [%d - %d] (%p) to [%d - %d] %p\n", from_to-L, L, L_end, left, R, R_end, right, t, R_end, to );
		printf("Premerge [%d - %d] (%p):\n[ ", L, L_end, left);
		for(int i=L; i<=L_end; i++) {
			printf("%3d ", left[i]);
			if( i< L_end-1 && (i+1)%block_width==0 ) {
				printf("]\n[ ");
			}
		}
		printf("]\n");
		printf("Premerge [%d - %d] (%p):\n[ ", R, R_end, right);
		for(int i=R; i<=R_end; i++) {
			printf("%3d ", right[i]);
			if( i< R_end-1 && (i+1)%block_width==0 ) {
				printf("]\n[ ");
			}
		}
		printf("]\n");
#endif

		while( t <= R_end ) { // until we've copied everything to the target array
#ifdef VERBOSE
			printf("\t L=%d\t R=%d\t t=%d\n", L, R, t);
#endif
			// copy items from left array as long as they are lte
			// (short-circuit evaluation means we never acces list[R] if R is out of bounds)
			// of course R is not modified here, but R might have "ran out" so here we need to copy the rest of L
			stats.comparisons++;
			while( L <= L_end && (R > R_end || left[L] <= right[R]) ) {
#ifdef VERBOSE
				printf("\tto[%d] = L[%d] (%3d)\n", t, L, left[L] );
#endif
				to[t] = left[L];
				t++;
				L++;
				stats.comparisons++;
				stats.moves++;
			}

			// copy items from right array as long as they are lte
			stats.comparisons++;
			while( R <= R_end && (L > L_end || right[R] <= left[L]) ) {
#ifdef VERBOSE
				printf("\tto[%d] = R[%d] (%3d)\n", t, R, right[R] );
#endif
				to[t] = right[R];
				t++;
				R++;
				stats.comparisons++;
				stats.moves++;
			}

			// maybe memcpy?
		}

#ifdef VERBOSE
		printf("Postmerge [%d - %d] (%p):\n[ ", start, from_to-1, to);
		for(int i=start; i<from_to; i++) {
			printf("%3d ", to[i]);
			if( i< from_to-1 &&  (i+1)%block_width==0 ) {
				printf("]\n[ ");
			}
		}
		printf("]\n");
#endif
		// if we do sequential merges, we merge the result of what we just did, with an older one (of equal size)
		// so where we read from and write to swaps
		to = to == in ? buf : in; // target is the other one
		left = right = ( to == in ? buf : in );	// we read from wherever we're not writing
		
#ifdef VERBOSE
		printf("Pointers now: to=%p, left=right=%p=%p\n", to, left, right);
#endif		
		// while we have even numbers, we merge blocks up
		mergecounter /= 2;
		merge_width *= 2; // every time we merge twice as much
		// endpoint stays the same
	}
	
	
	// process the next block
	from += block_width;
}

#ifdef VERBOSE
printf("Every block sorted (from=%d), now doing remaining merges\n", from);
#endif

// at this point every block is sorted and the last thing that happened was either
// a sort: left/right/in has unmerged blocks, to/buf has the merged rest
// or a merge: this means to doesn't point to the merged stuff
// so which one is it?

if( blocks_done % 2 == 0 ) {
#ifdef VERBOSE
	printf("Even blocks done, last thing we did was a merge to: %p\n", left);
#endif
} else if( blocks_done == 1 ) {
	// inelegent special case: if block_width > number of items, shellsort did all the
	// work and we can just return the input pointer
	return;
} else {
#ifdef VERBOSE
	printf("Odd number of blocks, single remaining block to merge is here: %p\n", in);
#endif
	right = in;
	left = buf;
	to = in;
}

// now we are either done or are left with a situation like example 14 or 13 or one of those
// how to proceed?
// the trick is to factor the blocks_done into powers of 2
// lolwut?
// so 14 = 8 + 4 + 2 which describes the leftover pyramid
// so 13 = 8 + 4 + 1
// so 8 = 8 (we were already done)
// so 10 = 8 + 2
// etc
// conveniently, numbers are actually made up of powers of two (unless you're running this on a decimal computer)

#ifdef VERBOSE
printf( "Doing wrapup merges for %d blocks\n", blocks_done );
#endif
int first, second;
while( blocks_done & blocks_done-1 ) { // test to see if single bit is set
	// so getting the two lowest powers of 2 from blocks_done would be easier if we could read the shift carry-out
	// but that's not a thing. Never figured not having access to the carry bit would be a problem I'd be having.
	
	// problem: we have 0b1100 and I want 0b1000 and 0b0100
	// or: 0b10010001 and I want 0b10000 and 0b1
	
	/*
	Holy linecount Batman! I typed up all that code, FFS!
	That is an incredibly dumb way of doing this. Leaving it here for posterity.
	
	pow2 = -1;
	while( (blocks_done & 1) == 0 ) { // put the rightmost 1 in the LSBit position
		blocks_done >>= 1;
		pow2++;
	}
	blocks_done >>= 1; // move it off
	pow2++;

	first = 1 << pow2;

	while( (blocks_done & 1) == 0 ) { // continue with the next one
		blocks_done >>= 1;
		pow2++;
	}
	blocks_done >>= 1; // move it off
	pow2++;
	
	second = 1 << pow2;
	*/

	// assume blocks_done is XXX1000. then bd-1 = XXX0111
	// so XXX0111 ^ XXX1000 = 00001111
	// then add 1 to get 00010000, then shift to get 00001000 :)
	first = ((blocks_done ^ (blocks_done-1)) +1) >> 1;
	blocks_done ^= first; // remove this power
	second = ((blocks_done ^ (blocks_done-1)) +1) >> 1;

#ifdef VERBOSE
	printf( "Remainder: %d first: %d second: %d\n", blocks_done, first, second );
	// so now merge the first to last block with the second to last one
	// YOYOYO those widths make no sense, and the right is always until end of array
	// maybe swap meaning of first and second since we're doing this backwards?
	printf("Current in [%d - %d] (%p):\n[ 0-%2d] [ ", 0, count, in, block_width-1);
	for(int i=0; i<count; i++) {
		printf("%3d ", in[i]);
		if( i< count-1 &&  (i+1)%block_width==0 ) {
			printf("]\n[%2d-%2d] [ ", i+1, MIN(i+block_width, count-1));
		}
	}
	printf("]\n");
	printf("Current buf [%d - %d] (%p):\n[ 0-%2d] [ ", 0, count, buf, block_width-1);
	for(int i=0; i<count; i++) {
		printf("%3d ", buf[i]);
		if( i< count-1 &&  (i+1)%block_width==0 ) {
			printf("]\n[%2d-%2d] [ ", i+1, MIN(i+block_width, count-1));
		}
	}
	printf("]\n");
#endif	
	// merge step here
	// the start offsets are "back from the end by offsets" which we'd have to keep track of over more merges
	// but blocks_done already remove "first" every time which makes it work. think about it :)
	int start = (blocks_done - second) * block_width;
	int L = start; // start of "left" array
	int R = blocks_done * block_width; // start of "right" array

	int L_end = R-1;
	int R_end = count-1; // in this case it's always the end of the array 
	int t = start; // where we start writing to the target array (corresponds to a)
#ifdef VERBOSE
	printf("Merging %d blocks from left (%p) with %d blocks from right (%p)\n", second, left, first, right );
	printf( "Merging %d elements: [%d - %d] (%p) with [%d - %d] (%p) to [%d - %d] %p\n", R_end-L+1, L, L_end, left, R, R_end, right, t, R_end, to );
	printf("Premerge [%d - %d] (%p):\n[ ", L, L_end, left);
	for(int i=L; i<=L_end; i++) {
		printf("%3d ", left[i]);
		if( i< L_end-1 && (i+1)%block_width==0 ) {
			printf("]\n[ ");
		}
	}
	printf("]\n");
	printf("Premerge [%d - %d] (%p):\n[ ", R, R_end, right);
	for(int i=R; i<=R_end; i++) {
		printf("%3d ", right[i]);
		if( i< R_end-1 && (i+1)%block_width==0 ) {
			printf("]\n[ ");
		}
	}
	printf("]\n");
#endif

	while( t <= R_end ) { // until we've copied everything to the target array
#ifdef VERBOSE
		printf("\t L=%d\t R=%d\t t=%d\n", L, R, t);
#endif
		// copy items from left array as long as they are lte
		// (short-circuit evaluation means we never acces list[R] if R is out of bounds)
		// of course R is not modified here, but R might have "ran out" so here we need to copy the rest of L
		stats.comparisons++;
		while( L <= L_end && (R > R_end || left[L] <= right[R]) ) {
#ifdef VERBOSE
			printf("\tto[%d] = L[%d] (%3d)\n", t, L, left[L] );
#endif
			to[t] = left[L];
			t++;
			L++;
			stats.comparisons++;
			stats.moves++;
		}

		// copy items from right array as long as they are lte
		stats.comparisons++;
		while( R <= R_end && (L > L_end || right[R] <= left[L]) ) {
#ifdef VERBOSE
			printf("\tto[%d] = R[%d] (%3d)\n", t, R, right[R] );
#endif
			to[t] = right[R];
			t++;
			R++;
			stats.comparisons++;
			stats.moves++;
		}

		// maybe memcpy?
	}

#ifdef VERBOSE
	printf("Postmerge [%d - %d] (%p):\n[ ", start, t, to);
	for(int i=start; i<t; i++) {
		printf("%3d ", to[i]);
		if( i< t-1 &&  (i+1)%block_width==0 ) {
			printf("]\n[ ");
		}
	}
	printf("]\n");
#endif
	
	// now swap pointers
	right = to; // to is what we created, which is the smaller block at the end
	left = to; // guess
	to = right == in ? buf : in;
	
}

// now we're done, so pick the right thing to set numbers to
// which is the opposite of *to
if( to == in ) {
	*numbers = buf;
	free(to);
} else {
	free(buf);
}



}



int main(int argc, char *argv[]) {  
	
	if( argc != 4 ) {
		puts("Usage: pyramid_merge block_width infile outfile");
		exit(0);
	}
		
	block_width = atoi( argv[1] );
	const char* filename_in = argv[2];
	const char* filename_out = argv[3];
	printf("Sorting file %s, writing to %s\n", filename_in, filename_out);
	
	int* numbers = NULL;
	size_t count = read_numbers( filename_in, &numbers );
#ifdef VERBOSE
	printf( "Read %zu numbers\n", count);
	for(size_t i=0; i<count; i++ ) {
		printf( "N[%02zu] = %d\n", i, numbers[i] );
	}
	printf("numbers premerge %p\n", numbers);
#endif
	unsigned long start, stop;
	start = mach_absolute_time();

	stats.comparisons = 0;
	stats.moves = 0;
	mergesort_pyramid( &numbers, count );
#ifdef VERBOSE
	printf("numbers postmerge %p\n", numbers);
#endif
// use builtin sort
	//	mergesort( numbers, count, sizeof(int), compare_int);

	stop = mach_absolute_time();

	mach_timebase_info_data_t timebase_info;
	mach_timebase_info( &timebase_info );
	
	unsigned long elapsed_nano = (stop-start) * timebase_info.numer / timebase_info.denom;

	// write count,nano, micro, milli, sec
	fprintf(stderr, "%zu,%ld\n", count,elapsed_nano);
	
#ifdef VERBOSE
	for( size_t i=0; i<count; i++ ) {
		printf("n[%02zu] = %d\n", i, numbers[i] );
	}

	printf( "Stats\n\tComparisons: %d\n\tMoves: %d\n", stats.comparisons, stats.moves );
#endif	
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
	
	free( numbers );
	return 0;
}
