#include "lib/syscalls.h"

#define write_const(fd, msg) write((fd), (msg), sizeof(msg)-1)

void *dynld_solvename(const char *symbol) {
	int i;

	for(i=0; symbol[i] != '\0'; i++);
	/*write_const(0, "Dynamic solving symbol '");
	write(0, symbol, i);
	write_const(0, "'\n");
	*/

	void *ret;
	if(dynbind(symbol, &ret) == 0)
		return ret;

	return NULL;
}

