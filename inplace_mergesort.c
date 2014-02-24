#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

void print_slices(const char* msg, widget* list, size_t al, size_t ah, ssize_t ml, ssize_t mh, size_t bl, size_t bh) {

   char* out1 = malloc( 1 ); *out1 = '\0';
   char* out2 = malloc( 1 ); *out2 = '\0';
   char* out3 = malloc( 1 ); *out3 = '\0';
 	 
	size_t i;
	    
    // do A
    if( al < ah ) {
		 out1 = append_str( out1, "%3d ", (list + al)->number );
		 out2 = append_str( out2, " ^^ " );
		 out3 = append_str( out3, " al " );
    } else if( al==ah ) {
		 out1 = append_str( out1, "%3d ", (list + al)->number );
		 out2 = append_str( out2, "^^^^" );
		 out3 = append_str( out3, "al+h" );
    } else {
        // don't show any a
    }
   
    for( i=al+1; i<ah; i++ ) {
        out1 = append_str( out1, "%3d ", (list + i)->number );
        out2 = append_str( out2, "    " );
        out3 = append_str( out3, "    " );
    }
    
    if( al < ah ) {
        out1 = append_str( out1, "%3d ", (list + ah)->number );
        out2 = append_str( out2, " ^^ " );
        out3 = append_str( out3, " ah " );
    }

    // maybe mid
    if( ml < mh ) {
        out1 = append_str( out1, " ( %02d ", (list + ml )->number );
        out2 = append_str( out2, "   ^^ " );
        out3 = append_str( out3, "   ml " );
        for( i=ml+1; i<mh; i++ ) {
            out1 = append_str( out1, "%3d ", (list + i)->number );
            out2 = append_str( out2, "    " );
            out3 = append_str( out3, "    " );
            
        }
        out1 = append_str( out1, "%3d  ) ", (list + mh)->number );
        out2 = append_str( out2, "^^^    " );
        out3 = append_str( out3, "mh     " );
    }


    // do B
    if( bl < bh ) {
        out1 = append_str( out1, "%3d ", (list + bl)->number );
        out2 = append_str( out2, " ^^ " );
        out3 = append_str( out3, " bl " );
    } else if( bl == bh ) {
        out1 = append_str( out1, "%3d ", (list + bl)->number );
        out2 = append_str( out2, " ^^ " );
        out3 = append_str( out3, "bl+h" );
    }
        
    for( i=bl+1; i<bh; i++ ) {
        out1 = append_str( out1, "%3d ", (list + i)->number );
        out2 = append_str( out2, "    " );
        out3 = append_str( out3, "    " );
    }
    
    if( bl < bh ) {
        out1 = append_str( out1, "%3d ", (list + bh)->number );
        out2 = append_str( out2, " ^^ " );
        out3 = append_str( out3, " bh " );
    }

	 printf("\n%s - a: [%zu,%zu], m: [%zd,%zd], b: [%zu,%zu]\n", msg, al, ah, ml, mh, bl, bh );
	 puts(out1);
	 puts(out2);
	 puts(out3);

	 free( out1 );
	 free( out2 );
	 free( out3 );

}

void merge_in_place( void* base, size_t start, size_t end, size_t midpoint, size_t width, comparator compare ) {

	char* list = (char*)base;
	
	size_t al = start, ah = midpoint - 1, bl = midpoint, bh = end; 
	
	say("Merging [%zu,%zu] - [%zu,%zu]\n", al, ah, bl, bh);
	
	// initially empty
	// TODO: this would be nicer has a m_start + m_offset so both can be size_t's
	ssize_t ml = 0, mh = -1;
	
   print_slices( "Current", (widget*)base, al, ah, ml, mh, bl, bh);

   int done = 0;
   int sort_mid = 0;

	/*
	The way this works is having 3 conceptual subarrays: A, M and B.
	M is initally empty, so the list is AB.
	Then we move off elements from the start of A and the end of B until we can't do that anymore.
	Then we swap the first and last of A and of B, creating M (which is then either sorted or reverse sorted)
	Then we can do a 4-way-swap witht the A[0], M[0], M[end], B[end] so that we can take elements off
	the start of A and the end of B again.
	The only remaining thing is to resort M to maintain the invariant. 
	*/
	char* temp = malloc( width );
	if( temp == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	while( !done ) {

       // 0. if |a|==0 or |b|==0, reset (a takes m or b takes m)
       //      (or a and b are empty -> done (since m is also sorted)
       // 1. shift a
       // 2. shift b
       // 3. if max is mh or min is ml
       //      swap out, resort ml/mh
       // 4. (now ah is max and bl is min)
       //      swap4
       //      if ml>mh, swap them
       //      |m| > 2 ? -> move ml to correct spot, move mh to correct spot
       
       if( al>ah ) {
           // mid also empty?
           if( ml>mh ) {
               print_slices("a and m empty, thus done", (widget*)base, al, ah, ml, mh, bl, bh);
               break; // we're done (whatever is left in b is sorted)
           } else {
               // mid is now a
               al = ml;
               ah = mh;
               // set mid to empty
               ml = 0;
               mh = -1;
               print_slices("a was empty, mid now a", (widget*)base, al, ah, ml, mh, bl, bh);
           }
       } else 
       
       if( bl>bh ) {
           if( ml>mh ) {
               print_slices("b and m empty, thus done", (widget*)base, al, ah, ml, mh, bl, bh);
               break; // we're done (whatever is left in a is sorted)
           } else {
               // mid is now b
               bl = ml;
               bh = mh;
               // set mid to empty
               ml = 0;
               mh = -1;
               print_slices("b was empty, mid now b", (widget*)base, al, ah, ml, mh, bl, bh);
           }
       } else
       
       // as long as al points to the lowest item, it is sorted
       // a lower than b or if we have mid also lower than mid
       if( compare( (list+width*al), (list+width*bl) ) < 1 && ( ml>mh || (ml<=mh && compare( (list+width*al), (list+width*ml) ) < 1 ) ) ) {
           al++;
           print_slices("Shifted a", (widget*)base, al, ah, ml, mh, bl, bh);
       } else
       
       // as long as bl points to the highest item, it is sorted
       if( compare( (list+width*ah), (list+width*bh) ) < 1  && ( ml>mh || (ml<=mh && compare( (list+width*mh), (list+width*bh) ) < 1 ) ) ) {
           bh--;
           print_slices("Shifted b", (widget*)base, al, ah, ml, mh, bl, bh);
       } else
       
		// try to get rid of mid (step 3)
		if( ml<mh && compare( (list+width*ml), (list+width*al) ) < 0 && compare( (list+width*ml), (list+width*bl) ) < 1 ) {
			// we just swap it out with a
			memcpy( temp, list + width*al, width );
			memcpy( list + width*al, list + width*ml, width );
			memcpy( list + width*ml, temp, width );

			sort_mid = 1; // might have to resort mid
			print_slices("Swapped mid with a", (widget*)base, al, ah, ml, mh, bl, bh);
       } else
       if( ml<mh && compare( (list+width*ah), (list+width*bh) ) < 0 && compare( (list+width*bh), (list+width*mh) ) < 1 ) {
			// we just swap it out with b
			memcpy( temp, list + width*bh, width );
			memcpy( list + width*bh, list + width*mh, width );
			memcpy( list + width*mh, temp, width );

			sort_mid = 1; // might have to resort mid
			print_slices("Swapped mid with b", (widget*)base, al, ah, ml, mh, bl, bh);
       } else
               
       // now bl points to min AND ah points to max
       {
			// only 1 left in a and b
			if( bh-bl == 0 && ah-al == 0 ) {
					memcpy( temp, list + width*al, width );
					memcpy( list + width*al, list + width*bh, width );
					memcpy( list + width*bh, temp, width );
					print_slices("Swap - single", (widget*)base, al, ah, ml, mh, bl, bh);
			} else if( bh-bl == 0 ) {
				
					memcpy( temp, list + width*bl, width ); // save bl
					memcpy( list + width*bl, list + width*al, width ); // copy al->bl
					memcpy( list + width*al, list + width*ah, width ); // copy ah->al
					memcpy( list + width*ah, temp, width ); // copy temp->ah

					ml = ah;
					mh = bl;
					bl++;
					ah--;
					sort_mid = 1;
					print_slices("Swap - Rot3 b", (widget*)base, al, ah, ml, mh, bl, bh);
           } else if( ah-al == 0 ) {

					memcpy( temp, list + width*bh, width ); // save bh
					memcpy( list + width*bh, list + width*al, width ); // copy al->bh
					memcpy( list + width*al, list + width*bl, width ); // copy bl->al
					memcpy( list + width*bl, temp, width ); // copy temp(bh)->bl

					ml = ah;
					mh = bl;
					bl++;
					ah--;
					sort_mid = 1;
					print_slices("Swap - Rot3 a", (widget*)base, al, ah, ml, mh, bl, bh);
           } else {
				  /* Example:
					146 237 103 212
					 ^^  ^^  ^^  ^^
					 al  ah  bl  bh
				  
				  	bl is lowest, must end up in al (103)
				  	ah is highest, must end up in bh (237)
				   ah,bl becomes ml,mh so we'd like that to be (146,212) (might be not right since al could be > bh, but we trigger sort_mid anyway)

					103 237 103 212 		(1) bl->al
					 ^^  ^^  ^^  ^^
					 al  ah  bl  bh

					103 237 212 212 		(2) bh->bl
					 ^^  ^^  ^^  ^^
					 al  ah  bl  bh
				  
					103 237 212 237 		(3) ah->bh
					 ^^  ^^  ^^  ^^
					 al  ah  bl  bh
				  
					103 146 212 237 		(4) temp(al)->ah
					 ^^  ^^  ^^  ^^
					 al  ah  bl  bh
				  
				  
				  */
					memcpy( temp, list + width*al, width ); // save al
					memcpy( list + width*al, list + width*bl, width ); // copy bl->al since bl must be the lowest
					memcpy( list + width*bl, list + width*bh, width ); // copy bh->bl since bh is high (but noy highest) and becomes mh)
					memcpy( list + width*bh, list + width*ah, width ); // copy ah->bh since ah is highest
					memcpy( list + width*ah, temp, width ); // copy temp(al)->ah since ah becomes ml

					ml = ah;
					mh = bl;
					ah--;
					bl++;
					sort_mid = 1;
					print_slices("Swap4", (widget*)base, al, ah, ml, mh, bl, bh);
           }
       }                

       if( sort_mid ){
           if( mh-ml == 1 && compare( (list+width*ml), (list+width*mh) ) > 0 ) {
					memcpy( temp, list + width*ml, width );
					memcpy( list + width*ml, list + width*mh, width );
					memcpy( list + width*mh, temp, width );
					print_slices("SortMid 2", (widget*)base, al, ah, ml, mh, bl, bh);
           } else if( mh-ml > 1 ) {
               // first swap these, could be a nice speedup
               if( compare( (list + width*ml), (list + width*mh) ) > 0 ) {
						memcpy( temp, list + width*ml, width );
						memcpy( list + width*ml, list + width*mh, width );
						memcpy( list + width*mh, temp, width );
						print_slices("SortMid - swapped L/H", (widget*)base, al, ah, ml, mh, bl, bh);
               }
               // now swap up the low one
               size_t cur = ml;
               while( cur<mh && compare( (list + width*cur + width), (list + width*cur) ) < 1 ) {
						memcpy( temp, list + width*cur + width, width );
						memcpy( list + width*cur + width, list + width*cur, width );
						memcpy( list + width*cur, temp, width );
						cur++;
						print_slices("SortMid - swapping L up", (widget*)base, al, ah, ml, mh, bl, bh);
               }
               // now swap down the high one
               cur = mh;
               while( cur>ml && compare( (list + width*cur), (list + width*cur - width) ) < 1 ){
						memcpy( temp, list + width*cur - width, width );
						memcpy( list + width*cur - width, list + width*cur, width );
						memcpy( list + width*cur, temp, width );
						cur--;
						print_slices("SortMid - swapping H down", (widget*)base, al, ah, ml, mh, bl, bh);
               }
               // so yes, we did (potentially a lot of) sorting here, but a max of 2 items, so max ~2n swaps
           }
       }
       
       done = al==ah && ml==mh && bl==bh; // all "empty"
	 }
	
}


/*
Non-recursive mergesort without inner sort that merges in place.
*/
void inplace_mergesort(void* base, size_t nel, size_t width, comparator compare) {

	if( nel <= 1 ) {
		return;
	}

//--- First: divide the array in pairs and sort those

	char* list = (char*)base;
// (instead of pairs you could pick a larger number and use a different sorting algorithm, but we're doing that elsewhere)

// for a list with an odd number of elements we don't need to sort the remaining single element.
	size_t pairs = nel / 2; // (implicit floor)
	char* temp = malloc( width );
	if( temp == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	char* first;
	for( size_t i=0; i<2*pairs; i+=2 ) {
		first = list + i*width; // first element of the pair
		if( compare( first, first + width ) == 1 ) { // if the first is bigger than the second
			memcpy( temp, first + width, width ); // save second
			memcpy( first + width, first, width ); // overwrite second with first
			memcpy( first, temp, width ); // copy second to first	
		}
	}
	free( temp );
	say("Merge step 1 done, all pairs now sorted\n");
	print_array( (widget*)base, 0, nel, 16 );

//--- Second: perform the merge

	size_t merge_width = 2; // start with the pairs that are sorted

	while( merge_width < nel ) {
	   
		say("Merge width %zu\n", merge_width);
		
		// merge k pairs of size mergeLength
		for( size_t start=0; start<nel; start += 2*merge_width ) {
			// use indices for the Left of the pair and the Right of the pair
			size_t L = start;
			size_t L_end = MIN(start + merge_width - 1, nel - 1);
			size_t R = MIN(L_end + 1, nel - 1);
			size_t R_end = MIN(R + merge_width - 1, nel - 1);

         // if we're merging chunks of size 8, but we're at the end of the array and have like 5 elements left
         // it means we're done :)
         if( L_end >= R ) {
             continue;
         }

    	   // now merge in place, and since we're slicing we need to offset the midpoint
         merge_in_place( base, L, R_end, R, width, compare );
		}
		
		merge_width *= 2;
	}


}
