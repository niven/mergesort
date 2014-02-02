use warnings FATAL => 'all';
use strict;

local $\ = "\n";

my $results_dir = "results";

# now munge results for averages and combine all of them in a nice CSV so we can have gnuplot make graphs
# (this feels like a miniature bigdata setup :)
my @results = glob("$results_dir/*.csv");
my $data = {};
my %names = (); # collect names inelegantly
for my $result_csv (@results) {
	print $result_csv;
	my ($name) = $result_csv =~ m/$results_dir\/(.*)_results\.csv/; # more name extraction
	$names{$name} = 1;
	open(my $CSV, "<", $result_csv);
	while( <$CSV> ) {
		chomp;
		my ($num_elements, $ticks) = split /,/;
		# update the average for that count for the current name
		my $param = $num_elements;
		my $value = $ticks/$num_elements;
		$data->{ $param }->{ $name }->{num}++;
		$data->{ $param }->{ $name }->{avg} += ($value-($data->{ $param }->{ $name }->{avg}||0)) / $data->{ $param }->{ $name }->{num};
	}
	close($CSV);
}

# now write a nice CSV
open(my $OUT, ">", "$results_dir/overall.csv");
my @all_names = sort keys %names;
print $OUT "Elements," . join(",", map { my $n = $_; $n =~ s/_/ /; "\"$n\"" } @all_names);

for my $param ( sort { $a <=> $b } keys %$data ) {
	print $OUT "$param," . join(",", map { $data->{$param}->{$_} ? $data->{$param}->{$_}->{avg} : "" } @all_names );
}

close($OUT);



print "Done";