use strict;
use warnings FATAL => 'all';

use Data::Dumper;
use File::Path qw( make_path remove_tree );
use Getopt::Long;
use POSIX qw( ceil );

local $\ = "\n";

my @distributions = qw( random saw_up );

my %opt = (
	min => 10,
	max => 1000,
	num => 50,
	iterations => 1,
	element_size => 8,
	mem => 0,
	distribution => $distributions[0],
);

GetOptions (
	\%opt,
	"min=i",
	"max=i",
	"num=i",
	"iterations=i",
	"element_size=i",
	"distribution=s"
	)
or die "Usage: perl benchmark.pl --min=NN --max=NN --num=NN --iterations=NN --element_size_bytes=NN --distribution=" . join("|", @distributions);

die "Not a valid distribution '$opt{distribution}'. Choose one of " . join(", ", @distributions) if grep { $_ eq $opt{distribution} } @distributions == 0;

$opt{step} = ceil( ($opt{max}-$opt{min}) / $opt{num} ); 

print "Running benchmark for $opt{min} to $opt{max} elements in $opt{num} steps of size $opt{step} with distribution $opt{distribution}";
print "Element size $opt{element_size} bytes, $opt{iterations} iterations per sorter";

die "SORTER_BLOCK_WIDTH not set for mergesorts (set it in the environment with set -x SORTER_BLOCK_WIDTH 32)" if !defined $ENV{SORTER_BLOCK_WIDTH};
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
	system "./gen_random_structs $opt{distribution} $size $datafile > /dev/null";
	
	for (1..$opt{iterations}) {
		
		# sorters write their count/nano to stderr so we have them append that to a results file
		for my $sorter (@sorters) {
			my ($name) = $sorter =~ m/\/(.*)/; # name is everything after the / 
			#		test 	input	  output	append stderr to results
			my $cmd = "$sorter $datafile /dev/null 2>>$results_dir/${name}_results.csv";
			system $cmd; # use string so system() uses the shell
	        if ($? == -1) {
	       		print "failed to execute: $!\n$sorter $datafile\n";
			} elsif ($? & 127) {
	        	printf "$name died with signal %d, %s coredump\n$sorter $datafile\n", ($? & 127), ($? & 128) ? 'with' : 'without';
	       	} elsif( $? != 0 ) {
	       		printf "$name exited with value %d\n$sorter $datafile\n", $? >> 8;
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
my %names = (); # collect names inelegantly
for my $result_csv (@results) {
	my ($name) = $result_csv =~ m/$results_dir\/(.*)_results\.csv/; # more name extraction
	$names{$name} = 1;
	open(my $CSV, "<", $result_csv);
	while( <$CSV> ) {
		chomp;
		my ($num_elements, $ticks, $memory) = split /,/;
		# update the average for that count for the current name
		my $param = $opt{mem} ? $memory : $num_elements;
		my $value = $ticks/$num_elements;
		$data->{ $param }->{ $name }->{num}++;
		$data->{ $param }->{ $name }->{avg} += ($value-($data->{ $param }->{ $name }->{avg}||0)) / $data->{ $param }->{ $name }->{num};
	}
	close($CSV);
}

# now write a nice CSV
open(my $OUT, ">", "$results_dir/overall.csv");
my @all_names = sort keys %names;
print $OUT ($opt{mem} ? "Memory," : "Elements,") . join(",", @all_names);

for my $param ( sort { $a <=> $b } keys %$data ) {
	print $OUT "$param," . join(",", map { $data->{$param}->{$_} ? $data->{$param}->{$_}->{avg} : "" } @all_names );
}

close($OUT);

print "Done";