#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "utils.h"
#include "ziggurat.h"

// to produce these sequences we have a common signature
typedef uint32_t (*generator)(uint32_t);

/*
Produce sequence like /|/|/|/|
What we actually need is a series of monotonically increasing values
What we do is set a mean and standard deviation for the length of a series
and then set a min/max increase to get to the end of the series.
What I don't want to do is partition an int into 10 pieces or something:
the mean will probably not be that high which means we would have many
similar numbers.
*/

// shared by saw_up and saw_down
uint32_t mean = 0;
uint32_t sd = 0;

// max defines the maximum value which is helpful for debugging
// since the values will just be smaller
uint32_t generator_saw_up(uint32_t max) {
	
	static uint32_t generator_last = 0;
	static size_t series_length = 0;
	static size_t series_length_target = 0;
	static uint32_t max_increase = 0;
	
	// this will also happen the first time this is called
	if( series_length >= series_length_target ) {
		// set a new length
		generator_last = 0;
		series_length = 0;
		series_length_target = mean + (uint32_t)( abs(ziggurat_next()) * (double)sd );
		say("Starting new series for saw_up with length %zu\n", series_length_target);
	}

	// this evenly partitions the remainder. but since we pick a number within that range
	// the next time this will be different
	max_increase = (max - generator_last) / (series_length_target-series_length);

	uint32_t increase =  (random() % max_increase) + 1;

	say("%d left over %d values max now %d, increase %d\n", max - generator_last,series_length_target-series_length, max_increase, increase );

	generator_last += increase; // make sure we never increase by 0
	
	series_length++;
	
	say("Returning %d\n", generator_last);
	
	return generator_last;
}

/*
Produce sequence like |\|\|\|\
*/
uint32_t generator_saw_down(uint32_t max) {
	
	return max - generator_saw_up( max );

}

/*
Produce sequence like /\/\/\/\/\/\/\/\
*/
uint32_t generator_organ_pipes(uint32_t max) {
	
	return 0;

}


/*
Produce random sequence
*/
uint32_t generator_random(uint32_t max) {
	
	return random() % max;
}

/*
Produce a sequence that is random, but has many similar values.

This is actually no different than randomly generating numbers
which is uniform but due to the range has many duplicates.

The desired result is numbers with 'mean' different numbers in the
range.
*/
uint32_t generator_duplicates(uint32_t max) {
	
	
	// random % mean produces 1 number out of mean possibilites
	// then the ratio max/mean maps that to the actual range.
	// max/min are both ints, so we reorder them to not have int rounding fail
	// and since max could be UINT32_MAX we could overflow so we widen
	return ( (uint64_t)(random() % mean) * (uint64_t)max ) / mean;
}



int main(int argc, char* argv[]) {

	if( argc < 4 ) {
		printf("Usage: gen_random_structs distribution count file [optional max INT_MAX=%d]\n", INT_MAX);
		exit( EXIT_SUCCESS );
	}
	
	ziggurat_init( time(NULL) );
	
	const char* distribution_str = argv[1];
	say("Distribution %s\n", distribution_str);
	generator numbers_gen = NULL;
	if( strcmp( distribution_str, "saw_up" ) == 0 ) {
		numbers_gen = generator_saw_up;
	} else if( strcmp( distribution_str, "saw_down" ) == 0 ) {
		numbers_gen = generator_saw_down;
	} else if( strcmp( distribution_str, "random" ) == 0 ) {
		numbers_gen = generator_random;
	} else if( strcmp( distribution_str, "duplicates" ) == 0 ) {
		numbers_gen = generator_duplicates;
	} else if( strcmp( distribution_str, "organ" ) == 0 ) {
		numbers_gen = generator_organ_pipes;
	} else {
		printf("Don't know distribution type %s\n", distribution_str );
		exit( EXIT_FAILURE );
	}
	
	// read some env things we might need
	const char* env_mean = getenv( "GEN_STRUCTS_MEAN" );
	if( env_mean != NULL ) {
		mean = atoi( env_mean );
	} else {
		printf("GEN_STRUCTS_MEAN not defined in environment.\n");
		exit( EXIT_FAILURE );
	}

	say("Using GEN_STRUCTS_MEAN %d\n", mean);
	const char* env_sd = getenv( "GEN_STRUCTS_SD" );
	if( env_sd != NULL ) {
		sd = atoi( env_sd );
	} else {
		printf("GEN_STRUCTS_SD not defined in environment.\n");
		exit( EXIT_FAILURE );
	}
	say("Using GEN_STRUCTS_SD %d\n", sd);
	

	
	size_t count = atoi( argv[2] );
	const char* filename = argv[3];

	printf("Writing %ld %s structs to %s\n", count, distribution_str, filename);
	
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
