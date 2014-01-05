use strict;
use warnings FATAL => 'all';

use Data::Dumper;
use File::Path qw( make_path remove_tree );
use Getopt::Long;

local $\ = "\n";

my %opt = (
	min => 10,
	max => 1000,
	target => undef,
);

my $opt_ok = GetOptions (
	\%opt,
	"min=i",
	"max=i",
	"target=s",
	);
	
$opt_ok && $opt{target} or die "Usage: perl test.pl --min=NN --max=NN --target=SSS";

print "Running test for $opt{min} to $opt{max} for $opt{target}";

die "SORTER_BLOCK_WIDTH not set for mergesorts" if !defined $ENV{SORTER_BLOCK_WIDTH};
print "Inner sort width for mergesorts: $ENV{SORTER_BLOCK_WIDTH}";

# first clean & make everything
system "make clean";
my $make_cmd = "make all PAD_SIZE=4 $opt{target}";
print "Compiling with: $make_cmd";
system $make_cmd;

my $testdata_dir = "testdata";
my $sorters_dir = "bin";
my $results_dir = "results";

# check if stuff exists
die "Can't find gen_random_structs" if !-e "gen_random_structs";

die "./bin/$opt{target} does not exist" if !-e "./bin/$opt{target}";

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
	
	if( $size % 32 == 1 ) {
		printf("%d", $size);
	} elsif( $size % 32 == 0 ) {
		printf("%d\n", $size);
	} else {
		printf(".");
	}
	
	# random numbers
	my $datafile = "$testdata_dir/data_$size.dat";
	system "./gen_random_structs $size $datafile &> /dev/null";

	my $cmd = "./bin/$opt{target} $datafile /dev/null 2>/dev/null";
	system $cmd; # use string so system() uses the shell
    if ($? == -1) {
   		print "failed to execute: $!\n";
	} elsif ($? & 127) {
    	printf "child died with signal %d, %s coredump\n", ($? & 127), ($? & 128) ? 'with' : 'without';
   	} elsif( $? != 0 ) {
   		printf "child exited with value %d\n", $? >> 8;
    }
	if( $? != 0 ) {
		printf("Failed at $size elements ($datafile)\n");
		exit;
	}
	
	$size++;
}

print "Done";