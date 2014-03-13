#include "utils.h"
#include "widget.h"

int write_widgets( widget* widgets, size_t count, const char* filename_out ) {

	FILE* out = fopen( filename_out, "wb" );
	if( out == NULL ) {
		perror("fopen()");
		return -1;
	}
	
	size_t written = fwrite( widgets, sizeof(widget), count, out );
	if( written != count ) {
		perror("frwrite()");
		return -1;
	}	
	
	fclose( out );

	return 0;
}

// as a check to see if the original numbers array contains the same elements
// as the sorted one
void contains_same_elements( widget* a, widget* b, size_t count) {

	int xored_a = 0, xored_b = 0;
	size_t sum_a = 0, sum_b = 0;
	for(size_t i=0; i<count; i++) {
		sum_a += a[i].number;
		sum_b += b[i].number;

		xored_a ^= a[i].number;
		xored_b ^= b[i].number;
	}

	// so in theory the sort function could have a bug that results
	// in different values that sum and xor to the same.

	if( sum_a != sum_b || xored_a != xored_b ) {
		puts( "Arrays don't contain the same numbers" );
		print_array( a, 0, count, 8 );
		print_array( b, 0, count, 8 );
		exit( EXIT_FAILURE );
	}
}


const char* sfw( char* is_a_widget ) {
	
	char* formatted_string;
	
	asprintf( &formatted_string, "%3d:%.1s", ((widget*)is_a_widget)->number, ((widget*)is_a_widget)->padding );
	
	save_allocated_string( formatted_string );
	
	return formatted_string;
}


/*
Compare 2 structs of type widget
	a > b: 1
	a == b: 0
	a < b: -1
*/
int compare_widget(const void* a, const void* b) {

	widget* wa = (widget*)a;
	widget* wb = (widget*)b;	

	if ( wa->number == wb->number )
		return 0;

	if ( wa->number < wb->number )
		return -1;

	return 1;
}

