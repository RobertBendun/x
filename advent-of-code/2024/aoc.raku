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
