#include <stdlib.h>

#include "utils.h"

void sort_function( void* base, size_t nel, size_t width, comparator compare ) {

	mergesort( base, nel, width, compare );

}
