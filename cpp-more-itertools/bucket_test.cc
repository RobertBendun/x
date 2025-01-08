#include "test.hh"
#include "bucket.hh"

#include <vector>
#include <map>

namespace test::bucket
{
	void runtime()
	{
		std::vector<std::string_view> vec{"a1", "b1", "c1", "a2", "b2", "c2", "b3"};

		assert_eq("[ [ a a1 ] [ b b1 ] [ c c1 ] ]", to_string(ranges::bucket_to<std::map>(vec, [](auto x) { return x[0]; })));
		assert_eq("[ [ a a1 ] [ a a2 ] [ b b1 ] [ b b2 ] [ b b3 ] [ c c1 ] [ c c2 ] ]", to_string(ranges::bucket_to<std::multimap>(vec, [](auto x) { return x[0]; })));

		assert_eq("[ [ a a1 ] [ b b1 ] [ c c1 ] ]", to_string(vec | ranges::bucket_to<std::map>([](auto x) { return x[0]; })));
		assert_eq("[ [ a a1 ] [ a a2 ] [ b b1 ] [ b b2 ] [ b b3 ] [ c c1 ] [ c c2 ] ]", to_string(vec | ranges::bucket_to<std::multimap>([](auto x) { return x[0]; })));
	}
}
