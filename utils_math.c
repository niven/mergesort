

/*
      +inf [ x^k ]           x^2    x^3      x^4        x^5
e^x = SUM  [ --- ] = 1 + x + --- + ----- + ------- + --------- + ....
      k=0  [  k! ]           2*1   3*2*1   4*3*2*1   5*4*3*2*1

Which reduces to
		 x     x                     x
1 + x + --- * --- + (last result) * ---  
         1     2                     3 

The output of this compared to exp() often has a difference
of 32, 128, 65536, 1024... which are suspicious numbers.

The most likely culprit it floating point rounding errors,
mostly because the smallest contributions come last
(so we effectively only get the large numbers from the beginning)

The solution would be to start from the "small end", but that would
mean starting with large x^n and n! which limits the max exp we can do
without going to bignums. The other alternative is to store all the numbers,
and than add in reverse, costing a lot of space and being ugly.

After the 4th taylot_exp() I found that the rounding error are basically caused
by the largest term. I think it makes more sense to check how my versions compare
to something not C, so I can see if I'm implementing someting that makes sense
instead of comparing it to builtin exp() which may also have rounding errors.

Also, reading the source should be educational.

Seems to make more sense to approximate exp() with Remez:
http://lolengine.net/blog/2011/12/21/better-function-approximations


*/

const double ETA = 0.00001;

double taylor_exp( double x ) {
	
	if( x == 0.0 ) {
		return 1;
	}
	
	double factor = x / 1.0;
	double result = 1.0 + x;
	double k = 2.0;
	while( factor > ETA ) {
		factor /= k++;
		factor *= x;
		result += factor;
		say("k=%f factor: %f result = %f\n", k, factor, result);
    }
	
	return result;
}

double taylor_exp3( double x ) {
	
	if( x == 0.0 ) {
		return 1;
	}
	size_t count = 8;
	size_t index = 2;
	double* factors = (double*)malloc( count*sizeof(double) );
	if( factors == NULL ) {
		perror("malloc()");
		exit( EXIT_FAILURE );
	}

	factors[0] = 1;
	factors[1] = x;
	
	double factor = x / 1.0;
	double k = 2.0;
	while( factor > ETA ) {
		factor /= k++;
		factor *= x;
		say("Factor %.0f = %f\n", k, factor);
		factors[index++] = factor;
		if( index == count ) {
			count *= 2;
			say("Realloc factors to %zu\n", count);
			double* temp = realloc( factors, count*sizeof(double) );
			if( temp == NULL ) {
				perror("realloc()");
				free( factors );
				exit( EXIT_FAILURE );
			}
			factors = temp;
		}
    }

	qsort( factors, index, sizeof(double), compare_double);

	double result = 0.0;
	for(size_t i=0; i<index; i++){
		result += factors[i];
		say("Factor %zu = %f, result = %f\n", i, factors[i], result);
	}

	free( factors );
	
	return result;
}


double taylor_exp2( double x ) {
	
	if( x == 0.0 ) {
		return 1;
	}
	
	double x_pow = x*x;
	double factorial = 2; // we start result at 1, and add the last term x^/1! at the end. So the factorials start at 2! = 2
	double k = 2; // same story
	while( x_pow/factorial > ETA ) {
		x_pow *= x;
		factorial *= ++k;
		assert( factorial > 0 );
		assert( x_pow > 0 );
		say("%.0f! = %f\n", k, factorial);
    }
	
	say("x_pow = %f, fac = %f k=%f\n", x_pow, factorial, k);

	double result = 1.0; // x^0 / 1
	// now go backwards
	while( k > 1 ) {
		say("Adding %.0f^%.0f/%.0f! = %f/%f = %f (k=%f)\n", x, k, k, x_pow, factorial, x_pow/factorial, k);
		result += x_pow/factorial;
		x_pow /= x;
		factorial /= k--;
		say("Result now %f\n", result);
	}

	say("Adding %.0f^1/1! = %f\n", x, x);
	result += x;
	
	return result;
}

double taylor_exp_scaled( double x ) {
	
	if( x == 0.0 ) {
		return 1;
	}
	

	double rest = 0.0;
	double factor = floor(x / 1.0);
	double factor_rest = x - floor(x/1.0);
	double sum_factors = 0.0;
	double k = 2.0;
	double intermediate;

	while( factor+factor_rest > ETA ) {
	
		intermediate = (factor + factor_rest) / k++;
		intermediate *= x;

		factor = floor(intermediate);
		factor_rest = intermediate - floor(intermediate);
		
		factor += floor(factor_rest);
		factor_rest = factor_rest - floor(factor_rest); 
		
		sum_factors += factor;
		rest += factor_rest;
		say("k=%f factor: %f + %f result = %f rest = %f\n", k, factor, factor_rest, sum_factors, rest);
    }
	
	return 1 + x + sum_factors + rest;
}

double taylor_kahan_exp( double x ) {
	
	if( x == 0.0 ) {
		return 1;
	}
	
	double factor = x / 1.0;
	double result = 1.0 + x;
	double k = 2.0;
	
	double compensation = 0.0;
	double temp;
	
	while( factor > ETA ) {
		factor /= k++;
		factor *= x;
		
		factor -= compensation;
		temp = result + factor;
		compensation = (temp-result) - factor;
		
		result += factor;
		say("k=%f factor: %f result = %f\n", k, factor, result);
    }
	
	return result;
}

