#include <string_view>
#include <source_location>
#include <print>

namespace test
{
	using namespace std::string_view_literals;

	void assert_eq(auto const& lhs, auto const& rhs, std::string_view message = {}, std::source_location loc = std::source_location::current())
	{
		if (lhs != rhs) {
			std::print(stderr, "{}:{}:{}: error: {}\n", loc.file_name(), loc.line(), loc.column(), message);
			std::print(stderr, "{}:{}:{}: info: lhs: {}\n", loc.file_name(), loc.line(), loc.column(), lhs);
			std::print(stderr, "{}:{}:{}: info: rhs: {}\n", loc.file_name(), loc.line(), loc.column(), rhs);
		}
	}

	using std::to_string;

	std::string to_string(std::ranges::input_range auto &&range)
	{
		std::string result = "[";
		for (auto const& element : range) {
			result += ' ';
			result += to_string(element);
		}
		return result + " ]";
	}
}

