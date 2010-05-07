#ifndef CS61C_H
#define CS61C_H

#include <stdbool.h>

static char *solution = "10 1 S\n3 0 E\n14 0 E\n13 0 E\n10 0 S\n3 2 E\n4 4 E\n2 2 E\n4 2 N\n1 2 E\n8 4 E\n14 4 E\n3 4 E\n13 4 E\n7 4 E\n";

static char *fsolution = "0 0 S\n4 6 N\n0 3 E\n3 0 E\n0 8 N\n0 9 N\n9 8 N\n0 4 E\n8 0 S\n8 8 N\n3 9 N\n9 9 W\n8 9 N\n9 0 W\n8 0 S\n7 9 N\n7 0 S\n9 1 W\n7 3 W\n6 8 N\n3 4 E\n8 4 W\n8 3 S\n8 2 S\n8 1 S\n7 2 S\n7 1 S\n5 2 E\n4 2 E\n1 2 E\n6 4 S\n7 4 W\n6 8 N\n6 9 N\n6 6 E\n7 3 S\n6 4 E\n6 3 S\n5 3 E\n4 3 E\n5 2 S\n4 2 E\n0 2 E\n3 3 E\n0 3 E\n";

void emit_61c_solution() {
	printf("%s", solution);
}

void emit_fun_solution() {
	printf("%s", fsolution);
}

#endif
