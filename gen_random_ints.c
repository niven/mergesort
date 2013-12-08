#include "stdlib.h"
#include "stdio.h"
#include "time.h"

int main(int argc, char* argv[]) {

	if( argc < 3 ) {
		printf("Usage: gen_random_ints count file [optional seed]\n");
	}
	
	size_t count = atoi( argv[1] );
	const char* filename = argv[2];

	printf("Writing %ld random ints to %s\n", count, filename);

	unsigned int seed = 0;
	if( argc > 3 ) {
		seed = atoi( argv[3] );
	} else {
		time_t epoch = time( NULL );
		seed = epoch;
	}
	
	printf("Seed: %d\n", seed);
	srand( seed );

	FILE* out = fopen( filename, "wb" );
	if( out == NULL ) {
		perror("fopen()");
		exit( EXIT_FAILURE );
	}
	
	int r, w;
	for( size_t i=0; i<count; i++ ) {
		r = rand();
		w = fwrite( &r, sizeof(int), 1, out );
		if( w != 1 ) {
			perror("frwrite()");
			exit( EXIT_FAILURE );
		}
	}
	
	
	fclose( out );

	return EXIT_SUCCESS;
}