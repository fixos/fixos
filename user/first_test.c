/**
 * This is the first test of a user process (for now it's a  piece of code
 * compiled and linked within the kernel itself, not really an independant
 * binary).
 */

#include <syscalls/syscalls.h>

int usertest_main() __attribute__ (( section(".user.pretext") ));

int usertest_main() {
//	asm volatile ("mov.l ");

	//asm volatile ("trapa #42");

	// force use of stack (check TLB miss from userland)
	asm volatile ("mov.l r0, @-r15;"
				  "add #-4, r15;");

	// try to open "/dev/console"
	int fd;
	fd = open("/dev/console", 123);

	// never return
	while(1);

	return 0;
}
