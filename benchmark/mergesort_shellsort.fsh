#!/usr/local/bin/fish

#############################################################################
# Benchmark: Mergesort with inner shellsort with different inner sort widths.
#############################################################################

benchmark/global_settings.fsh

rm -rf results/

for ISW in 8 16 #24 32 40 48 56 64
	
	echo "Setting inner sort width to $ISW"
	set -x INNER_SORT_WIDTH $ISW

	make clean
	make mergesort PAD_SIZE=4 # PAD_SIZE should be global
	make tools
	
	perl benchmark/benchmark.pl --min=100 --max=2000 --num=50 --iterations=3 --distribution=random
	mv results/mergesort_results.csv "results/mergesort_$ISW\_results.csv" # results dir is global
	
end

perl benchmark/reduce.pl
