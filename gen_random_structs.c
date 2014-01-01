#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include <limits.h>

#include "utils.h"

int main(int argc, char* argv[]) {

	if( argc < 3 ) {
		printf("Usage: gen_random_structs count file [optional max]\n");
		exit( EXIT_SUCCESS );
	}
	
	size_t count = atoi( argv[1] );
	const char* filename = argv[2];

	printf("Writing %ld random structs to %s\n", count, filename);
	
	unsigned int max = INT_MAX;
	if( argc > 3 ) {
		max = atoi( argv[3] );
	}
	
	printf("Max: %u\n", max);

	srand( time( NULL ) );

	FILE* out = fopen( filename, "wb" );
	if( out == NULL ) {
		perror("fopen()");
		exit( EXIT_FAILURE );
	}
	
	size_t bytes_written;
	widget* wid = (widget*)malloc( sizeof(widget) );
	if( wid == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}
	for( size_t i=0; i<count; i++ ) {
		wid->number = rand() % max;
		for(int i=0; i<PAD_SIZE-1; i++) {
			wid->padding[i] = 'A' + rand() % 26;
		}
		wid->padding[PAD_SIZE-1] = '\0';
		say("widget: %d:%s (%p)\n", wid->number, wid->padding, wid);
		bytes_written = fwrite( wid, 1, sizeof(widget), out );
		if( bytes_written != sizeof(widget) ) {
			perror("frwrite()");
			free( wid );
			exit( EXIT_FAILURE );
		}
	}
	free( wid );
	
	fclose( out );

	return EXIT_SUCCESS;
}
