#!/usr/local/bin/fish

#############################################################################
# Benchmark: Mergesort with inner shellsort with different inner sort widths.
#############################################################################

# command should be called 'source' but hasn't been renamed yet (or my fish shell is old)
. benchmark/global_settings.fsh

rm -rf results/

set -x WIDTHS 2 8 16 32 64 128 256
set -x NUM_WIDTHS (count $WIDTHS +1) # count is actually lastindex I think

make clean
make mergesort
make tools

for ISW in $WIDTHS
	
	echo "Setting inner sort width to $ISW"
	set -x SORTER_BLOCK_WIDTH $ISW
	
	perl benchmark/benchmark.pl --min=100 --max=$NUM_ELEMENTS --num=50 --iterations=$ITERATIONS --distribution=random
	mv {$RESULTS_DIR}mergesort_results.csv {$RESULTS_DIR}mergesort_{$ISW}_results.csv # results dir is global
	
end

perl benchmark/reduce.pl
gnuplot -e "COUNT=$NUM_WIDTHS; OUTPUT_FILE='benchmark/mergesort_shellsort.svg'; PLOT_TITLE='Mergesort with inner shellsort'; PLOT_NUM_ELEMENTS=$NUM_ELEMENTS" benchmark/create_plot.gnuplot

make clean
