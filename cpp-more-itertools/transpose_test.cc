#include "transpose.hh"

#include <vector>

using test_type = std::vector<std::vector<int>>;
using outer_transpose_range = ranges::transpose<test_type>;
static_assert(std::input_or_output_iterator<ranges::iterators::transpose<test_type::iterator>>);
using outer_transpose_iterator = std::ranges::iterator_t<outer_transpose_range>;
using inner_transpose_iterator = std::iter_value_t<outer_transpose_iterator>;

static_assert(std::random_access_iterator<outer_transpose_iterator>);
static_assert(std::random_access_iterator<inner_transpose_iterator>);
static_assert(std::ranges::bidirectional_range<outer_transpose_range>);
static_assert(std::ranges::bidirectional_range<inner_transpose_iterator>);
static_assert(std::ranges::sized_range<outer_transpose_range>);
static_assert(std::ranges::sized_range<inner_transpose_iterator>);

#include "test.hh"

namespace test::transpose
{
	using namespace ::test;

	void runtime()
	{
		assert_eq("[ [ 1 2 3 ] [ 4 5 6 ] ]"sv, to_string(std::vector<std::vector<int>>{{1, 2, 3}, {4, 5, 6}}));
		assert_eq("[ [ 1 4 ] [ 2 5 ] [ 3 6 ] ]"sv, to_string(std::vector<std::vector<int>>{{1, 2, 3}, {4, 5, 6}} | ::views::transpose));
		assert_eq("[ [ 1 2 3 ] [ 4 5 6 ] ]"sv, to_string(std::vector<std::vector<int>>{{1, 2, 3}, {4, 5, 6}} | ::views::transpose | ::views::transpose));
	}
}
