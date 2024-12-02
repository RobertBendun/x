#!/usr/bin/env raku

proto MAIN(Int $day, Int $part, Str $file where *.IO.f, |) {*}

multi MAIN(1, 1, $file) {
	# As one liner:
	say [+] ([Z] ([Z] $file.IO.lines.map(*.split('  ')>>.Int))>>.sort).map({ (.[0] - .[1]).abs });

	# Cleaner
	my ($a, $b) = ([Z] $file.IO.lines.map(*.split('  ')>>.Int))>>.sort;
	say [+] ($a <<->> $b)>>.abs;
}

multi MAIN(1, 2, $file) {
	my ($a, $b) = [Z] $file.IO.lines.map(*.split('  ')>>.Int);
	$b = $b.Bag;
	say [+] $a.map({$_ * $b{$_}});
}

sub day2_verify(@r) {
	(([<] @r) || ([>] @r)) && ((@r Z- @r.tail(*-1))>>.abs.all == 1|2|3)
}

multi MAIN(2, 1, $file) {
	my $count = 0;
	for $file.IO.lines -> $line {
		my $r = $line.split(' ')>>.Int;
		$count += 1 if day2_verify($r);
	}
	say $count;
}

multi MAIN(2, 2, $file) {
	my $count = 0;
	for $file.IO.lines -> $line {
		my @raport = $line.split(' ')>>.Int;

		if day2_verify(@raport) {
			$count += 1;
		} else {
			for ^@raport -> $i {
				if day2_verify(@raport.slice(^$i, ($i+1)..*)) {
					$count += 1;
					last;
				}
			}
		}

	}
	say $count;
}
