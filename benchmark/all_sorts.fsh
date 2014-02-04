#!/usr/local/bin/fish

#############################################################################
# Benchmark: Every sort we find in bin/ except insertionsort
#############################################################################

# command should be called 'source' but hasn't been renamed yet (or my fish shell is old)
. benchmark/global_settings.fsh

rm -rf results/

make clean
make all PAD_SIZE=$PAD_SIZE
make tools

rm bin/insertionsort

set -x NUM_SORTS (count bin/*)

echo "Benchmarking $NUM_SORTS sorts"
set -x NUM_COLUMNS $NUM_SORTS + 1

perl benchmark/benchmark.pl --min=100 --max=$NUM_ELEMENTS --num=100 --iterations=10 --distribution=random
	
perl benchmark/reduce.pl
gnuplot -e "COUNT=$NUM_COLUMNS; OUTPUT_FILE='benchmark/all_sorts.svg'; PLOT_TITLE='All sorts'; PLOT_NUM_ELEMENTS=$NUM_ELEMENTS" benchmark/create_plot.gnuplot

make clean
