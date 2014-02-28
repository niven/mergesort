#!/usr/local/bin/fish

#############################################################################
# Run every benchmark
#############################################################################

# command should be called 'source' but hasn't been renamed yet (or my fish shell is old)
. benchmark/global_settings.fsh

rm -rf results/
make clean

fish benchmark/standard_library_sorts.fsh
fish benchmark/mergesort_shellsort.fsh
fish benchmark/all_sorts.fsh
fish benchmark/all_sorts_saw_up.fsh
fish benchmark/all_sorts_saw_down.fsh
fish benchmark/mergesort_showdown.fsh

make clean
