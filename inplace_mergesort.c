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

	// all the done things on the left
	for( i=0; i<al; i++ ) {
		out1 = append_str( out1, "--- " );
		out2 = append_str( out2, "    " );
		out3 = append_str( out3, "    " );
	}      
    
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

	 /*

    // do B
    if bl < bh {
        out1 += fmt.Sprintf("%02d ", numbers[bl] )
        out2 += "^^ "
        out3 += "bl "
    } else if bl == bh {
        out1 += fmt.Sprintf("%02d ", numbers[bl] )
        out2 += "^^ "
        out3 += "blh"
    }
        
    for i=bl+1; i<bh; i++ {
        out1 += fmt.Sprintf("%02d ", numbers[i] )
        out2 += "   "
        out3 += "   "
    }
    
    if bl < bh {
        out1 += fmt.Sprintf("%02d ", numbers[bh] )
        out2 += "^^ "
        out3 += "bh "
    }
        
    // done things on the right
    for i=bh; i<len(numbers)-1; i++ {
        out1 += "-- "
        out2 += "   "
        out3 += "   "
    } 
*/
	 printf("\n%s - a: [%zu,%zu], m: [%zd,%zd], b: [%zu,%zu]\n", msg, al, ah, ml, mh, bl, bh );
	 puts(out1);
	 puts(out2);
	 puts(out3);

	 free( out1 );
	 free( out2 );
	 free( out3 );

}

void merge_in_place( void* base, size_t start, size_t end, size_t midpoint, size_t width, comparator compare ) {

	size_t al = start, ah = midpoint - 1, bl = midpoint, bh = end; 
	
	say("Merging [%zu,%zu] - [%zu,%zu]\n", al, ah, bl, bh);
	
	// initially empty
	ssize_t ml = 0, mh = -1;
	
   print_slices( "Current", (widget*)base, al, ah, ml, mh, bl, bh);
/*	
   done := false
   sortMid := false
for !done {
   
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
       
       if al>ah {
           // mid also empty?
           if ml>mh {
               current2("a and m empty, thus done", n.(IntSlice), al, ah, ml, mh, bl, bh)
               break // we're done (whatever is left in b is sorted)
           } else {
               // mid is now a
               al = ml
               ah = mh
               // set mid to empty
               ml = 0
               mh = -1
               current2("a was empty, mid now a", n.(IntSlice), al, ah, ml, mh, bl, bh)
           }
       } else 
       
       if bl>bh {
           if ml>mh {
               current2("b and m empty, thus done", n.(IntSlice), al, ah, ml, mh, bl, bh)
               break // we're done (whatever is left in a is sorted)
           } else {
               // mid is now b
               bl = ml
               bh = mh
               // set mid to empty
               ml = 0
               mh = -1
               current2("b was empty, mid now b", n.(IntSlice), al, ah, ml, mh, bl, bh)
           }
       } else
       
       // as long as al points to the lowest item, it is sorted
       // a lower than b or if we have mid also lower than mid
       if n.LessEqual( al, bl ) && ( ml>mh || (ml<=mh && n.LessEqual( al, ml ) ) ) {
           al++
           current2("Shifted a", n.(IntSlice), al, ah, ml, mh, bl, bh)
       } else
       
       // as long as bl points to the highest item, it is sorted
       if n.LessEqual( ah, bh ) && ( ml>mh || (ml<=mh && n.LessEqual( mh, bh ) ) ) {
           bh--
           current2("Shifted b", n.(IntSlice), al, ah, ml, mh, bl, bh)
       } else
       
       // try to get rid of mid (step 3)
       if ml<mh && n.Less( ml, al) && n.LessEqual( ml, bl ) {
           // we just swap it out with a
           n.Swap( al, ml )
           sortMid = true // might have to resort mid
           current2("Swapped mid with a", n.(IntSlice), al, ah, ml, mh, bl, bh)
       } else
       if ml<mh && n.Less(ah, mh) && n.LessEqual( bh, mh ){
           // we just swap it out with b
           n.Swap( bh, mh )
           sortMid = true // might have to resort mid
           current2("Swapped mid with b", n.(IntSlice), al, ah, ml, mh, bl, bh)
       } else
               
       // now bl points to min AND ah points to max
       {
           // only 1 left in a and b
           if bh-bl == 0 && ah-al == 0 {
               n.Swap( al, bh )
               current2("Swap - single", n.(IntSlice), al, ah, ml, mh, bl, bh)
           } else if bh-bl == 0 {
               n.Swap( al, ah )
               n.Swap( ah, bl )
               ml = ah
               mh = bl
               bl++
               ah--
               sortMid = true
               current2("Swap - Rot3 b", n.(IntSlice), al, ah, ml, mh, bl, bh)
           } else if ah-al == 0 {
               n.Swap( al, bl )
               n.Swap( bl, bh )
               ml = ah
               mh = bl
               bl++
               ah--
               sortMid = true
               current2("Swap - Rot3 a", n.(IntSlice), al, ah, ml, mh, bl, bh)
           } else {
               n.Swap( ah, bl )
               current2("Swap4a", n.(IntSlice), al, ah, ml, mh, bl, bh)
               n.Swap( al, ah )
               current2("Swap4b", n.(IntSlice), al, ah, ml, mh, bl, bh)
               n.Swap( bl, bh )
               current2("Swap4c", n.(IntSlice), al, ah, ml, mh, bl, bh)
               ml = ah
               mh = bl
               ah--
               bl++
               sortMid = true
               current2("Swap4", n.(IntSlice), al, ah, ml, mh, bl, bh)
           }
       }                

       if sortMid {
           if mh-ml == 1 && n.LessEqual( mh, ml ) {
               n.Swap( ml, mh )
               current2("SortMid 2", n.(IntSlice), al, ah, ml, mh, bl, bh)
           } else if mh-ml > 1 {
               // first swap these, could be a nice speedup
               if n.LessEqual( mh, ml ) {
                   n.Swap( ml, mh )
                   current2("SortMid - swapped L/H", n.(IntSlice), al, ah, ml, mh, bl, bh)
               }
               // now swap up the low one
               cur := ml
               for cur<mh && n.LessEqual( cur+1, cur ) {
                   n.Swap( cur+1, cur )
                   cur++
                   current2("SortMid - swapping L up", n.(IntSlice), al, ah, ml, mh, bl, bh)
               }
               // now swap down the high one
               cur = mh
               for cur>ml && n.LessEqual( cur, cur-1) {
                   n.Swap( cur-1, cur )
                   cur--
                   current2("SortMid - swapping H down", n.(IntSlice), al, ah, ml, mh, bl, bh)
               }
               // so yes, we did (potentially a lot of) sorting here, but a max of 2 items, so max ~2n swaps
           }
       }
       
       done = al==ah && ml==mh && bl==bh // all "empty"

}

   if debug {
       fmt.Println("Done", n, debug)
   }
	
	*/
	
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
