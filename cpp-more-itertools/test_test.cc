#include "test.hh"

#include <vector>

namespace test::test
{
	void runtime()
	{
		assert_eq("[ 1 2 3 ]"sv, to_string(std::vector{1, 2, 3}));
		assert_eq("[ [ 1 2 3 ] [ 4 5 ] ]"sv, to_string(std::vector{std::vector{1, 2, 3}, std::vector{4, 5}}));
	}
}
