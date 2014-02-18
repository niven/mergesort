package sorting

import "fmt"
import "math"

type Sortable interface {
    Len() int
    Less(i, j int) bool
    LessEqual(i, j int) bool
    Swap(i, j int)
    Copy(dest interface{}, destIndex, sourceIndex int)
}

// alias a type to slice of ints so we can attach methods to it
type IntSlice []int

func (is IntSlice) Len() int {
    return len(is)
}

func (is IntSlice) Less(i, j int) bool {
    return is[i] < is[j]
}

func (is IntSlice) LessEqual(i, j int) bool {
    return is[i] <= is[j]
}

var swaps = 0
func (is IntSlice) Swap(i, j int) {
    swaps++
    is[i], is[j] = is[j], is[i]
}

func (is IntSlice) Copy(dest interface{}, destIndex, sourceIndex int) {
    dest.(IntSlice) [destIndex] = is[sourceIndex]
}

func MergeInts(a []int) {
    buf := IntSlice( make( []int, len(a) ) )
    mergesort( IntSlice(a), buf )
}

func MergeIntsInPlace(a []int) {
    mergesort_inplace( IntSlice(a) )
}

// Normal mergesort (uses O(n) memory, O(n log_2 n) time and is stable)
func mergesort(n Sortable, buf Sortable) {
    
    length := n.Len()
   
    if length <= 1 {
		return
	}

//--- First: divide the array in pairs and sort those

// (instead of pairs you could pick a larger number and use a different sorting algorithm)

// for a list with an odd number of elements we don't need to sort the remaining single element.
	pairs := length / 2 // (implicit floor)
	for i:=0; i<2*pairs; i+=2 {
		if n.Less( i+1, i ) {
			n.Swap( i, i+1 )
		}
	}

//--- Second: perform the merge

	mergeLength := 2; // start with the pairs that are sorted
    var L, R, L_end, R_end, to int
	for mergeLength < n.Len() {
	   

	   
		// merge k pairs of Len mergeLength
		for start:=0; start<length; start += 2*mergeLength {
			// use indices for the Left of the pair and the Right of the pair
			L = start
			R = start + mergeLength
			L_end = min(R - 1, length - 1)
			R_end = min(R+mergeLength-1, length - 1)
			to = 0

			
			for start+to <= R_end { // we always know how many elements to merge
				// copy as many from the left pair as we can
				// (short-circuit evaluation means we never acces n[R] if R is out of bounds)
				for L <= L_end && (R > R_end || n.LessEqual( L, R ) ) {
					n.Copy(buf, start + to, L)
					to++
					L++
				}

				// then copy as many as we can from the right pair
				for R <= R_end && (L > L_end || n.Less( R, L ) ) {
					n.Copy( buf, start + to, R)
					to++
					R++
				}

			}

			
		}
		
		
		// reset the source array for merging
		n, buf = buf, n

		mergeLength *= 2;
	}

}


// mergesort a slice of ints
func mergesort_inplace( n Sortable ) {

    swaps = 0
    length := n.Len()

    if debug {
        fmt.Printf("\n\n***************************\nMerging %v\n***************************\n\n", n )
    }
    
	if length <= 1 {
		return
	}

//--- First: divide the array in pairs and sort those

// (instead of pairs you could pick a larger number and use a different sorting algorithm)

// for a list with an odd number of elements we don't need to sort the remaining single element.
	pairs := length / 2 // (implicit floor)
	for i:=0; i<2*pairs; i+=2 {
		if n.Less(i+1, i) {
            n.Swap( i, i+1 )
		}
	}
	
//--- Second: perform the merge

	mergeLength := 2; // start with the pairs that are sorted

	for mergeLength < length {
	   
        if debug {
	       fmt.Printf("Merging size %v: %v\n", mergeLength, n)
        }	   
		// merge k pairs of size mergeLength
		for start:=0; start<length; start += 2*mergeLength {
			// use indices for the Left of the pair and the Right of the pair
			L := start
			L_end := min(start + mergeLength - 1, length - 1)
			R := min(L_end + 1, length - 1)
			R_end := min(R + mergeLength - 1, length - 1)

            // if we're merging chunks of size 8, but we're at the end of the array and have like 5 elements left
            // it means we're done :)
            if L_end >= R {
                continue
            }

            if debug {
    	      fmt.Printf("Merging subarray (%v,%v)-(%v,%v) = %v\n", L, L_end,R, R_end )
            }
    	   // now merge in place, and since we're slicing we need to offset the midpoint
            merge_slice_in_place( n, L, R_end, R )
		}
		
		mergeLength *= 2;
	}

    logn := int(math.Log(float64(length)))
    fmt.Println( "\nN, swaps, ln N ", length, swaps, logn, float64(swaps)/float64(length*length)) 
	return
}

func merge_slice_in_place( n Sortable, start, end, midpoint int ) {


    var al, ah, bl, bh, ml, mh int
	al = start
	ah = midpoint-1
	bl = midpoint
	bh = end
	
	if debug {
    	fmt.Printf("Merging (%v,%v)-(%v,%v)", al, ah, bl, bh)
	}
	
	// initially empty
	ml = 0
	mh = -1
	
    
    current2("Initial", n.(IntSlice), al, ah, ml, mh, bl, bh)
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
    
}


func current2(msg string, numbers IntSlice, al, ah, ml, mh, bl, bh int) {

    if !debug {
        return
    }
    out1 := ""
    out2 := ""
    out3 := ""
    
    var i int
    
    // all the done things on the left
    for i=0; i<al; i++ {
        out1 += "-- "
        out2 += "   "
        out3 += "   "
    }      
    
    // do A
    if al < ah {
        out1 += fmt.Sprintf("%02d ", numbers[al] )
        out2 += "^^ "
        out3 += "al "
    } else if al==ah {
        out1 += fmt.Sprintf("%02d ", numbers[al] )
        out2 += "^^ "
        out3 += "alh"
    } else {
        // don't show any a
    }
    
    for i=al+1; i<ah; i++ {
        out1 += fmt.Sprintf("%02d ", numbers[i] )
        out2 += "   "
        out3 += "   "
    }
    
    if al < ah {
        out1 += fmt.Sprintf("%02d ", numbers[ah] )
        out2 += "^^ "
        out3 += "ah "
    }

    // maybe mid
    if ml < mh {
        out1 += fmt.Sprintf(" ( %02d ", numbers[ml] )
        out2 += "   ^^ "
        out3 += "   ml "
        for i=ml+1; i<mh; i++ {
            out1 += fmt.Sprintf("%02d ", numbers[i] )
            out2 += "   "
            out3 += "   "
            
        }
        out1 += fmt.Sprintf("%02d  ) ", numbers[mh] )
        out2 += "^^    "
        out3 += "mh    "
    }

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

fmt.Printf( "\n%v - a: [%v,%v], m: [%v,%v], b: [%v,%v]\n", msg, al, ah, ml, mh, bl, bh )
fmt.Println(out1)
fmt.Println(out2)
fmt.Println(out3)

}
