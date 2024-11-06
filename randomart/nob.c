#define NOB_IMPLEMENTATION
#include "nob.h"

int main(int argc, char **argv)
{
	NOB_GO_REBUILD_URSELF(argc, argv);

	Nob_Cmd cmd = {};

	nob_cmd_append(&cmd, "cc", "-o", "grammar2c", "grammar2c.c", "-Wall", "-Wextra", "-ggdb", "-lm");
	if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

	nob_cmd_append(&cmd, "./grammar2c", "simple.txt", "simple.c");
	if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

	nob_cmd_append(&cmd, "cc", "-o", "generate", "generate.c", "-Wall", "-Wextra", "-ggdb", "-lm");
	if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

	nob_cmd_append(&cmd, "./generate", "output.png");
	if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

	return 0;
}
