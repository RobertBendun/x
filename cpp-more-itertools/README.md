# `more_itertools` in C++

Learning Python's [`more_itertools`](https://more-itertools.readthedocs.io) library by reimplementing it in C++.

## Usage

Each algorithm is in seperate header file.

Tests for each algorithm are in `algorithm_test.cc` file.

You can build and run tests using `python3 build.py` command.

## Progress

### Grouping

- [x] `chunked`: [`std::views::chunk`](https://en.cppreference.com/w/cpp/ranges/chunk_view)
- [x] `ichunked`: [`std::views::chunk`](https://en.cppreference.com/w/cpp/ranges/chunk_view)
- [ ] `chunked_even`
- [x] `sliced`: [`std::views::chunk`](https://en.cppreference.com/w/cpp/ranges/chunk_view)
- [ ] `constrained_batches`
- [ ] `distribute`
- [ ] `divide`
- [ ] `split_at`
- [ ] `split_before`
- [ ] `split_after`
- [ ] `split_into`
- [ ] `split_when`
- [x] `bucket`: [`ranges::bucket_to`](./bucket.hh)
- [x] `unzip`: [`ranges::unzip, ranges::unzip_to`](./unzip.hh)
- [ ] `batched`
- [ ] `grouper`
- [ ] `partition`
- [x] `transpose`: [`views::transpose`](./transpose.hh)

### Lookahead and lookback

- [ ] `spy`
- [ ] `peekable`
- [ ] `seekable`

### Windowing


- [ ] `windowed`
- [ ] `substrings`
- [ ] `substrings_indexes`
- [ ] `stagger`
- [ ] `windowed_complete`
- [ ] `pairwise`
- [ ] `triplewise`
- [ ] `sliding_window`
- [ ] `subslices`

### Augmenting

- [ ] `count_cycle`
- [ ] `intersperse`
- [ ] `padded`
- [ ] `repeat_each`
- [ ] `mark_ends`
- [ ] `repeat_last`
- [ ] `adjacent`
- [ ] `groupby_transform`
- [ ] `pad_none`
- [ ] `ncycles`

### Combining

- [ ] `collapse`
- [ ] `sort_together`
- [ ] `interleave`
- [ ] `interleave_longest`
- [ ] `interleave_evenly`
- [ ] `zip_offset`
- [ ] `zip_equal`
- [ ] `zip_broadcast`
- [ ] `flatten`
- [ ] `roundrobin`
- [ ] `prepend`
- [ ] `value_chain`
- [ ] `partial_product`

### Summarizing

- [ ] `ilen`
- [ ] `unique_to_each`
- [ ] `sample`
- [ ] `consecutive_groups`
- [ ] `run_length`
- [ ] `map_reduce`
- [ ] `join_mappings`
- [ ] `exactly_n`
- [ ] `is_sorted`
- [ ] `all_equal`
- [ ] `all_unique`
- [ ] `minmax`
- [ ] `first_true`
- [ ] `quantify`
- [ ] `iequals`

### Selecting

- [ ] `islice_extended`
- [ ] `first`
- [ ] `last`
- [ ] `one`
- [ ] `only`
- [ ] `strictly_n`
- [ ] `strip`
- [ ] `lstrip`
- [ ] `rstrip`
- [ ] `filter_except`
- [ ] `map_except`
- [ ] `filter_map`
- [ ] `iter_suppress`
- [ ] `nth_or_last`
- [ ] `unique_in_window`
- [ ] `before_and_after`
- [ ] `nth`
- [ ] `take`
- [ ] `tail`
- [ ] `unique_everseen`
- [ ] `unique_justseen`
- [ ] `unique`
- [ ] `duplicates_everseen`
- [ ] `duplicates_justseen`
- [ ] `classify_unique`
- [ ] `longest_common_prefix`
- [ ] `takewhile_inclusive`

### Math

- [ ] `dft`
- [ ] `idft`
- [ ] `convolve`
- [ ] `dotproduct`
- [ ] `factor`
- [ ] `matmul`
- [ ] `polynomial_from_roots`
- [ ] `polynomial_derivative`
- [ ] `polynomial_eval`
- [ ] `sieve`
- [ ] `sum_of_squares`
- [ ] `totient`

### Combinatorics

- [ ] `distinct_permutations`
- [ ] `distinct_combinations`
- [ ] `circular_shifts`
- [ ] `partitions`
- [ ] `set_partitions`
- [ ] `product_index`
- [ ] `combination_index`
- [ ] `permutation_index`
- [ ] `combination_with_replacement_index`
- [ ] `gray_product`
- [ ] `outer_product`
- [ ] `powerset`
- [ ] `powerset_of_sets`
- [ ] `random_product`
- [ ] `random_permutation`
- [ ] `random_combination`
- [ ] `random_combination_with_replacement`
- [ ] `nth_product`
- [ ] `nth_permutation`
- [ ] `nth_combination`
- [ ] `nth_combination_with_replacement`

### Wrapping

- [ ] `always_iterable`
- [ ] `always_reversible`
- [ ] `countable`
- [ ] `consumer`
- [ ] `with_iter`
- [ ] `iter_except`

### Others

- [ ] `locate`
- [ ] `rlocate`
- [ ] `replace`
- [ ] `numeric_range`
- [ ] `side_effect`
- [ ] `iterate`
- [ ] `difference`
- [ ] `make_decorator`
- [ ] `SequenceView`
- [ ] `time_limited`
- [ ] `map_if`
- [ ] `iter_index`
- [ ] `consume`
- [ ] `tabulate`
- [ ] `repeatfunc`
- [ ] `reshape`
