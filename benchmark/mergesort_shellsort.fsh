#!/usr/local/bin/fish

#############################################################################
# Benchmark: Mergesort with inner shellsort with different inner sort widths.
#############################################################################

# command should be called 'source' but hasn't been renamed yet (or my fish shell is old)
. benchmark/global_settings.fsh

rm -rf results/

for ISW in 8 16 24 32 40 48 56 64
	
	echo "Setting inner sort width to $ISW"
	set -x INNER_SORT_WIDTH $ISW

	make clean
	make mergesort PAD_SIZE=$PAD_SIZE
	make tools
	
	perl benchmark/benchmark.pl --min=100 --max=2000 --num=5 --iterations=3 --distribution=random
	mv {$RESULTS_DIR}mergesort_results.csv {$RESULTS_DIR}mergesort_{$ISW}_results.csv # results dir is global
	
end

perl benchmark/reduce.pl
