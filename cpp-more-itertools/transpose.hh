#include <iterator>
#include <print>
#include <ranges>
#include <utility>

#define FORWARD_METHODS(name) \
	auto operator++(int) { auto copy = *this; ++*this; return copy; }

#define BIDIRECTIONAL_METHODS(name) \
	FORWARD_METHODS(name) \
	auto operator--(int) { auto copy = *this; --*this; return copy; }

#define RANDOM_ACCESS_METHODS(name) \
	BIDIRECTIONAL_METHODS(name) \
	auto operator+(difference_type d) const { auto copy = *this; copy += d; return copy; } \
	auto operator-(difference_type d) const { auto copy = *this; copy -= d; return copy; } \
	friend auto operator+(difference_type d, name const& i) { return i + d; } \
	reference_type operator*() const { return (*this)[difference_type()]; }

namespace ranges
{
namespace iterators
{
	template<typename Iterator>
	struct transpose
	{
		struct inner
		{
			Iterator iterator;
			std::iter_difference_t<Iterator> i, j, i_max;

			using value_type = std::ranges::range_value_t<std::iter_value_t<Iterator>>;
			using reference_type = std::ranges::range_reference_t<std::iter_value_t<Iterator>>;
			using difference_type = std::iter_difference_t<Iterator>;

			reference_type operator[](difference_type d) const { return iterator[i+d][j]; }
			inner& operator++() { ++i; return *this; }
			inner& operator--() { --i; return *this; }
			inner& operator+=(difference_type d) { i += d; return *this; }
			inner& operator-=(difference_type d) { i -= d; return *this; }
			difference_type operator-(inner const& other) const { return i - other.i; }
			auto operator<=>(inner const& other) const { return i <=> other.i; }
			inner begin() const { return *this; }
			inner end() const { return { .iterator = iterator, .i = i_max, .j = j, .i_max = i_max }; }
			inner cbegin() const { return *this; }
			inner cend() const { return { .iterator = iterator, .i = i_max, .j = j, .i_max = i_max }; }
			bool operator==(inner const&) const = default;
			auto size() const { return i_max; }

			RANDOM_ACCESS_METHODS(inner)
		};

		using value_type = inner;
		using reference_type = value_type;
		using difference_type = std::ranges::range_difference_t<std::iter_value_t<Iterator>>;



		Iterator iterator;
		std::iter_difference_t<Iterator> j, i_max, j_max;

		reference_type operator[](difference_type d) const
		{
			return { .iterator = iterator, .i = 0, .j = j + d, .i_max = i_max };
		}

		transpose& operator++() { ++j; return *this; }
		transpose& operator--() { --j; return *this; }
		transpose& operator+=(difference_type d) { j += d; return *this; }
		transpose& operator-=(difference_type d) { j -= d; return *this; }
		difference_type operator-(transpose const& other) const { return j - other.j; }
		auto operator<=>(transpose const& other) const { return j <=> other.j; }
		bool operator==(transpose const&) const = default;

		RANDOM_ACCESS_METHODS(transpose)
	};
}

template<typename View>
requires std::ranges::random_access_range<View>
	&& std::ranges::random_access_range<std::ranges::range_value_t<View>>
struct transpose : std::ranges::view_interface<transpose<View>>
{
	View underlying;

	auto begin()
	{
		auto begin = std::ranges::begin(underlying);
		auto end = std::ranges::end(underlying);

		return iterators::transpose {
			.iterator = begin,
			.j = 0,
			.i_max = end - begin,
			.j_max = std::end(*begin) - std::begin(*begin),
		};
	}

	auto end()
	{
		auto begin = std::ranges::begin(underlying);
		auto end = std::ranges::end(underlying);

		auto const j_max = std::end(*begin) - std::begin(*begin);

		return iterators::transpose {
			.iterator = begin,
			.j = j_max,
			.i_max = end - begin,
			.j_max = j_max,
		};
	}

	auto cbegin() const
	{
		auto cbegin = std::ranges::cbegin(underlying);
		auto cend = std::ranges::cend(underlying);

		return iterators::transpose {
			.iterator = cbegin,
			.j = 0,
			.i_max = cend - cbegin,
			.j_max = std::cend(*cbegin) - std::cbegin(*cbegin),
		};
	}

	auto cend() const
	{
		auto cbegin = std::ranges::cbegin(underlying);
		auto cend = std::ranges::cend(underlying);

		auto const j_max = std::cend(*cbegin) - std::cbegin(*cbegin);

		return iterators::transpose {
			.iterator = cbegin,
			.j = j_max,
			.i_max = cend - cbegin,
			.j_max = j_max,
		};
	}


	std::size_t size() const
	{
		return cend() - cbegin();
	}
};
}

namespace views
{
	struct transpose_t : std::ranges::range_adaptor_closure<transpose_t>
	{
		template<typename View>
		constexpr auto operator()(View &&view) const
		{
			return ranges::transpose{{}, std::forward<View>(view)};
		}
	} static constexpr transpose;
}

