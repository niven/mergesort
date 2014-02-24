#!/usr/local/bin/fish

#############################################################################
# Benchmark: Every mergesort we find in bin/
#############################################################################

# command should be called 'source' but hasn't been renamed yet (or my fish shell is old)
. benchmark/global_settings.fsh

set -x NUM_ELEMENTS 20000

rm -rf results/

make clean
make all PAD_SIZE=$PAD_SIZE
make tools

rm bin/insertionsort
rm bin/shellsort
rm bin/stdlib_heapsort
rm bin/stdlib_qsort
rm bin/inplace_mergesort

set -x NUM_SORTS (count bin/*)

echo "Benchmarking $NUM_SORTS sorts"
set -x NUM_COLUMNS $NUM_SORTS + 1

perl benchmark/benchmark.pl --min=100 --max=$NUM_ELEMENTS --num=100 --iterations=10 --distribution=random
	
perl benchmark/reduce.pl
gnuplot -e "COUNT=$NUM_COLUMNS; OUTPUT_FILE='benchmark/mergesort_showdown.svg'; PLOT_TITLE='All mergesorts'; PLOT_NUM_ELEMENTS=$NUM_ELEMENTS" benchmark/create_plot.gnuplot

make clean
