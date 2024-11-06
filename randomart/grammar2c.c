#include <assert.h>
#include <stdio.h>
#include <stdio.h>
#include <libgen.h>
#include <time.h>

#define SV_IMPLEMENTATION
#include "sv.h"

#define NOB_IMPLEMENTATION
#include "nob.h"

// Nonterminals have count > 0, terminals count = 0
typedef struct Expr {
	String_View symbol;
	struct Expr *items;
	size_t count, capacity;
} Expr;

typedef struct Branch {
	Expr expr;
	unsigned long prob_num, prob_den;
} Branch;

typedef struct Rule {
	String_View name;
	Branch *items;
	size_t count, capacity;
} Rule;

typedef struct Grammar {
	Rule *items;
	size_t count, capacity;
} Grammar;

bool parse_grammar(String_View *source_file, Grammar *grammar);
bool parse_rule_body(String_View *source_file, Rule *rule);
bool parse_branch(String_View *source_file, Branch *branch);
bool parse_function_or_constant(String_View *source_file, Expr *expr);

bool randomart(FILE *output, Grammar g, int depth, String_View rule);

int main(int argc, char **argv)
{
	srand(time(NULL));

	if (argc < 3) {
		fprintf(stderr, "usage: %s <grammar source> <C output>\n", basename(argv[0]));
		return 2;
	}

	char const *const input_path = argv[1];
	char const *const output_path = argv[2];

	Nob_String_Builder sb = {};
	if (!nob_read_entire_file(input_path, &sb)) {
		fprintf(stderr, "failed to read input file: %s", input_path);
		return 1;
	}
	Nob_String_View nob_source_file = nob_sb_to_sv(sb);
	String_View source_file = { .data = nob_source_file.data, .count = nob_source_file.count };

	Grammar grammar = {};
	if (!parse_grammar(&source_file, &grammar)) return 1;

	FILE *output = fopen(output_path, "w");
	assert(output && "TODO: nice error message");

	if (!randomart(output, grammar, 10, SV("E"))) return 1;

	return 0;
}

bool parse_grammar(String_View *source_file, Grammar *grammar)
{
	for (;;) {
		Rule rule = {};
		rule.name = sv_trim(sv_chop_by_sv(source_file, SV(" ::= ")));
		if (source_file->count == 0 || rule.name.count == 0) {
			return true;
		}
		if (!parse_rule_body(source_file, &rule)) return false;
		nob_da_append(grammar, rule);
	}
	return true;
}

bool parse_rule_body(String_View *source_file, Rule *rule)
{
	assert(source_file->count > 0);

	for (;;) {
		Branch branch = {};
		if (!parse_branch(source_file, &branch)) return false;
		nob_da_append(rule, branch);

		*source_file = sv_trim_left(*source_file);
		if (sv_starts_with(*source_file, SV("|"))) {
			sv_chop_left(source_file, 1);
			*source_file = sv_trim_left(*source_file);
		} else {
			break;
		}
	}

	return true;
}

bool valid_symbol_character(char c)
{
	return isalnum(c) || c == '_' || c == '-' || c == '.';
}

bool parse_branch(String_View *source_file, Branch *branch)
{
	if (!parse_function_or_constant(source_file, &branch->expr)) return false;

	*source_file = sv_trim_left(*source_file);
	branch->prob_num  = sv_chop_u64(source_file);
	*source_file = sv_trim_left(*source_file);

	if (sv_starts_with(*source_file, SV("/"))) {
		sv_chop_left(source_file, 1);
		branch->prob_den = sv_chop_u64(source_file);
	} else {
		branch->prob_den = 1;
	}

	return true;
}

bool parse_function_or_constant(String_View *source_file, Expr *expr)
{
	assert(source_file->count > 0);
	*source_file = sv_trim_left(*source_file);

	size_t i;
	for (i = 0;
			 i < source_file->count && valid_symbol_character(source_file->data[i]);
			 i++)
	{
	}

	expr->symbol = sv_chop_left(source_file, i);

	*source_file = sv_trim_left(*source_file);

	if (sv_starts_with(*source_file, SV("("))) {
		sv_chop_left(source_file, 1);

		for (;;) {
			Expr subexpr = {};
			if (!parse_function_or_constant(source_file, &subexpr)) return false;
			nob_da_append(expr, subexpr);
			*source_file = sv_trim_left(*source_file);

			if (sv_starts_with(*source_file, SV(","))) {
				sv_chop_left(source_file, 1);
				*source_file = sv_trim_left(*source_file);
			} else if (sv_starts_with(*source_file, SV(")"))) {
				sv_chop_left(source_file, 1);
				break;
			} else {
				// TODO: Report line number and character
				fprintf(stderr, "failed to parse: opening ( without closing )\n");
				return false;
			}
		}
	}

	return true;
}

void randomart_render_expr(FILE *output, Grammar g, int depth, Expr *expr)
{
	if (expr->count == 0) {
		if (sv_eq(expr->symbol, SV("rand"))) {
			fprintf(output, "%f", (float)rand() / RAND_MAX);
			return;
		}

		for (size_t i = 0; i < g.count; ++i) {
			if (sv_eq(g.items[i].name, expr->symbol)) {
				if (randomart(output, g, depth-1, expr->symbol)) {
					return;
				} else {
					break;
				}
			}
		}
		fprintf(output, SV_Fmt, SV_Arg(expr->symbol));
		return;
	}

	fprintf(output, SV_Fmt "(", SV_Arg(expr->symbol));
	for (size_t i = 0; i < expr->count; ++i) {
		randomart_render_expr(output, g, depth-1, &expr->items[i]);
		if (i+1 < expr->count) {
			fprintf(output, ", ");
		} else {
			fprintf(output, ")");
		}
	}
}

bool randomart(FILE *output, Grammar g, int depth, String_View rule_name)
{
	Rule *rule = NULL;
	for (size_t i = 0; i < g.count; ++i) {
		if (sv_eq(g.items[i].name, rule_name)) {
			rule = &g.items[i];
			break;
		}
	}
	if (rule == NULL) {
		fprintf(stderr, "Failed to found rule: \"" SV_Fmt "\"\n", SV_Arg(rule_name));
		fprintf(stderr, "Available: ");
		for (size_t i = 0; i < g.count; ++i) {
			fprintf(stderr, "\""SV_Fmt"\" ", SV_Arg(g.items[i].name));
		}
		fprintf(stderr, "\n");
		return false;
	}

	Branch *branch = &rule->items[0];
	if (depth > 0) {
		// TODO: Pick with probability
		branch = &rule->items[rand()%rule->count];
	}

	randomart_render_expr(output, g, depth, &branch->expr);
	return true;
}
