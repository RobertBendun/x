#include <utility>
#include <ranges>

// TODO: Consider passing arguments to std::ranges::to

// Requires two versions: unzip for lvalue references to ranges (TODO parhaps this is what borrowed ranges are for)
// and unzip_to that takes any range and can create subranges in the same way as std::ranges::to (which fixes rvalue lifetimes issue)
// See test for clarification

namespace ranges
{
	template<std::ranges::forward_range Range>
	auto unzip(Range &range)
	{
		using value_type = std::ranges::range_value_t<Range>;
		return []<std::size_t ...I>(std::index_sequence<I...>, auto &&range) {
			return std::tuple {
				(range | std::views::transform([](auto &el) { return std::get<I>(el); }))...
			};
		}(std::make_index_sequence<std::tuple_size_v<value_type>>{}, std::forward<Range>(range));
	}

	template<template<typename... > typename C, std::ranges::forward_range Range>
	auto unzip_to(Range &&range)
	{
		using value_type = std::ranges::range_value_t<Range>;
		return []<std::size_t ...I>(std::index_sequence<I...>, auto &&range) {
			return std::tuple {
				(range | std::views::transform([](auto &el) { return std::get<I>(el); }) | std::ranges::to<C>())...
			};
		}(std::make_index_sequence<std::tuple_size_v<value_type>>{}, std::forward<Range>(range));
	}


	template<template<typename...> typename C>
	struct unzip_to_adaptor : std::ranges::range_adaptor_closure<unzip_to_adaptor<C>>
	{
		template<typename Range>
		auto operator()(Range &&range)
		{
			return unzip_to<C>(std::forward<Range>(range));
		}
	};

	template<template<typename... > typename C>
	auto unzip_to()
	{
		return unzip_to_adaptor<C>{};
	}
}
