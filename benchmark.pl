use strict;
use warnings FATAL => 'all';

use Data::Dumper;
use File::Path qw( make_path remove_tree );
use Getopt::Long;
use POSIX qw( ceil );

local $\ = "\n";

my %opt = (
	min => 10,
	max => 1000,
	num => 50,
	iterations => 1,
	element_size => 8,
);

GetOptions (
	\%opt,
	"s|start=i",
	"e|end=i",
	"n|steps=i",
	"i|iterations=i",
	"z|element_size=i",
	)
or die "Usage: perl benchmark.pl --start=NN --end=NN --steps=NN --iterations=NN --element_size_bytes=NN";

$opt{step} = ceil( ($opt{max}-$opt{min}) / $opt{num} ); 

print "Running benchmark for $opt{min} to $opt{max} elements in $opt{num} steps of size $opt{step}";
print "Element size $opt{element_size} bytes, $opt{iterations} iterations per sorter";

die "SORTER_BLOCK_WIDTH not set for mergesorts" if !defined $ENV{SORTER_BLOCK_WIDTH};
print "Inner sort width for mergesorts: $ENV{SORTER_BLOCK_WIDTH}";

if( $opt{element_size} < 5 ) {
	die "Elements can's be smaller than 5 since they are a struct of a 4 byte int and at minimum 1 char";
}

# first clean & make everything
system "make clean";
$opt{element_size} -= 4; # reduce by size of number member uint32_t
my $make_cmd = "make all PAD_SIZE=$opt{element_size}";
print "Compiling with: $make_cmd";
system $make_cmd;

if( $opt{max} > 10_000 ) {
	print "Removing insertionsort since you probably don't want to wait forever";
	unlink "bin/insertionsort";
}

my $testdata_dir = "testdata";
my $sorters_dir = "bin";
my $results_dir = "results";

# check if stuff exists
die "Can't find gen_random_structs" if !-e "gen_random_structs";

die "./$sorters_dir does not exist" if !-e $sorters_dir;

my @sorters = glob("$sorters_dir/*");
die "No sorters in $sorters_dir" if scalar @sorters == 0;

# create results dir
if( -e $results_dir ) {
	remove_tree( $results_dir );
}
make_path( $results_dir );

# create test files from scratch
if( -e $testdata_dir ) {
	remove_tree( $testdata_dir );
}
make_path( $testdata_dir );

# create files and run tests
my $size = $opt{min};
while( $size <= $opt{max} ) {
	
	print "Benchmarking $size elements";
	
	# random numbers
	my $datafile = "$testdata_dir/data_$size.dat";
	system "./gen_random_structs $size $datafile > /dev/null";
	
	for (1..$opt{iterations}) {
		
		# sorters write their count/nano to stderr so we have them append that to a results file
		for my $sorter (@sorters) {
			my ($name) = $sorter =~ m/\/(.*)/; # name is everything after the / 
			#		test 	input	  output	append stderr to results
			my $cmd = "$sorter $datafile /dev/null 2>>$results_dir/${name}_results.csv";
			system $cmd; # use string so system() uses the shell
	        if ($? == -1) {
	       		print "failed to execute: $!\n";
			} elsif ($? & 127) {
	        	printf "child died with signal %d, %s coredump\n", ($? & 127), ($? & 128) ? 'with' : 'without';
	       	} elsif( $? != 0 ) {
	       		printf "child exited with value %d\n", $? >> 8;
	        }
			if( $? != 0 ) {
				exit;
			}
		}
	}
	
	$size += $opt{step};
}

# now munge results for averages and combine all of them in a nice CSV so we can have a spreadsheet make graphs
# (this feels like a miniature bigdata setup :)
my @results = glob("$results_dir/*.csv");
my $data = {};
for my $result_csv (@results) {
	my ($name) = $result_csv =~ m/$results_dir\/(.*)_results\.csv/; # more name extraction
	open(my $CSV, "<", $result_csv);
	while( <$CSV> ) {
		chomp;
		my ($count, $nanos, @rest) = split /,/;
		# update the average for that count for the current name
		$data->{ $count }->{ $name }->{num}++;
		$data->{ $count }->{ $name }->{avg} += ($nanos-($data->{ $count }->{ $name }->{avg}||0)) / $data->{ $count }->{ $name }->{num};
	}
	close($CSV);
}

# now write a nice CSV
open(my $OUT, ">", "$results_dir/overall.csv");
my @all_names = sort keys %{$data->{$opt{min}}};
print $OUT "Elements," . join(",", @all_names);

for my $c ( sort { $a <=> $b } keys %$data ) {
	print $OUT "$c," . join(",", map { $data->{$c}->{$_}->{avg}/$c } @all_names );
}

close($OUT);

print "Done";