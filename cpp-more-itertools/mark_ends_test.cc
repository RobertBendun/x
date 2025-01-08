#include "test.hh"

#include "mark_ends.hh"

#include <iterator>
#include <forward_list>
#include <list>
#include <vector>

// static_assert(!std::input_iterator<ranges::iterator::mark_ends<std::istream_iterator<int>>>);
static_assert(std::forward_iterator<ranges::iterators::mark_ends<std::forward_list<int>::iterator>>);
static_assert(std::bidirectional_iterator<ranges::iterators::mark_ends<std::list<int>::iterator>>);
static_assert(std::random_access_iterator<ranges::iterators::mark_ends<std::vector<int>::iterator>>);

namespace test::mark_ends
{
	void runtime()
	{
		assert_eq("[ [ true false 1 ] [ false false 2 ] [ false false 3 ] [ false true 4 ] ]"sv, to_string(std::vector{1, 2, 3, 4} | views::mark_ends));
	}
}
