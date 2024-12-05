#!/usr/bin/env raku

proto MAIN(Int $day, Int $part, Str $file where *.IO.f, |) {*}

multi MAIN(1, 1, $file) {
	# As one liner:
	say [+] ([Z] ([Z] $file.IO.lines.map(*.words>>.Int))>>.sort).map({ (.[0] - .[1]).abs });

	# Cleaner
	my ($a, $b) = ([Z] $file.IO.lines.map(*.words>>.Int))>>.sort;
	say [+] ($a <<->> $b)>>.abs;
}

multi MAIN(1, 2, $file) {
	my (@a, @b) = [Z] $file.IO.lines.map(*.words>>.Int);
	@b = @b.Bag;
	say [+] @a.map({@_ * @b{@_}});
}

sub day2_verify(@r) {
	(([<] @r) || ([>] @r)) && ((@r Z- @r.tail(*-1))>>.abs.all == 1|2|3)
}

multi MAIN(2, 1, $file) {
	my $count = 0;
	for $file.IO.lines -> $line {
		my $r = $line.words>>.Int;
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
				# TODO: Implement swap with last and subrange instead of allocating list every time
				if day2_verify(@raport.slice(^$i, ($i+1)..*)) {
					$count += 1;
					last;
				}
			}
		}

	}
	say $count;
}

multi MAIN(3, 1, $file) {
	given $file.IO.slurp {
		say (for m:exhaustive{ 'mul('( \d ** 1..3) ',' (\d**1..3) ')' } -> $match {
			$match[0] * $match[1]
		}).sum
	}
}

multi MAIN(3, 2, $file) {
	my $enabled = True;
	my $total = 0;

	given $file.IO.slurp {
		for m:exhaustive{ ("do" "n't"? "()") || "mul("( \d ** 1..3) "," (\d**1..3) ")" } {
			when "do()"    { $enabled = True  }
			when "don't()" { $enabled = False }
			default        { $total += $_[0] * $_[1] if $enabled }
		}
	}

	say $total;
}

# First annoyance with Raku - there is no builtin two element value container
# So the next best solution is to use string like in TCL
# Great.
multi MAIN(4, 1, $file) {
	my @grid = $file.IO.lines>>.comb;

	my @match = do for -1..1 X -1..1 -> ($dy, $dx) {
		(for 1..3 { ["MAS".comb[$_-1], $dx*$_, $dy*$_] }) if $dx != 0 or $dy != 0
	};

	my @vx = 0..^@grid.elems;
	my @vy = 0..^@grid[0].elems;

	my $count = 0;
	for 0..^@grid X 0..^@grid -> ($y, $x) {
		if @grid[$x;$y] eq "X" {
			dir: for @match -> @dir {
				for @dir -> ($exp, $dx, $dy) {
					my $nx = $x + $dx;
					my $ny = $y + $dy;
					next dir unless $nx (elem) @vx && $ny (elem) @vy && @grid[$nx;$ny] eq $exp;
				}
				$count++;
			}
		}
	}

	say $count;
}

multi MAIN(4, 2, $file) {
	my @grid = $file.IO.lines>>.comb;
	my $count = 0;

	for 1..^@grid-1 X 1..^@grid-1 -> ($y, $x) {
		if @grid[$x][$y] eq "A" {
			$count++ if
				"@grid[$x-1;$y-1]@grid[$x+1;$y+1]" ~~ /"MS"|"SM"/ &&
				"@grid[$x+1;$y-1]@grid[$x-1;$y+1]" ~~ /"MS"|"SM"/;
		}
	}

	say $count;
}
