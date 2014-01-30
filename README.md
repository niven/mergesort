# So What's All This Then?
	
I wanted to do some C for fun, and also finally implement a sort that uses another sort in it's inner loop. This was something that I've always read was a thing sorting algorithms do for performance. Then it got a bit out of hand. Now I'm implementing interesting sort algorithms and doing benchmarking. So yes, the internet is filled with benchmarks, sorting algorithms and the like, but I find that reading a high level description and looking at some graphs doesn't do it for me.

# TODO
- timsort doesn't have "Gallop Mode" for merge_hi() yet
- stdlib merge/heap return -1 on error and apparently qsort can't fail :) So find some way to put that in the main_template.c
- print_array could also be generic (maybe have a tostring somewere)
- gen_random_structs should be able to generate data that is random (check),zigzag asc/desc(check), organ pipes, equal numbers(check), etc.
- create a benchmark.conf file that specifies a series of benchmarks to run, and the parameters for each one


# Sorting algorithms

## C Standard library sorts (qsort, mergesort, heapsort)

I put these in for comparisons reasons. Stdlib mergesort has the strange restriction of the min width of the elements to be sizeof(void*)/2, I haven't found out why yet.

## Mergesort

Non-recursive mergesort that is essentially a copy of the Javascript one in the /js repo. This mergesort is very simple to understand, doesn't suffer form overflow when arrays are large and is not recursive. Fun fact: I never realized people assume mergesort is recursive. I never did and never implemented it that way. I only discovered this after programming for 20 years. Thanks Damian :)

This also uses an inner sort.

## Pyramid Mergesort

Same as the above mergesort, but merges blocks as soon as possible to take advantage of elements still being in L1/2/3 caches. Haven't seen any evidence of that so far, but haven't given up either. This also uses an inner sort, and I rewrote it a bit to not use indices but all pointers and that seems to make it quite a bit faster.

## Shellsort

The least appreciated of sorts. This is actually very fast for small runs and doesn't use extra space. Someone named Marcin Ciura did a lot of benchmarking to figure out the optimal gap sequence and that is the one I'm using.

## Insertionsort

Implemented this to have an alternative to shellsort for the inner loops.

## Timsort

Apparently the standard sort in Python at the moment. It's basically mergesort with some tricks and optimizations. Instead of porting this from the Python source I'm going with the description form the author (Tim Peters) found here: bugs.python.org/file4451/timsort.txt

This has sure been educational, so shoutout to him. I'm mostly done with this, except for Gallop Mode for merging runs.

This algorithm supposedly has an advantage over others when sorting "Real World Data". Now I don't have any of that lying around, so this led me to implement some things that generate datasets with appropriate characteristics.

## More Cool sorts

- natural mergesort
- recursive mergesort
- quicksort3
- smoothsort
- samplesort
- inplace mergesort that is somewhere on my work machine in Go

# Benchmarking

Note: I'm doing this on my 2013 Macbook Pro. Timings courtesy of Mach timing functions that actually return "ticks" (ticks turn out to be nanoseconds).
I've put in an #ifdef that uses standard time.h type stuff, but am not able to test this, so let me know if you are :)

The problem with benchmarking on modern hardware is that CPUs can do more at the same time, reorder your instructions and sometimes "step down" to save power.

To run the benchmark:

    perl benchmark.pl --min=100 --max=20000 --num=100 --element_size=8 --iterations=3

It will compile everything and generate files with randomly generated "Widgets". These are structs with a uint32_t number used for comparisons and a variable padding. You can set this with *element_size*. In practice Widgets may have different sizes due to struct alignment.

Other options are *min/max* which specify the size of the datasets, and *num* specifies the number of steps to take. *iterations* specifies how many time each sort is run to avoid random spikes.

## Generating "Real World Data"

This actually means generating data with specific patterns in it, like ranges that are already sorted, high number of equal items etc.

### Update 1
Currently I'm figuring out how to create a "sawtooth" pattern that is made up of increasing sets of variable lengths around a mean.
I came up with a simple but in retrospect awful rejection method.
Instead, I'm going to go with the Ziggurat Algorithm as unclearly described on Wikipedia (http://en.wikipedia.org/wiki/Ziggurat_algorithm) and very comprehensibly here: http://heliosphan.org/zigguratalgorithm/zigguratalgorithm.html

### Update 2
saw_up and saw_down are done. Organ pipes are next up. This is an interesting case I learned about through John Bentley's video/talk about 3 quicksorts as a pattern that some qsort version was very bad at.

Maybe it's also good to add a 'sorted' pattern as that is both real world data and an interesting edge case for some
algorithms.

First finishing timsort though, but is a bit of a chore since it is *a lot* of code for sorting.


# Updates

# Update 1: First Benchmark
Ran some test to compare how different widths for inner shellsort do and made a graph comparing them to std qsort and mergesort.
The good news: my mergesort looks like N log(N)
The bad news: std implementations do much better

I need to run it a bit more often to get some better averages (this was just a first test to see how it goes)
image: terrible_result_1.png

# Update 2: Compiling with -O3 makes your program run faster
Compiled with -O3. Results now super close to stdlib!
Still not seeing any kind of cache related problem.

# Update 3: Added Pyramid Mergesort
results_3.png

Comparing merge_sort and pyramid_merge with disappointing results: pyramid is actually slower.
Haven't seen any evidence of cache problems in either version though.

# Update 4: Using a callback for comparisons in Mergesort
results_4.png

Now converted everything but pyramid_merge into a generic sort (same interface as stdlib ones have)
Now things "finally" look in favor of the stdlib functions, which is a good thing.
Note: the mergesort with inner shellsort uses an inner_block_width of 4 which is a very bad choice I'm sure.
The difference is large enough that it makes sense reimplementing a sort if you know your datatype beforehand and if you really need the performance.

# Update 5: Pyramid Mergesort now also with comparison callback
results_5.png

Pyramid_mergesort now also generic and ran a benchmark with inner_sort_width=100.
The mergesorts still have a lot of innefficient code though.
Next step is to see if I can actually find any effect from L1/2/3 caches.

# Update 6: Timsort
results_6.png

Timsort (http://bugs.python.org/file4451/timsort.txt) is the sort used by Python. This sort purports to be fast on "Real World Data", on of which is a dataset that has internal order (subsets of it are already sorted). Performance is pretty good and beating out mergesort which is nice. It doesn't get better that qsort though. Maybe it'll get better with saw_down (which qsort should be bad at).



# Building

    make all

or

    make verbose

Since we are sorting structs that have padding, you can also set the padding size which defaults to 4 (creating structs of size 8 bytes)
Override this with:
 
    make verbose PAD_SIZE=6

Using *verbose* will print a ton of information on what is happening exactly when you run the sorts.

The mergesorts use shellsort as an inner sort, with a default block width of 4, which is convenient for debugging.
You can override this with

    set -x SORTER_BLOCK_WIDTH (for the excellent fish shell) 
	
or the equivalent export if you use bash or something.
The perl benchmark script will abort if you haven't set this.