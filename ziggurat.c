/* ***************************************************************************
 * This file is part of SharpNEAT - Evolution of Neural Networks.
 * 
 * Copyright 2011 Colin Green (sharpneat@gmail.com)
 *
 * SharpNEAT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SharpNEAT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SharpNEAT.  If not, see <http://www.gnu.org/licenses/>.
 */
 
 /* This is just a port of the C# algorithm as described: http://heliosphan.org/zigguratalgorithm/zigguratalgorithm.html
  *
  */

#include <assert.h>
#include <limits.h>
#include <math.h> /* exp ln sqrt */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ziggurat.h"

#define BLOCK_COUNT 128

// Right hand x coordinate of the base rectangle.
#define R 3.442619855899

// area of each rectangle
#define AREA 9.91256303526217e-3

static const double uint_to_u = 1.0 / (double)INT_MAX;

static double AREA_Div_Y0;

static double x[BLOCK_COUNT+1];
static double y[BLOCK_COUNT];

static uint32_t x_comp[BLOCK_COUNT];

double gaussian_PDF_denormalized( double x ) {
	return exp( -(x*x / 2.0) );
}

double gaussian_PDF_denormalized_inverse( double y ) {
	return sqrt( -2.0 * log(y) );
}

double ziggurat_sample_tail() {

    double x, y;

    do {
        x = -log( (double)rand()/(double)RAND_MAX ) / R;
        y = -log( (double)rand()/(double)RAND_MAX );
    } while( y+y < x*x );

    return R + x;
}

void ziggurat_init( const long rand_seed ) {
	
	srandom( rand_seed );
	
	x[0] = R;
	y[0] = gaussian_PDF_denormalized( R );

	x[1] = R;
	y[1] = y[0] + AREA/x[1];
	
	for(int i=2; i<BLOCK_COUNT; i++) {
		x[i] = gaussian_PDF_denormalized_inverse( y[i-1] );
		y[i] = y[i-1] + AREA/x[i];
	}
	
	x[BLOCK_COUNT] = 0.0;
	
    // Useful precomputed values.
    AREA_Div_Y0 = AREA / y[0];
	
    x_comp[0] = (( R * y[0]) / AREA) * INT_MAX;

    for(int i=1; i<BLOCK_COUNT-1; i++) {
        x_comp[i] = (uint32_t) ((x[i+1] / x[i]) * (double)UINT32_MAX);
    }
    x_comp[BLOCK_COUNT-1] = 0;  // Shown for completeness.

}

// returns a randomly generated, normal distributed number
double ziggurat_next() {

	uint8_t random_box;
	uint32_t u;
	double x_coord, sign;

	// There is a small chance e don't find a number
	// instead of a while(1) or for(;;) a goto makes more sense
generate_sample:	

	random_box = (uint8_t)random();

	sign = (random_box & 0x80) == 0 ? -1.0 : 1.0; // high bit determines sign

	random_box &= 0x7f; // keep lower 7 bits

    // Generate uniform random value with range [0,0xffffffff]
	// ehr, so essentially a 32 bit unsigned int ;)
	// turns out RAND_MAX == 0x7fffffff, which is not enough
    u = random(); // so use random()
	
	if( random_box == 0 ) {
		if( u < x_comp[0] ) {
			return sign * (double)u * uint_to_u * AREA_Div_Y0;
		}
		
		return sign * ziggurat_sample_tail();
	}
	
	if( u < x_comp[random_box] ) {
		return sign * (double)u * uint_to_u * x[random_box];
	}
	
	x_coord = u * uint_to_u * x[random_box];

    if( y[random_box-1] + ((y[random_box] - y[random_box-1]) * ((double)rand()/(double)RAND_MAX)) < gaussian_PDF_denormalized(x_coord) ) {
        return sign * x_coord;
    }	

	goto generate_sample; // try again

}

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
	



