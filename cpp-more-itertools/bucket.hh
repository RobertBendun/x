#include <concepts>
#include <ranges>

// TODO: Add requirement that we can std::ranges::to from pairs of Key(), val
// TODO: Potential lifetime weirdness

namespace ranges
{
	template<template<typename...> typename C, typename Key, std::ranges::input_range Range>
	requires
		std::invocable<Key, std::ranges::range_reference_t<Range>>
	auto bucket_to(Range &&range, Key &&key)
	{
		return std::forward<Range>(range)
			| std::views::transform([&](auto &&val) { return std::pair{key(val), val}; })
			| std::ranges::to<C>();
	}


	template<template<typename...> typename C, typename Key>
	auto bucket_to(Key &&key)
	{
		return std::views::transform([&](auto &&val) { return std::pair{key(val), val}; })
			| std::ranges::to<C>();
	}
}
