#!/usr/local/bin/fish

#############################################################################
# Benchmark: Sorts from the C standard library
#############################################################################

# command should be called 'source' but hasn't been renamed yet (or my fish shell is old)
. benchmark/global_settings.fsh

rm -rf results/

make clean
make stdlib_qsort PAD_SIZE=$PAD_SIZE
make stdlib_heapsort PAD_SIZE=$PAD_SIZE
make stdlib_mergesort PAD_SIZE=$PAD_SIZE
make tools
	
perl benchmark/benchmark.pl --min=100 --max=$NUM_ELEMENTS --num=100 --iterations=10 --distribution=random
	
perl benchmark/reduce.pl
gnuplot -e "COUNT=4; OUTPUT_FILE='benchmark/standard_library_sorts.svg'; PLOT_TITLE='Standard Library sorts'; PLOT_NUM_ELEMENTS=$NUM_ELEMENTS" benchmark/create_plot.gnuplot

make clean
