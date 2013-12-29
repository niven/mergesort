#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "unistd.h"

#include "mach/mach_time.h"

#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

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
void pyramid_merge(void* base, size_t nel, size_t width, comparator compare, size_t inner_sort_width, sorter inner_sorter) {

	int block_width = inner_sort_width;
	
	char* in = (char*)base;
	char* buf = malloc( nel * width );
	if( buf == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}

	// a block is a subarray with width equal to block_width
	// so if count=100 and block_width = 20 our array consists of 5 blocks
	int blocks_done = 0;

	int index_start = 0, index_end = 0;

	// for the first merge (blocks_done==2) we have to read from in
	char* left = (char*)base;
	char* right = (char*)base;
	char* to = buf;

	say("in array: %p, buf %p\n", base, buf);

	while( index_start < nel ) {

		index_end = MIN(index_start + block_width, nel);// temp name, too manythings called "to"
	
		// shellsort will inplace sort a block of the in array
		inner_sorter( in + index_start*width, index_end-index_start, width, compare );
		blocks_done++;

		say("Blocks done: %d ", blocks_done);
		print_array( (int*)in, index_start, index_end, block_width );
	
		int mergecounter = blocks_done;
		int merge_width = block_width;
		int m = 0;
		// now merge 2 subsections of the array, and if the number of blocks is even, it means we can do more merges

		// always read from in and merge to buf for the first merge pass
		to = buf;
		left = right = in;
		while( mergecounter % 2 == 0 ) {

			say("Mergecounter at %d\n", mergecounter );
		
			// how much we would have done if each block were block_width (every single one is except maybe the last one)
			int start = (blocks_done*block_width) - 2*merge_width; 
			char* L = left + start*width; // start of "left" array
			char* R = right + (start + merge_width)*width; // start of "right" array

			char* L_end = left + MIN( start+merge_width, nel )*width;
			char* R_end = right + MIN( start+ 2*merge_width, nel )*width; 

		//	say( "Merging %d elements: [%d - %d] (%p) with [%d - %d] (%p) to [%d - %d] %p\n", index_end-index_start, L, L_end, left, R, R_end, right, t, R_end, to );
			say("Merging %d elements to %p\n", index_end-index_start, to);
			say("Premerge left [%d - %d] (%p):\n", (L-left)/width, (L_end-left)/width-1, left);
			print_array( (int*)left, (L-left)/width, (L_end-left)/width, block_width );
			say("Premerge right [%d - %d] (%p):\n", (R-right)/width, (R_end-right)/width-1, right);
			print_array( (int*)right, (R-right)/width, (R_end-right)/width, block_width );

			say("remaining left %d, remaining right %d\n", (L_end-L)/width, (R_end-R)/width);
			while( L<L_end || R<R_end ) { // until left and right run out

				say("\t L=%d\t R=%d\t to=%d\n", (L-left)/width, (R-right)/width, (to-buf)/width);

				// copy items from left array as long as they are lte
				// (short-circuit evaluation means we never acces list[R] if R is out of bounds)
				// of course R is not modified here, but R might have "ran out" so here we need to copy the rest of L
				m = 0;
				while( (L+m) < L_end && (R >= R_end || compare( L+m, R ) <= 0 ) ) {

					say("\tto[%d] = L[%d] (%3d)\n", (to-buf+m)/width, (L-left+m)/width, *(int*) (L+m) );
					m += width;
				}
				say("Copying %d bytes (%d elements)\n", m, m/width);
				memcpy( to, L, m );
				to += m;
				L += m;
				
				// copy items from right array as long as they are lte
				m = 0;
				while( (R+m) < R_end && (L >= L_end || compare( R+m, L ) <= 0 ) ) {

					say("\tto[%d] = R[%d] (%3d)\n", (to-buf+m)/width, (R-right+m)/width, *(int*) (R+m) );
					m += width;
				}
				say("Copying %d bytes (%d elements)\n", m, m/width);
				memcpy( to, R, m );
				to += m;
				R += m;
				
				say("remaining left %d, remaining right %d\n", (L_end-L)/width, (R_end-R)/width);
			}

			say("Postmerge [%d - %d] (%p):\n", start, index_end-1, to);
			print_array( (int*)(to - 2*block_width*width), 0, 2*block_width, block_width );

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
		// the above is terrible and ugly

		say( "second=%d = 2^%d -> left=%p\n", second, pow, left );
		say( "to=%p\n", to );
		say( "Current in [%d - %d] (%p):\n", 0, nel-1, in );
		print_array( (int*)in, 0, nel, block_width );
		say( "Current buf [%d - %d] (%p):\n", 0, nel-1, buf );
		print_array( (int*)buf, 0, nel, block_width );
	
		// Merge the first to last block with the second to last one
		// (maybe swap meaning of first and second since we're doing this backwards?)
		// the start offsets are "back from the end by offsets" which we'd have to keep track of over more merges
		// but blocks_done already remove "first" every time which makes it work. think about it :)
		int start = (blocks_done - second) * block_width;
		int L = start; // start of "left" array
		int R = blocks_done * block_width; // start of "right" array

		int L_end = R-1;
		int R_end = nel-1; // in this case it's always the end of the array 
		int t = start; // where we start writing to the target array (corresponds to a)
		int m = 0;
		
		say( "Merging %d blocks from left (%p) with %d blocks from right (%p)\n", second, left, first, right );
		say( "Merging %d elements: [%d - %d] (%p) with [%d - %d] (%p) to [%d - %d] %p\n", R_end-L+1, L, L_end, left, R, R_end, right, t, R_end, to );
		say( "Premerge left [%d - %d] (%p):\n", L, L_end, left );
		print_array( (int*)left, L, L_end+1, block_width );
		say( "Premerge right [%d - %d] (%p):\n", R, R_end, right );
		print_array( (int*)right, R, R_end+1, block_width );

		while( t <= R_end ) { // until we've copied everything to the target array

			say( "\t L=%d\t R=%d\t t=%d\n", L, R, t );

			// copy items from left array as long as they are lte
			// (short-circuit evaluation means we never acces list[R] if R is out of bounds)
			// of course R is not modified here, but R might have "ran out" so here we need to copy the rest of L
			m = 0;
			while( (L+m) <= L_end && (R > R_end || compare( left + (L+m)*width, right + R*width ) <= 0) ) {

				say("\tto[%d] = R[%d] (%3d)\n", t+m, L+m, *(int*) (left + (L+m)*width) );
				m++;
			}
			say("Copying %d bytes (%d elements)\n", m*width, m);
			memcpy( to + (t*width), left + (L*width), m*width );
			t += m;
			L += m;
			
			// copy items from right array as long as they are lte
			m = 0;
			while( (R+m) <= R_end && (L > L_end || compare( right + (R+m)*width, left + L*width ) <= 0) ) {

				say("\tto[%d] = R[%d] (%3d)\n", t+m, R+m, *(int*) (right + (R+m)*width) );
				m++;
			}
			say("Copying %d bytes (%d elements)\n", m*width, m);
			memcpy( to + (t*width), right + (R*width), m*width );
			t += m;
			R += m;

		}

		say( "Postmerge [%d - %d] (%p):\n", start, t, to );
		print_array( (int*)to, start, t, block_width );
	
		// now swap pointers
		right = to; // to is what we created, which is the smaller block at the end
		left = to; // guess
		to = right == base ? buf : base;
	
	}

	// unfortunate result of the stdlib sort interface: we might end up with the end result in buf
	// and not in wherever base points to. In that case we copy over everything :(
	if( to == base ) {
		memcpy( base, buf, nel*width );
		free( buf );
	} else {
		free( to );
	}
}

void sort_function( void* base, size_t nel, size_t width, comparator compare ) {
	
	int block_width = 4;
	const char* env_block_width = getenv( "SORTER_BLOCK_WIDTH" );
	if( env_block_width != NULL ) {
		block_width = atoi( env_block_width );
	}
	
	pyramid_merge( base, nel, width, compare, block_width, shellsort );
	
}
