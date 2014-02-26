#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* for SUPPRESS_STDOUT macros*/
#include <unistd.h>
#include <fcntl.h> 


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
void pyramid_mergesort(void* base, size_t nel, size_t width, comparator compare, size_t inner_sort_width, sorter inner_sorter) {

	uint32_t elements_per_block = inner_sort_width;
	
	char* in = (char*)base;
	char* buf = malloc( nel * width );
	if( buf == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}

	// a block is a subarray with width equal to elements_per_block
	// so if count=100 and elements_per_block = 20 our array consists of 5 blocks
	uint32_t blocks_done = 0;

	uint32_t index_start = 0, index_end = 0;

	// for the first merge (blocks_done==2) we have to read from in
	char* from = (char*)base;
	char* to = buf;

	// these define the ranges we're going to merge
	char *L, *L_end, *R, *R_end;

	say("in array: %p, buf %p\n", base, buf);

	while( index_start < nel ) {

		index_end = MIN(index_start + elements_per_block, nel);// temp name, too manythings called "to"
	
		// shellsort will inplace sort a block of the in array
		SUPPRESS_STDOUT
		inner_sorter( in + index_start*width, index_end-index_start, width, compare );
		RETURN_STDOUT
		blocks_done++;

		say("Blocks done: %d ", blocks_done);
		print_array( (widget*)in, index_start, index_end, elements_per_block );
	
		uint32_t mergecounter = blocks_done;
		uint32_t merge_width = elements_per_block;
		uint32_t m = 0;
		// now merge 2 subsections of the array, and if the number of blocks is even, it means we can do more merges

		// always read from in and merge to buf for the first merge pass
		from = in;
		to = buf;
		while( mergecounter % 2 == 0 ) {

			say("Mergecounter at %d\n", mergecounter );
		
			// how much we would have done if each block were elements_per_block (every single one is except maybe the last one)
			uint32_t start = (blocks_done*elements_per_block) - 2*merge_width; 
			L = from + start*width; // start of "left" array
			R = from + (start + merge_width)*width; // start of "right" array

			L_end = from + MIN( start+merge_width, nel )*width;
			R_end = from + MIN( start+ 2*merge_width, nel )*width; 

			say("Merging %d elements to %p (%s)\n", (R_end-L)/width, to, to==in?"in":"buf");
			say("Premerge left [%d - %d] (%p):\n", (L-from)/width, (L_end-from)/width-1, from);
			print_array( (widget*)from, (L-from)/width, (L_end-from)/width, elements_per_block );
			say("Premerge right [%d - %d] (%p):\n", (R-from)/width, (R_end-from)/width-1, from);
			print_array( (widget*)from, (R-from)/width, (R_end-from)/width, elements_per_block );

			to = to + start*width; // offset is always the same as left

			say("remaining left %d, remaining right %d\n", (L_end-L)/width, (R_end-R)/width);
			while( L<L_end && R<R_end ) { // until left and right run out

				say("\t L=%d\t R=%d\t to=%d\n", (L-from)/width, (R-from)/width, (to-buf)/width);

				// copy items from left array as long as they are lte
				// (short-circuit evaluation means we never acces list[R] if R is out of bounds)
				// of course R is not modified here, but R might have "ran out" so here we need to copy the rest of L
				m = 0;
				while( (L+m) < L_end && compare( L+m, R ) <= 0 ) {

					say("\tto[%d] = L[%d] (%3d)\n", (to-buf+m)/width, (L-from+m)/width, *(int*) (L+m) );
					m += width;
				}
				say("Copying %d bytes (%d elements)\n", m, m/width);
				memcpy( to, L, m );
				to += m;
				L += m;
				
				if( L >= L_end ) { // don't overshoot
					break;
				}
				
				// copy items from right array as long as they are lte
				m = 0;
				while( (R+m) < R_end && compare( R+m, L ) <= 0 ) {

					say("\tto[%d] = R[%d] (%3d)\n", (to-buf+m)/width, (R-from+m)/width, *(int*) (R+m) );
					m += width;
				}
				say("Copying %d bytes (%d elements)\n", m, m/width);
				memcpy( to, R, m );
				to += m;
				R += m;
				
				say("remaining left %d, remaining right %d\n", (L_end-L)/width, (R_end-R)/width);
			}
			
			if( L < L_end ) {
				memcpy( to, L, L_end-L );
			}
			if( R < R_end ) {
				memcpy( to, R, R_end-R );
			}

			say("Postmerge [%d - %d] (%p):\n", start, index_end-1, to);
			print_array( (widget*)(to - (index_end-start)*width), 0, (index_end-start),  elements_per_block );
			is_sorted( (widget*)(to - (index_end-start)*width), 0, (index_end-start) );

			// if we do sequential merges, we merge the result of what we just did, with an older one (of equal size)
			// so where we read from and write to swaps
			
			from = from == in ? buf : in; // swap from, which is easy because it is never modified (hmm, maybe we don't need it then?)
			to = from == in ? buf : in; // opposite of from, but we just switched from
		
			say("Pointers now: to=%s (%p), from=%s (%p)\n", to==in?"in":"buf", to, from==in?"in":"buf", from);
		
			// while we have even numbers, we merge blocks up
			mergecounter /= 2;
			merge_width *= 2; // every time we merge twice as much
			// endpoint stays the same
		}
	
		// now reset
		from = in;
		to = buf;

		// process the next block
		index_start += elements_per_block;
	}

	say("\nEvery block sorted (index_start=%d), now doing remaining merges\n", index_start);
	say( "Current in [%d - %d] (%p):\n", 0, nel-1, in );
	print_array( (widget*)in, 0, nel, elements_per_block );
	say( "Current buf [%d - %d] (%p):\n", 0, nel-1, buf );
	print_array( (widget*)buf, 0, nel, elements_per_block );

	// inelegent special case: if elements_per_block > number of items, shellsort did all the
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

	say( "\nDoing wrapup merges for %d blocks\n", blocks_done );

	uint32_t first, second;
	char *left, *right; // could be different
	
	say("in %p buf %p from %p to %p ttt %d\n", in, buf, from, to);
	// if we don't have any postmerging to do this will make sure we output the right array in the final if
	char* to_start = in;
	uint32_t b = blocks_done;
	while( (b = b >> 1) ) {
		to_start = to_start == in ? buf : in;
	} 

	// see explanation below
	if( (ffs( blocks_done ) & 1) == 0 ) {
		right = buf;
	} else {
		right = in;
	}
	say("right starting at %s (%p)\n", right==in?"in":"buf", right);

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
		say( "smallest: %d larger: %d. blocks_done remainder: %d\n", first, second, blocks_done );

		/*
		We have to figure out where left, right and to should point to.
		The first time right points to the last thing we sorted or merged.
		Left always points to a block merged earlier determined by the algorithm below.
		Since we must always write to the opposite of the large part (because that's where the space is)
		to is equal to !left.
		After we've merged we have to set right to where we just wrote (to), figure out left again and set to to the opposite
		
		So where do they point to?
		If we merge 2 blocks they go from in to buf (2^1)
		If we merge 2+2 blocks they go from buf to in (2^2)
		If we merge 4+4 blocks they go from in to buf again etc (2^3)
		so blocks_done = 2^n and if n is odd we wrote it to buf, if n is even we wrote it to in
		*/
		if( (ffs( second ) & 1) == 0 ) {
			left = buf;
			to = to_start = in;
		} else {
			left = in;
			to = to_start = buf;
		}

		say( "smaller=%d (%d blocks) right=%s (%p), ffs(first)=%d\n", first, 1<<(ffs(first)-1), right==in?"in":"buf", right, ffs(first) );
		say( "larger=%d (%d blocks) left=%s (%p) ffs(second)=%d\n", second, 1<<(ffs(second)-1), left==in?"in":"buf", left, ffs(second) );
		say( "to=%s (%p)\n", to==in?"in":"buf", to );
	
		// Merge the first to last block with the second to last one
		// (maybe swap meaning of first and second since we're doing this backwards?)
		// the start offsets are "back from the end by offsets" which we'd have to keep track of over more merges
		// but blocks_done already remove "first" every time which makes it work. think about it :)
		uint32_t start = (blocks_done - second) * elements_per_block;
		L = left + start*width; // start of "left"/bigger array
		to += start*width; // to points to the opposite of left, but has the same offset
		R = right + (blocks_done * elements_per_block)*width; // start of "right" array (first has already been subtracted from blocks_done)

		L_end = left + (blocks_done*elements_per_block-1)*width;
		R_end = right + (nel-1) * width; // in this case it's always the end of the array (but we don't know if it's in or buf)
		
		uint32_t m = 0;
		say( "Merging %d blocks from left (%p) with %d blocks from right (%p)\n", second, L, first, R );
		say( "Merging %d+%d=%d elements: %s[%d - %d] (%p) with %s[%d - %d] (%p)\n", (L_end-L)/width+1,(R_end-R)/width+1, (L_end-L + R_end-R)/width+2, left==in?"in":"buf", (L-left)/width, (L_end-left)/width, left, right==in?"in":"buf", (R-right)/width, (R_end-right)/width, right );
		say( "Premerge left [%d - %d] (%p):\n", (L-left)/width, (L_end-left)/width, left );
		print_array( (widget*)left, (L-left)/width, (L_end-left)/width+1, elements_per_block );
		say( "Premerge right [%d - %d] (%p):\n", (R-right)/width, (R_end-right)/width, right );
		print_array( (widget*)right, (R-right)/width, (R_end-right)/width +1, elements_per_block );

		while( L <= L_end || R <= R_end ) { // until we've copied everything to the target array

			say("\t L=%d\t R=%d\t to=%d\n", (L-left)/width, (R-right)/width, (to-to_start)/width);

			// copy items from left array as long as they are lte
			// (short-circuit evaluation means we never acces list[R] if R is out of bounds)
			// of course R is not modified here, but R might have "ran out" so here we need to copy the rest of L
			m = 0;
			while( (L+m) <= L_end && (R > R_end || compare( L+m, R ) <= 0) ) {

				say("\tto[%d] = L[%d] (%3d)\n", start+(to-to_start+m)/width, (L-left+m)/width, *(int*) (L+m) );
				m += width;
			}
			say("Copying %d bytes (%d elements)\n", m, m/width);
			memcpy( to, L, m );
			to += m;
			L += m;
			
			// copy items from right array as long as they are lte
			m = 0;
			while( (R+m) <= R_end && (L > L_end || compare( R+m, L ) <= 0) ) {

				say("\tto[%d] = R[%d] (%3d)\n", start+(to-to_start+m)/width, (R-right+m)/width, *(int*) (R+m) );
				m += width;
			}
			say("Copying %d bytes (%d elements)\n", m, m/width);
			memcpy( to, R, m );
			to += m;
			R += m;

		}

		say( "Postmerge [%d - %d] (%p):\n", start, (R_end-right)/width, to_start );
		print_array( (widget*)to_start, start, (R_end-right)/width +1, elements_per_block );
		is_sorted( (widget*)to_start, start, (R_end-right)/width +1 );
	
		right = to_start;
	}
	say("Finished wrapup merge\n");

	// unfortunate result of the stdlib sort interface: we might end up with the end result in buf
	// and not in wherever base points to. In that case we copy over everything :(
	say("in %p buf %p to_s %p\n", in, buf, to_start);

	if( base != to_start ) {
		say("Final result is in buf, copying over to base\n");
		// base points to the last thing we merged to, except it was swapped so buf contains the correct stuff
		memcpy( base, buf, nel*width );
	} else {
		// things are fine
	}
	free( buf );
}

void pyramid_mergesort_wrapper( void* base, size_t nel, size_t width, comparator compare ) {
	
	uint32_t elements_per_block = 4;
	const char* env_elements_per_block = getenv( "SORTER_BLOCK_WIDTH" );
	if( env_elements_per_block != NULL ) {
		elements_per_block = atoi( env_elements_per_block );
	}
	say("Using SORTER_BLOCK_WIDTH %d\n", elements_per_block);
	
	pyramid_mergesort( base, nel, width, compare, elements_per_block, shellsort );
	
}

