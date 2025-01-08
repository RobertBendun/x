#include "common.hh"
#include <ranges>

namespace ranges::iterators
{
	// It can't be just an input iterator since we cannot test if they end without advancing
	template<std::forward_iterator Iterator>
	struct mark_ends
	{
		using value_type = std::tuple<bool, bool, std::iter_value_t<Iterator>>;
		using reference_type = std::tuple<bool, bool, std::iter_reference_t<Iterator>>;
		using difference_type = std::iter_difference_t<Iterator>;

		Iterator iterator, begin, end;

		reference_type operator*() const
		{
			return { iterator == begin, std::next(iterator) == end, *iterator };
		}

		reference_type operator[](difference_type d) const
		{
			auto target = iterator + d;
			return { target == begin, std::next(target) == end, *target };
		}

		mark_ends& operator++() { ++iterator; return *this; }
		mark_ends& operator--() requires std::bidirectional_iterator<Iterator> { --iterator; return *this; }

		bool operator==(mark_ends const& other) const
		{
			return iterator == other.iterator;
		}

		auto operator<=>(mark_ends const& other) const requires std::random_access_iterator<Iterator>
		{
			return iterator <=> other.iterator;
		}

		difference_type operator-(mark_ends const& other) const requires std::random_access_iterator<Iterator>
		{
			return iterator - other.iterator;
		}

		auto& operator+=(difference_type d) requires std::random_access_iterator<Iterator> { iterator += d; return *this; }
		auto& operator-=(difference_type d) requires std::random_access_iterator<Iterator> { iterator -= d; return *this; }

		RANDOM_ACCESS_METHODS(mark_ends)
	};
}

namespace ranges
{
	template<std::ranges::forward_range Range>
	struct mark_ends
	{
		using iterator = std::ranges::iterator_t<Range>;
		Range range;

		iterators::mark_ends<iterator> begin()
		{
			auto begin = std::ranges::begin(range);
			return { .iterator = begin, .begin = begin, .end = std::ranges::end(range) };
		}

		iterators::mark_ends<iterator> end()
		{
			auto end = std::ranges::end(range);
			return { .iterator = end, .begin = std::ranges::begin(range), .end = end };
		}
	};
}

namespace views
{
	struct mark_ends_adaptor : std::ranges::range_adaptor_closure<mark_ends_adaptor>
	{
		template<std::ranges::forward_range Range>
		auto operator()(Range &&range)
		{
			return ranges::mark_ends(std::forward<Range>(range));
		}
	} static inline mark_ends [[maybe_unused]];
}

