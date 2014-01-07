#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "utils.h"

enum distribution { 
	rnd,
	saw_up, 
	saw_down, 
	plateau, 
	organ_pipes, 
	partial_sorted, 
};

// to produce these sequences we have a common signature
typedef uint32_t (*generator)(int);

/*
Produce sequence like /|/|/|/|
*/
uint32_t generator_saw_up(int max) {
	
	static uint32_t generator_last = 0;

	if( (rand() % 100) < 10 ) {
		generator_last = 0;
	} else {
		generator_last += rand() % max/4; // might wraparound, but that is cool
	}
	
	return generator_last % max;
}

/*
Produce sequence like |\|\|\|\
*/
uint32_t generator_saw_down(int max) {
	
	static uint32_t generator_last = 255;

	if( (rand() % 100) < 10 ) {
		generator_last = max;
	} else {
		generator_last -= rand() % max/4; // might wraparound, but that is cool
	}
	
	return generator_last % max;
}

/*
Produce random sequence
*/
uint32_t generator_random(int max) {
	
	return rand() % max;
}


int main(int argc, char* argv[]) {

	if( argc < 4 ) {
		printf("Usage: gen_random_structs distribution count file [optional max INT_MAX=%d]\n", INT_MAX);
		exit( EXIT_SUCCESS );
	}
	
	const char* distribution_str = argv[1];
	say("Distribution %s\n", distribution_str);
	generator numbers_gen = NULL;
	if( strcmp( distribution_str, "saw_up" ) == 0 ) {
		numbers_gen = generator_saw_up;
	} else if( strcmp( distribution_str, "saw_down" ) == 0 ) {
		numbers_gen = generator_saw_down;
	} else if( strcmp( distribution_str, "random" ) == 0 ) {
		numbers_gen = generator_random;
	} else {
		printf("Don't know distribution type %s\n", distribution_str );
		exit( EXIT_FAILURE );
	}
	
	size_t count = atoi( argv[2] );
	const char* filename = argv[3];

	printf("Writing %ld random structs to %s\n", count, filename);
	
	unsigned int max = INT_MAX;
	if( argc > 4 ) {
		max = atoi( argv[4] );
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
		wid->number = numbers_gen( max );
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
