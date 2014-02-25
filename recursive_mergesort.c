#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )


void recursive_mergesort( void* base, size_t nel, size_t width, comparator compare ) {

	if( nel < 2 ) {
		return;
	}

	char* list = (char*)base;
	size_t mid = nel / 2;
	
	// allocate buf etc.
	
	recursive_mergesort( list, mid, width, comparator );
	recursive_mergesort( list + mid*width, nel - mid, width, comparator );

	

}
