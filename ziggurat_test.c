#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ziggurat.h"

// main just generates a lot of numbers and calculates a mean/sd for checking
// everything is within reasonable bounds.
int main( int argc, char* argv[] ) {
	
	if( argc < 4 ) {
		printf("Usage: ziggurat sample_count mean sd\n");
		exit( EXIT_FAILURE );
	}
	
	size_t count = atoi( argv[1] );
	int32_t mean = atoi( argv[2] );
	int32_t sd = atoi( argv[3] );
	
	printf("Ziggurat with %zu samples with mean %d and sd %d\n", count, mean, sd );
	
	ziggurat_init( time(NULL) );
	
	// Welford's method for a running sd
	double M = 0.0, S = 0.0, temp;
	double value;
	size_t k = 1;
	for(size_t i=0; i<count; i++) {
		temp = M;
		value = mean + (ziggurat_next() * sd);

		M += (double)(value - temp) / (double)k;
		S += (double)(value - temp) * (double)( value - M );
		k++;
	}
	
	double actual_sd = sqrt( S / (double)(k-1) );
	double actual_mean = M;
	
	printf("\nActual sd: %f, actual mean %f\n", actual_sd, actual_mean);
	
	exit( EXIT_SUCCESS );
}
