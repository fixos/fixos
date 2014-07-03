#include <lib/syscalls.h>
#include "test.h"
#include "shared_def.h"

#define write_const(fd, msg) write((fd), (msg), sizeof(msg)-1)

int abc = 2;
int __exported def = 1;

int myfunc2() {
	myfunc(458);
	write_const(0, "myfunc2()\n");
	return 1;
}

int __exported myfunc(int a) {
	abc = a;
	def = -a;
	name("test myfunc");
	return -1;
}

int __exported tada(int a) {
	abc = -a;
	def = a;
	myfunc2();
	return -5;
}

int __exported name(const char * str) {
	int i;

	for(i=0; str[i] != '\0'; i++);
	write_const(0, "shared name(\"");
	write(0, str, i);
	write_const(0, "\")\n");

	return str == (void*)0;
}

