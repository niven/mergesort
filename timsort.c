
#include "utils.h"

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

void timsort(void* base, size_t nel, size_t width, comparator compare) {
	

}


void sort_function( void* base, size_t nel, size_t width, comparator compare ) {
	
	timsort( base, nel, width, compare );
	
}

size_t working_set_size( size_t element_size, size_t nel ) {
	return 2 * element_size * nel;
}

