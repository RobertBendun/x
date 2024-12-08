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

	my $count = 0;
	for 0..^@grid X 0..^@grid -> ($y, $x) {
		if @grid[$x;$y] eq "X" {
			dir: for @match -> @dir {
				for @dir -> ($exp, $dx, $dy) {
					my $nx = $x + $dx;
					my $ny = $y + $dy;
					next dir unless inbounds(@grid, $nx, $ny) && @grid[$nx;$ny] eq $exp;
				}
				++$count
			}
		}
	}

	say $count
}

multi MAIN(4, 2, $file) {
	my @grid = $file.IO.lines>>.comb;
	my $count = 0;

	for 1..^@grid-1 X 1..^@grid-1 -> ($y, $x) {
		++$count
			if @grid[$x;$y] eq "A"
			&& "@grid[$x-1;$y-1]@grid[$x+1;$y+1]" & "@grid[$x+1;$y-1]@grid[$x-1;$y+1]" eq "MS" | "SM";
	}

	say $count;
}


grammar Day5 {
	rule TOP { <order>+ % "\n" <update>+ % "\n" }
	token order { <int> "|" <int> }
	token update { <int>+ % "," }
	token int { \d+ }
}

multi MAIN(5, 1, $file) {
	my $m = Day5.parse($file.IO.slurp);
	my %before .= new;
	my %after .= new;

	for $m<order> -> $order {
		%before.push: +$order<int>[1] => +$order<int>[0];
		%after.push:  +$order<int>[0] => +$order<int>[1];
	}

	my $total = 0;
	for $m<update> -> $update_ {
		my @update = $update_<int>>>.Int;
		$total += @update[* div 2] if all do for @update Z 0..* -> ($page, $i) { !(%before{$page}.one (elem) @update[$i..*]) && !(%after{$page}.one (elem) @update[0..$i]); };
	}
	say $total;
}

multi MAIN(5, 2, $file) {
	my $m = Day5.parse($file.IO.slurp);
	my %before .= new;
	my %after .= new;

	for $m<order> -> $order {
		%before.push: +$order<int>[1] => +$order<int>[0];
		%after.push:  +$order<int>[0] => +$order<int>[1];
	}

	my $total = 0;
	for $m<update> -> $update_ {
		my @update = $update_<int>>>.Int;
		next if all do for @update Z 0..* -> ($page, $i) { !(%before{$page}.one (elem) @update[$i..*]) && !(%after{$page}.one (elem) @update[0..$i]); };
		@update = @update.sort: { %before{$^a}.one == $^b ?? Order::Less !! Order::More }
		$total += @update[* div 2];
	}
	say $total;
}

multi MAIN(6, 1, $file) {
	my $cx = 0;
	my $cy = 0;
	my @grid = $file.IO.lines>>.comb;
	my SetHash $obstacles .= new;

	for @grid Z 0..* -> (@line, $y) {
		for @line Z 0..* -> ($_, $x) {
			# TCL core - using strings as data structures
			when "#" { $obstacles.set: "$x $y" }
			when "^" { $cx = $x; $cy = $y; }
		}
	}

	my $dir = 0;
	my SetHash $visited .= new;

	while inbounds @grid, $cx, $cy {
		$visited.set: "$cx $cy";

		my $nx = $cx + {0 =>  0, 1 => 1, 2 => 0, 3 => -1}{$dir};
		my $ny = $cy + {0 => -1, 1 => 0, 2 => 1, 3 =>  0}{$dir};

		if "$nx $ny" (elem) $obstacles {
			$dir = ($dir + 1) % 4;
			next;
		}

		$cx = $nx;
		$cy = $ny;
	}

	say $visited.elems;

#`(
	for @grid Z 0..* -> (@line, $y) {
		for @line Z 0..* -> ($c, $x) {
			if "$x $y" (elem) $obstacles {
				print "#"
			} elsif "$x $y" (elem) $visited {
				print "X"
			} else {
				print "."
			}
		}
		say "";
	}
)
}

multi MAIN(6, 2, $file) {
	my $cx = 0;
	my $cy = 0;
	my @grid = $file.IO.lines>>.comb;
	my SetHash $obstacles .= new;

	for @grid Z 0..* -> (@line, $y) {
		for @line Z 0..* -> ($_, $x) {
			# TCL core - using strings as data structures
			when "#" { $obstacles.set: "$x $y" }
			when "^" { $cx = $x; $cy = $y; }
		}
	}

	sub try($cx_, $cy_) {
		my $cx = $cx_;
		my $cy = $cy_;
		my $dir = 0;
		my SetHash $visited .= new;

		while inbounds @grid, $cx, $cy {
			if "$cx $cy $dir" (elem) $visited {
				return True;
			}
			$visited.set: "$cx $cy $dir";

			my $nx = $cx + {0 =>  0, 1 => 1, 2 => 0, 3 => -1}{$dir};
			my $ny = $cy + {0 => -1, 1 => 0, 2 => 1, 3 =>  0}{$dir};

			if "$nx $ny" (elem) $obstacles {
				$dir = ($dir + 1) % 4;
				next;
			}

			$cx = $nx;
			$cy = $ny;
		}

		return False;
	}

	my $count = 0;

	my $todo = +@grid * +@grid[0];

	for @grid Z 0..* -> (@line, $y) {
		for @line Z 0..* -> ($_, $x) {
			my $progress = ($y * +@line + $x) * 100 div $todo;
			print "\r$progress";

			when "." {
				$obstacles.set: "$x $y";
				$count++ if try(+$cx, +$cy);
				$obstacles.unset: "$x $y";
			}
		}
	}

	say "";
	say $count;
}


multi MAIN(7, 1, $file) {
	my $total = 0;

	for $file.IO.lines {
		when /(\d+) ": " (\d+)+ % " "/ {
			my $result = $/[0].Int;
			my @vals = $/[1]>>.Int;

			sub calc($i, $v) {
				if $i >= @vals.elems { return $v == $result }
				if $v > $result { return False }
				return calc($i+1, $v + @vals[$i]) || calc($i+1, $v * @vals[$i])
			}

			$total += $result if calc(1, @vals[0]);
		}
	}

	say $total;
}

multi MAIN(7, 2, $file) {
	my $total = 0;

	for $file.IO.lines {
		when /(\d+) ": " (\d+)+ % " "/ {
			my $result = $/[0].Int;
			my @vals = $/[1]>>.Int;

			sub calc($i, $v) {
				if $i >= +@vals { return $v == $result }
				if $v > $result { return False }
				return calc($i+1, $v + @vals[$i])
					|| calc($i+1,   $v * @vals[$i])
					|| calc($i+1,   $v ~ @vals[$i])
			}

			$total += $result if calc(1, @vals[0]);
		}
	}

	say $total;
}

multi MAIN(8, 1, $file) {
	my @grid = $file.IO.lines>>.comb;

	# % sygil makes Hash type by default
	my %antennas;

	for @grid Z 0..* -> (@line, $y) {
		for @line Z 0..* -> ($c, $x) {
			next if $c eq ".";
			%antennas{$c}.push: "$x $y";
		}
	}

	my SetHash $antinodes .= new;

	for %antennas.kv -> $antenna, @locations {
		for @locations X @locations -> ($a, $b) {
			next if $a eq $b;
			my ($x1, $y1) = $a.words>>.Int;
			my ($x2, $y2) = $b.words>>.Int;
			my ($xa, $ya) = $x1 + ($x1 - $x2), $y1 + ($y1 - $y2);
			$antinodes.set: "$xa $ya" if inbounds @grid, $xa, $ya;
		}
	}

#`(
	for @grid Z 0..* -> (@line, $y) {
		for @line Z 0..* -> ($c, $x) {
			when "$x $y" (elem) $antinodes { print "#" }
			print $c
		}
		say "";
	}
)

	say +$antinodes;
}

multi MAIN(8, 2, $file) {
	my @grid = $file.IO.lines>>.comb;

	# % sygil makes Hash type by default
	my %antennas;

	for @grid Z 0..* -> (@line, $y) {
		for @line Z 0..* -> ($c, $x) {
			next if $c eq ".";
			%antennas{$c}.push: "$x $y";
		}
	}

	my SetHash $antinodes .= new;

	for %antennas.kv -> $antenna, @locations {
		for @locations X @locations -> ($a, $b) {
			next if $a eq $b;
			my ($x1, $y1) = $a.words>>.Int;
			my ($x2, $y2) = $b.words>>.Int;
			for 0..* -> $s {
				my ($xa, $ya) = $x1 + $s * ($x1 - $x2), $y1 + $s * ($y1 - $y2);
				if inbounds @grid, $xa, $ya {
					$antinodes.set: "$xa $ya"
				} else {
					last;
				}
			}
		}
	}

#`(
	for @grid Z 0..* -> (@line, $y) {
		for @line Z 0..* -> ($c, $x) {
			when "$x $y" (elem) $antinodes { print "#" }
			print $c
		}
		say "";
	}
)

	say +$antinodes;
}

sub inbounds(@grid, $x, $y) {
	return $x >= 0 && $x < +@grid[0] && $y >= 0 && $y < +@grid;
}
