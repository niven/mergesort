#include "stdlib.h"

#include "utils.h"


void sort_function( void* base, size_t nel, size_t width, comparator compare ) {

	heapsort( base, nel, width, compare );

}

size_t working_set_size( size_t element_size, size_t nel ) {
	return element_size * nel;
}
