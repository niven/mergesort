use strict;
use warnings FATAL => 'all';

use Data::Dumper;
use File::Path qw( make_path remove_tree );
use POSIX qw( ceil );

local $\ = "\n";

die "Usage: perl benchmark.pl start step count" if scalar @ARGV != 4;

my ($min, $max, $num, $iterations) = @ARGV;
my $step = ceil( ($max-$min) / $num ); 

print "Running benchmark for $min to $max elements in $num steps of size $step, $iterations iterations per sorter";

my $testdata_dir = "testdata";
my $sorters_dir = "bin";
my $results_dir = "results";

# check if stuff exists
die "Can't find gen_random_ints" if !-e "gen_random_ints";

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
my $size = $min;
while( $size <= $max ) {
	
	print "Benchmarking $size elements";
	
	# random numbers
	my $datafile = "$testdata_dir/data_$size.dat";
	system "./gen_random_ints $size $datafile > /dev/null";
	
	for (1..$iterations) {
		
		# sorters write their count/nano to stderr so we have them append that to a results file
		for my $sorter (@sorters) {
			my ($name) = $sorter =~ m/\/(.*)/; # name is everything after the / 
			#		test 	input	  output	append stderr to results
			my $cmd = "$sorter $datafile /dev/null 2>>$results_dir/${name}_results.csv";
			system $cmd; # use string so system() uses the shell
		}
	}
	
	$size += $step;
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
		my ($count, $nanos) = split /,/;
		# update the average for that count for the current name
		$data->{ $count }->{ $name }->{num}++;
		$data->{ $count }->{ $name }->{avg} += ($nanos-($data->{ $count }->{ $name }->{avg}||0)) / $data->{ $count }->{ $name }->{num};
	}
	close($CSV);
}

# now write a nice CSV
open(my $OUT, ">", "$results_dir/overall.csv");
my @all_names = sort keys %{$data->{$min}};
print $OUT "Elements," . join(",", @all_names);

for my $c ( sort { $a <=> $b } keys %$data ) {
	print $OUT "$c," . join(",", map { $data->{$c}->{$_}->{avg} } @all_names );
}

close($OUT);

print "Done";