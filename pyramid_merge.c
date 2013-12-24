#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "unistd.h"

#include "mach/mach_time.h"

#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

int block_width = 8;

/*
Merge sorted blocks as soon as possible to hopefully optimize cpu cache usage (well, that's the theory)

After sorting block:

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

	int index_start = 0, index_end = 0;

	// for the first merge (blocks_done==2) we have to read from in
	int* left = in;
	int* right = in;
	int* to = buf;

	say("in array: %p, buf %p\n", in, buf);

	while( index_start < count ) {

		index_end = MIN(index_start + block_width, count);// temp name, too manythings called "to"
	
		say( "\nSorting [%d - %d]\n[ ", index_start, index_end-1 );
		for(int i=index_start; i<index_end; i++) {
			say("%3d ", in[i]);
		}
		say("]\n");


		// shellsort will inplace sort a block of the in array
		shellsort( in, index_start, index_end-1 ); // maybe not range but start and count would be better
		blocks_done++;

		say("Blocks done %d\n[ ", blocks_done);
		for(int i=index_start; i<index_end; i++) {
			say("%3d ", in[i]);
		}
		say("]\n");
	
	
		int mergecounter = blocks_done;
		int merge_width = block_width;

		// now merge 2 subsections of the array, and if the number of blocks is even, it means we can do more merges

		// always read from in and merge to buf for the first merge pass
		to = buf;
		left = right = in;
		while( mergecounter % 2 == 0 ) {

			say("Mergecounter at %d\n", mergecounter );
		
			// how much we would have done if each block were block_width (every single one is except maybe the last one)
			int start = (blocks_done*block_width) - 2*merge_width; 
			int L = start; // start of "left" array
			int R = start + merge_width; // start of "right" array

			int L_end = MIN( R-1, count-1 );
			int R_end = MIN( R+merge_width-1, count-1 ); 
			int t = start; // where we start writing to the target array (corresponds to a)

			say( "Merging %d elements: [%d - %d] (%p) with [%d - %d] (%p) to [%d - %d] %p\n", index_end-L, L, L_end, left, R, R_end, right, t, R_end, to );
			say("Premerge [%d - %d] (%p):\n[ ", L, L_end, left);
			for(int i=L; i<=L_end; i++) {
				say("%3d ", left[i]);
				if( i< L_end-1 && (i+1)%block_width==0 ) {
					say("]\n[ ");
				}
			}
			say("]\n");
			say("Premerge [%d - %d] (%p):\n[ ", R, R_end, right);
			for(int i=R; i<=R_end; i++) {
				say("%3d ", right[i]);
				if( i< R_end-1 && (i+1)%block_width==0 ) {
					say("]\n[ ");
				}
			}
			say("]\n");


			while( t <= R_end ) { // until we've copied everything to the target array

				say("\t L=%d\t R=%d\t t=%d\n", L, R, t);

				// copy items from left array as long as they are lte
				// (short-circuit evaluation means we never acces list[R] if R is out of bounds)
				// of course R is not modified here, but R might have "ran out" so here we need to copy the rest of L
				while( L <= L_end && (R > R_end || left[L] <= right[R]) ) {

					say("\tto[%d] = L[%d] (%3d)\n", t, L, left[L] );

					to[t] = left[L];
					t++;
					L++;
				}

				// copy items from right array as long as they are lte
				while( R <= R_end && (L > L_end || right[R] <= left[L]) ) {

					say("\tto[%d] = R[%d] (%3d)\n", t, R, right[R] );

					to[t] = right[R];
					t++;
					R++;
				}

				// maybe memcpy?
			}


			say("Postmerge [%d - %d] (%p):\n[ ", start, index_end-1, to);
			for(int i=start; i<index_end; i++) {
				say("%3d ", to[i]);
				if( i< index_end-1 &&  (i+1)%block_width==0 ) {
					say("]\n[ ");
				}
			}
			say("]\n");

			// if we do sequential merges, we merge the result of what we just did, with an older one (of equal size)
			// so where we read from and write to swaps
			to = to == in ? buf : in; // target is the other one
			left = right = ( to == in ? buf : in );	// we read from wherever we're not writing
		

			say("Pointers now: to=%p, left=right=%p=%p\n", to, left, right);
		
			// while we have even numbers, we merge blocks up
			mergecounter /= 2;
			merge_width *= 2; // every time we merge twice as much
			// endpoint stays the same
		}
	
	
		// process the next block
		index_start += block_width;
	}

	say("Every block sorted (index_start=%d), now doing remaining merges\n", index_start);

	// inelegent special case: if block_width > number of items, shellsort did all the
	// work and we can just return
	if( blocks_done == 1 ) {
		return;
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

	say( "Doing wrapup merges for %d blocks\n", blocks_done );

	int first, second;
	while( blocks_done & blocks_done-1 ) { // test to see if single bit is set
		/* 
		Getting the two lowest powers of 2 from blocks_done would be easier if we could read the shift carry-out
		but that's not a thing. Never figured not having access to the carry bit would be a problem I'd be having.
	
		problem: we have 0b1100 and I want 0b1000 and 0b0100
		or: 0b10010001 and I want 0b10000 and 0b1
	
		assume blocks_done is XXX1000. then bd-1 = XXX0111
		so XXX0111 ^ XXX1000 = 00001111
		then add 1 to get 00010000, then shift to get 00001000 :)
		*/
		first = ((blocks_done ^ (blocks_done-1)) +1) >> 1;
		blocks_done ^= first; // remove this power
		second = ((blocks_done ^ (blocks_done-1)) +1) >> 1;
		say( "Remainder: %d first: %d second: %d\n", blocks_done, first, second );

		/*
		We have to figure out where left, right and to should point to. Once we do and we
		go into the merge loop we alternate left/right and to which is correct since we always
		merge a smaller block with a larger one (I think)
		
		So where do they point to?
		If we merge 2 blocks they go from in to buf (2^1)
		If we merge 2+2 blocks they go from buf to in (2^2)
		If we merge 4+4 blocks they go from in to buf again etc (2^3)
		so blocks_done = 2^n and if n is odd we wrote it to buf, if n is even we wrote it to in
		*/
		int n = first;
		int pow = -1;
		while( n > 0 ) {
			n >>= 1;
			pow++;
		}
		if( pow % 2 == 0 ) {
			right = in;
		} else {
			right = buf;
		}
		say( "first=%d = 2^%d -> right=%p\n", first, pow, right );
		n = second;
		pow = -1;
		while( n > 0 ) {
			n >>= 1;
			pow++;
		}
		if( pow % 2 == 0 ) {
			left = in;
			to = buf;
		} else {
			left = buf;
			to = in;
		}
		say( "second=%d = 2^%d -> left=%p\n", second, pow, left );
		say( "to=%p\n", to );
		// so now merge the first to last block with the second to last one
		// YOYOYO those widths make no sense, and the right is always until end of array
		// maybe swap meaning of first and second since we're doing this backwards?
		say("Current in [%d - %d] (%p):\n[ 0-%2d] [ ", 0, count, in, block_width-1);
		for(int i=0; i<count; i++) {
			say("%3d ", in[i]);
			if( i< count-1 &&  (i+1)%block_width==0 ) {
				say("]\n[%2d-%2d] [ ", i+1, MIN(i+block_width, count-1));
			}
		}
		say("]\n");
		say("Current buf [%d - %d] (%p):\n[ 0-%2d] [ ", 0, count, buf, block_width-1);
		for(int i=0; i<count; i++) {
			say("%3d ", buf[i]);
			if( i< count-1 &&  (i+1)%block_width==0 ) {
				say("]\n[%2d-%2d] [ ", i+1, MIN(i+block_width, count-1));
			}
		}
		say("]\n");
	
		// merge step here
		// the start offsets are "back from the end by offsets" which we'd have to keep track of over more merges
		// but blocks_done already remove "first" every time which makes it work. think about it :)
		int start = (blocks_done - second) * block_width;
		int L = start; // start of "left" array
		int R = blocks_done * block_width; // start of "right" array

		int L_end = R-1;
		int R_end = count-1; // in this case it's always the end of the array 
		int t = start; // where we start writing to the target array (corresponds to a)

		say("Merging %d blocks from left (%p) with %d blocks from right (%p)\n", second, left, first, right );
		say( "Merging %d elements: [%d - %d] (%p) with [%d - %d] (%p) to [%d - %d] %p\n", R_end-L+1, L, L_end, left, R, R_end, right, t, R_end, to );
		say("Premerge [%d - %d] (%p):\n[ ", L, L_end, left);
		for(int i=L; i<=L_end; i++) {
			say("%3d ", left[i]);
			if( i< L_end-1 && (i+1)%block_width==0 ) {
				say("]\n[ ");
			}
		}
		say("]\n");
		say("Premerge [%d - %d] (%p):\n[ ", R, R_end, right);
		for(int i=R; i<=R_end; i++) {
			say("%3d ", right[i]);
			if( i< R_end-1 && (i+1)%block_width==0 ) {
				say("]\n[ ");
			}
		}
		say("]\n");


		while( t <= R_end ) { // until we've copied everything to the target array

			say("\t L=%d\t R=%d\t t=%d\n", L, R, t);

			// copy items from left array as long as they are lte
			// (short-circuit evaluation means we never acces list[R] if R is out of bounds)
			// of course R is not modified here, but R might have "ran out" so here we need to copy the rest of L
			while( L <= L_end && (R > R_end || left[L] <= right[R]) ) {

				say("\tto[%d] = L[%d] (%3d)\n", t, L, left[L] );

				to[t] = left[L];
				t++;
				L++;
			}

			// copy items from right array as long as they are lte
			while( R <= R_end && (L > L_end || right[R] <= left[L]) ) {

				say("\tto[%d] = R[%d] (%3d)\n", t, R, right[R] );

				to[t] = right[R];
				t++;
				R++;
			}

			// maybe memcpy?
		}


		say("Postmerge [%d - %d] (%p):\n[ ", start, t, to);
		for(int i=start; i<t; i++) {
			say("%3d ", to[i]);
			if( i< t-1 &&  (i+1)%block_width==0 ) {
				say("]\n[ ");
			}
		}
		say("]\n");

	
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
	say("Sorting file %s, writing to %s\n", filename_in, filename_out);
	
	int* numbers = NULL;
	size_t count = read_numbers( filename_in, &numbers );

	say( "Read %zu numbers\n", count);
	for(size_t i=0; i<count; i++ ) {
		say( "N[%02zu] = %d\n", i, numbers[i] );
	}
	say("numbers premerge %p\n", numbers);

	unsigned long start, stop;
	start = mach_absolute_time();

	mergesort_pyramid( &numbers, count );

	say("numbers postmerge %p\n", numbers);

// use builtin sort
	//	mergesort( numbers, count, sizeof(int), compare_int);

	stop = mach_absolute_time();

	mach_timebase_info_data_t timebase_info;
	mach_timebase_info( &timebase_info );
	
	unsigned long elapsed_nano = (stop-start) * timebase_info.numer / timebase_info.denom;

	// write count,nano, micro, milli, sec
	fprintf(stderr, "%zu,%ld\n", count,elapsed_nano);
	

	for( size_t i=0; i<count; i++ ) {
		say("n[%02zu] = %d\n", i, numbers[i] );
	}

	
	is_sorted( numbers, 0, count );
	
	write_numbers( numbers, count, filename_out );

	free( numbers );
	
	exit( EXIT_SUCCESS );
}
