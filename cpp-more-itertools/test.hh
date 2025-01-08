#include <format>
#include <string_view>
#include <source_location>
#include <print>
#include <utility>
#include <tuple>


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

	inline std::string to_string(char c)
	{
		return std::string{c};
	}

	inline std::string to_string(bool b)
	{
		return b ? "true" : "false";
	}

	inline std::string to_string(std::string s) { return std::string(s); }
	inline std::string to_string(std::string_view s) { return std::string(s); }

	template<typename Tuple>
	concept tuple_like = requires () { typename std::tuple_size<Tuple>::type; };

	std::string to_string(tuple_like auto t)
	{
		return "[" + [&]<std::size_t ...I>(std::index_sequence<I...>) {
			return ((' ' + to_string(std::get<I>(t))) + ...);
		}(std::make_index_sequence<std::tuple_size_v<decltype(t)>>()) + " ]";
	}

	std::string to_string(std::ranges::input_range auto &&range)
	{
		std::string result = "[";
		for (auto&& element : range) {
			result += ' ';
			result += to_string(element);
		}
		return result + " ]";
	}
}

