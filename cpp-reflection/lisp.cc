/* TODO: Change this example from lisp to forth, and allow for 2 types of bound functions:
 *   1. Those with type Stack -> Stack
 *   2. Those with type (A, B, C) -> (D, E) reflecting the a b c -- d e
 *
 *   The second one will be converted to the first one with automatic runtime checks of stack effect validity
 *   Maybe we could use even something like Porth type system and check against those types without the need
 *   to specify the additional signature
 */

#include <meta>
#include <print>
#include <string>

namespace lisp
{
	struct Value
	{
	};

	struct Static_Name { char const* name; };

	consteval Static_Name bind(auto const& array)
	{
		return Static_Name { .name = std::define_static_string(array) };
	}

	void load_environment();
}


namespace lisp::initial_environment
{
	[[=lisp::bind("+")]] lisp::Value add(Value args, Value env);
	[[=lisp::bind("-")]] lisp::Value sub(Value args, Value env);
	[[=lisp::bind("*")]] lisp::Value mul(Value args, Value env);
	[[=lisp::bind("/")]] lisp::Value div(Value args, Value env);
}

lisp::Value lisp::initial_environment::add(Value args, Value env) { return {}; }
lisp::Value lisp::initial_environment::sub(Value args, Value env) { return {}; }
lisp::Value lisp::initial_environment::mul(Value args, Value env) { return {}; }
lisp::Value lisp::initial_environment::div(Value args, Value env) { return {}; }

// Note that implementation of this function is positional.
void lisp::load_environment()
{
	constexpr auto ctx = std::meta::access_context::current();

	template for (constexpr std::meta::info member :
			std::define_static_array(std::meta::members_of(^^lisp::initial_environment, ctx)))
	{
		template for (constexpr std::meta::info annot :
				std::define_static_array(std::meta::annotations_of_with_type(member, ^^lisp::Static_Name)))
		{
			constexpr auto count = std::meta::parameters_of(member).size();
			std::string_view name = std::meta::extract<lisp::Static_Name>(annot).name;
			ptrdiff_t fptr = (ptrdiff_t)(std::meta::extract<lisp::Value(*)(lisp::Value, lisp::Value)>(member));
			std::println("{} {} {}", name, fptr, count);
		}
	}
}

int main()
{
	lisp::load_environment();
}
