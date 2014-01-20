/**
 * Test-purpose user process, using ELF loader.
 */

#include "lib/syscalls.h"


// global variable to have something in .data and .bss sections
static int chabada = 456;

static int choubidou;


int usertest_main() {
//	asm volatile ("mov.l ");

	//asm volatile ("trapa #42");
	choubidou = 123;

	// force use of stack (check TLB miss from userland)
	asm volatile ("mov.l r0, @-r15;"
				  "add #-4, r15;");

	// try to open "/dev/console"
	int fd;
	fd = open("/dev/console", chabada);
	chabada = 12;

	write(fd, "*** Hey! I'm a User process!\n*** I AM ALIVE!\n", sizeof("*** Hey! I'm a User process!\n*** I AM ALIVE!\n")-1);

	int fd_serial;
	fd_serial = open("/dev/serial", chabada);

	write(fd_serial, "*** Hi, dear serial terminal!\n", sizeof("*** Hi, dear serial terminal!\n")-1);

	/*
	int nbread;
	char buf[128];
	// never return
	while(1) {
		// write on /dev/console each byte received from /dev/serial
		if((nbread = read(fd_serial, buf, 128)) > 0) {
			write(fd_serial, buf, nbread);
		}


	}*/

	pid_t pid = fork();
	if(pid == 0) {
		// child process
		while(1)
			write(fd_serial, "And I am his son!\n", sizeof("And I am his son!\n")-1);
	}
	else {
		// parent process
		while(1)
			write(fd_serial, "I'm the father!\n", sizeof("I'm the father!\n")-1);
	}

	return 0;
}
