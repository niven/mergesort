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

#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

#include "ziggurat.h"

#define BLOCK_COUNT 128

// Right hand x coordinate of the base rectangle.
#define R 3.442619855899

// area of each rectangle
#define AREA 9.91256303526217e-3

static const double uint_to_u = 1.0 / (double)INT_MAX;

static double x[BLOCK_COUNT+1];
static double y[BLOCK_COUNT];

static uint32_t x_comp[BLOCK_COUNT];

uint32_t gaussian_PDF_denormalized( double x_coord ) {
	return 1;
}

double gaussian_PDF_denormalized_inverse( double y_coord ) {
	return 1.0;
}

void ziggurat_init( const long rand_seed ) {
	
	srand( rand_seed );
	
	x[0] = R;
	y[0] = gaussian_PDF_denormalized( R );

	x[1] = R;
	y[1] = y[0] + AREA/x[1];
	
	for(int i=2; i<BLOCK_COUNT; i++) {
		x[i] = gaussian_PDF_denormalized_inverse( y[i-1] );
		y[i] = y[i-1] + AREA/x[i];
	}
	
	x[BLOCK_COUNT] = 0.0;
	
    x_comp[0] = (( R * y[0]) / AREA) * INT_MAX;

    for(int i=1; i<BLOCK_COUNT-1; i++) {
        x_comp[i] = (x[i+1] / x[i]) * (double)INT_MAX;
    }
    x_comp[BLOCK_COUNT-1] = 0;  // Shown for completeness.
	
	
}

