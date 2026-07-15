#include <print>
#include <meta>

void print_structure(auto const& structure)
{
	constexpr auto S = std::meta::remove_cvref(std::meta::type_of(^^structure));

	std::print("{} {{", std::meta::has_identifier(S) ? std::meta::identifier_of(S) : "<anonymous>");

	// TODO: Find what context does what
	constexpr auto ctx = std::meta::access_context::current();

	bool first = true;

	template for (constexpr auto member : std::define_static_array(std::meta::nonstatic_data_members_of(S, ctx))) {
		if (first) {
			first = false;
		} else {
			std::print(", ");
		}
		std::string_view name = std::meta::has_identifier(member) ? std::meta::identifier_of(member) : "<anonymous>";
		std::print(".{} = {}", name, structure.[:member:]);

	}

	std::print("}}");
}

struct dog
{
	std::string name;
	unsigned number_of_toys;
};


int main()
{
	print_structure(dog { .name = "Kiki", .number_of_toys = 42 });
}
