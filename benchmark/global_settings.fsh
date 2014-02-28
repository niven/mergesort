#!/usr/local/bin/fish

# Global settings for all benchmarks

echo "Setting global settings"

set -x PAD_SIZE 7
set -x RESULTS_DIR "results/"

# for mergesorts with inner sort
set -x SORTER_BLOCK_WIDTH 16

# benchmark controls
set -x NUM_ELEMENTS 50000
set -x ITERATIONS 10