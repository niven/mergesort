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
static uint32_t mean = 20;
static uint32_t sd = 10;

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
		series_length_target = mean + (uint32_t)( ziggurat_next() * (double)sd );
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
	
	static uint32_t generator_last = INT_MAX;
	
	generator_last -= (rand() % max) / sqrt(max); // might wraparound, but that is cool
	
	return generator_last % max;
}

/*
Produce random sequence
*/
uint32_t generator_random(uint32_t max) {
	
	return random() % max;
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
