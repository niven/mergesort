#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include <limits.h>


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
	
	int r, w;
	char sprocket[PAD_SIZE];
	for( size_t i=0; i<count; i++ ) {
		r = rand() % max;
		w = fwrite( &r, sizeof(int), 1, out );
		if( w != 1 ) {
			perror("frwrite()");
			exit( EXIT_FAILURE );
		}
		// now write PAD_SIZE bytes
		for(int i=0; i<PAD_SIZE-1; i++) {
			sprocket[i] = 'A' + rand() % 26;
		}
		sprocket[PAD_SIZE-1] = '\0';
		printf("sprocket: %s\n", sprocket);
		w = fwrite( sprocket, sizeof(char), PAD_SIZE, out );
		if( w != PAD_SIZE ) {
			perror("frwrite()");
			exit( EXIT_FAILURE );
		}
	}
	
	
	fclose( out );

	return EXIT_SUCCESS;
}
