#!/usr/local/bin/fish

#############################################################################
# Benchmark: Every sort we find in bin/ except insertionsort with a "saw up" distribution
#############################################################################

# command should be called 'source' but hasn't been renamed yet (or my fish shell is old)
. benchmark/global_settings.fsh

set -x GEN_STRUCTS_MEAN 100
set -x GEN_STRUCTS_SD 20

rm -rf results/

make clean
make all
make tools

rm bin/insertionsort
rm bin/inplace_mergesort

set -x NUM_SORTS (count bin/*)

echo "Benchmarking $NUM_SORTS sorts"
set -x NUM_COLUMNS $NUM_SORTS + 1

perl benchmark/benchmark.pl --min=100 --max=$NUM_ELEMENTS --num=100 --iterations=$ITERATIONS --distribution=saw_down
	
perl benchmark/reduce.pl
gnuplot -e "COUNT=$NUM_COLUMNS; OUTPUT_FILE='benchmark/all_sorts_saw_down.svg'; PLOT_TITLE='All sorts, saw down distribution'; PLOT_NUM_ELEMENTS=$NUM_ELEMENTS" benchmark/create_plot.gnuplot

make clean
