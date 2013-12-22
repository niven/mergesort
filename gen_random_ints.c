#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include <limits.h>

int main(int argc, char* argv[]) {

	if( argc < 3 ) {
		printf("Usage: gen_random_ints count file [optional seed or -] [optional max]\n");
		exit( EXIT_SUCCESS );
	}
	
	size_t count = atoi( argv[1] );
	const char* filename = argv[2];

	printf("Writing %ld random ints to %s\n", count, filename);

	unsigned int seed = 0;
	if( argc > 3 && argv[3][0] != '-' ) {
		seed = atoi( argv[3] );
	} else {
		time_t epoch = time( NULL );
		seed = epoch;
	}
	
	unsigned int max = INT_MAX;
	if( argc > 4 ) {
		max = atoi( argv[4] );
	}
	
	printf("Seed: %d, max: %u\n", seed, max);
	srand( seed );

	FILE* out = fopen( filename, "wb" );
	if( out == NULL ) {
		perror("fopen()");
		exit( EXIT_FAILURE );
	}
	
	int r, w;
	for( size_t i=0; i<count; i++ ) {
		r = rand() % max;
		w = fwrite( &r, sizeof(int), 1, out );
		if( w != 1 ) {
			perror("frwrite()");
			exit( EXIT_FAILURE );
		}
	}
	
	
	fclose( out );

	return EXIT_SUCCESS;
}
