#include "test.hh"
#include "unzip.hh"

#include <vector>

namespace test::unzip
{
	void runtime()
	{
		{
			auto data = std::vector{
				std::tuple{1, 2, 3},
				std::tuple{4, 5, 6},
			};
			auto [a, b, c] = ranges::unzip(data);
			assert_eq("[ 1 4 ]", to_string(a));
			assert_eq("[ 2 5 ]", to_string(b));
			assert_eq("[ 3 6 ]", to_string(c));
		}

		{
			auto [a, b, c] = ranges::unzip_to<std::vector>(std::vector{
				std::tuple{1, 2, 3},
				std::tuple{4, 5, 6},
				std::tuple{7, 8, 9},
			});
			assert_eq("[ 1 4 7 ]", to_string(a));
			assert_eq("[ 2 5 8 ]", to_string(b));
			assert_eq("[ 3 6 9 ]", to_string(c));
		}

		{
			auto [a, b, c] = std::vector{
				std::tuple{1, 2, 3},
				std::tuple{4, 5, 6},
			} | ranges::unzip_to<std::vector>();
			assert_eq("[ 1 4 ]", to_string(a));
			assert_eq("[ 2 5 ]", to_string(b));
			assert_eq("[ 3 6 ]", to_string(c));
		}
	}
}
