#include "trap.h"

char buf[128];

int main() {
	printf(buf, "%s", "Hello world!\n");
	nemu_assert(strcmp(buf, "Hello world!\n") == 0);

	return 0;
}
