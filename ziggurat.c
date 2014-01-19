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
	return 0;
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
        x_comp[i] = (x[i+1] / x[i]) * (double)INT_MAX;
    }
    x_comp[BLOCK_COUNT-1] = 0;  // Shown for completeness.

}

int32_t ziggurat_next() {

	uint8_t random_box;
	int8_t sign;
	uint32_t u;
	double x_coord;

	// There is a small chance e don't find a number
	// instead of a while(1) or for(;;) a goto makes more sense
generate_sample:	
	random_box = (uint8_t)random();

	sign = (random_box & 0x80) == 0 ? -1 : 1; // high bit determines sign

	random_box &= 0x7f; // keep lower 7 bits

	printf("Random box %d, sign %d\n", random_box, sign);
	
    // Generate uniform random value with range [0,0xffffffff]
	// ehr, so essentially a 32 bit unsigned int ;)
	// turns out RAND_MAX == 0x7fffffff, which is not enough
    u = random(); // so use random()
	
	if( random_box == 0 ) {
		if( u < x_comp[0] ) {
			return sign * (int32_t) (u * uint_to_u * AREA_Div_Y0);
		}
		
		return sign * ziggurat_sample_tail();
	}
	
	if( u < x_comp[random_box] ) {
		return sign * (int32_t) (u * uint_to_u * x[random_box]);
	}
	
	x_coord = u * uint_to_u * x[random_box];
    if( y[random_box-1] + ((y[random_box] - y[random_box-1]) * (double)random()) < gaussian_PDF_denormalized(x_coord) ) {
        return sign * x_coord;
    }	

	goto generate_sample; // try again

}
