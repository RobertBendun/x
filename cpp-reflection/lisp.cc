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
			std::string_view name = std::meta::extract<lisp::Static_Name>(annot).name;
			ptrdiff_t fptr = (ptrdiff_t)(std::meta::extract<lisp::Value(*)(lisp::Value, lisp::Value)>(member));
			std::println("{} {}", name, fptr);
		}
	}
}

int main()
{
	lisp::load_environment();
}
