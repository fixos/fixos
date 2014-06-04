/**
 * Test-purpose user process, using ELF loader.
 */

#include "lib/syscalls.h"

#define write_const(fd, msg) write((fd), (msg), sizeof(msg)-1)


// need division implementation...
/*
void write_int_dec(int fd, int n)
{
	if(n==0) {
		write(fd, "0", 1);
	}
	else {
		int  i;
		int  cpt;
		int start = 0;

		// worst case is '-' with 9 digits, no need for final \0
		char string[10];


		if (n<0) {
			start=1;
			string[0] = '-';
			n *= -1;
		}
		for (i = 1, cpt = 1; n / i >= 10; i *= 10, cpt++);
		for (cpt = start; i; cpt++, i /= 10) string[cpt] = (n / i) % 10 + '0';
		write(fd, string, cpt);
	}
}
*/

void write_int_hex(int fd, unsigned int val) {
	int cpt = 0;
	int i = 0;
	char c;
	char buf[8];

	while((val & (0xF << ((7-cpt)*4) )) == 0 && cpt<8)
			cpt++;
	while(cpt<8) {
		c = (val & (0xF << ((7-cpt)*4) )) >> ((7-cpt)*4);
		if(c < 10)
			buf[i] = c + '0';
		else
			buf[i] = c - 10 + 'A';
		i++;
		cpt++;
	}

	// avoid the empty case if val is 0
	if(i==0) {
		buf[0] = '0';
		i=1;
	}

	write(fd, buf, i);
}


void test_gettimeofday(int fd) {
	struct hr_time prev;
	gettimeofday(&prev, NULL);
	while(1) {
		struct hr_time cur;
		gettimeofday(&cur, NULL);

		if(cur.sec - prev.sec >= 1) {
			prev.sec = cur.sec;
			prev.nano = cur.nano;
			write(fd, "-", 1);
		}
		
	}
}


void print_args(int fd, int argc, char **argv) {
	int argnb;
	int car;
	write_const(fd, "Args:\n");
	for(argnb=0; argnb<argc; argnb++) {
		/*for(car=0; argv[argnb][car] != '\0'; car++);
		write(fd, "\"", 1);
		write(fd, argv[argnb], car);
		write(fd, "\"\n", 2);
		*/
		write_const(fd, "  0x");
		write_int_hex(fd, (unsigned int)argv[argnb]);
		write_const(fd, "\n");
	}
}


// 0 < ibs <= 128 
void test_copy_files(int fdin, int fdout, int ibs) {
	int nbread;
	char buf[128];

	// never return
	while(1) {
		if((nbread = read(fdin, buf, ibs)) > 0) {
			write(fdout, buf, nbread);
		}
	}

}


void test_execve(const char *path) {
	char *exec_argv[4];
	exec_argv[0]="Hello";
	exec_argv[1]="dear execve()";
	exec_argv[2]="arguments";
	exec_argv[3]=NULL;
	execve(path, exec_argv, NULL);
}


// test fork/wait/exit
/*
void test_fork_wait(int fd_serial) {
	pid_t pid;

	pid = fork();
	if(pid == 0) {
		// child process
		while(1) {
			write_const(fd_serial, "And I am his son!\n");
			exit(3);
		}
	}
	else {
		// parent process
		while(1) {
			int status;
			pid_t pid;

			write_const(fd_serial, "I'm the father!\n");

			pid = wait(&status);
			if(pid == -1) {
				write_const(fd_serial, "Wait error.\n");
			}
			else {
				write_const(fd_serial, "Wait ret=");
				write_int_hex(fd_serial, status);

				write_const(fd_serial, ", pid=");
				write_int_dec(fd_serial, pid);
				write_const(fd_serial, "\n");

				pid = fork();
				if(pid == 0) {
					// child process
					write_const(fd_serial, "And I am his son!\n");
					exit(2);
				}
			}
		}
	}
}
*/

void test_simple_signals(int fdout, int fdin, pid_t pid) {
	int state;

	state = 0;
	while(1) {
		char c;
		if(read(fdin, &c, 1) == 1) {
			if(c=='C') {
				write_const(fdout, "Send SIGINT...\n");
				kill(pid, SIGINT);
			}
			else {
				if(state) {
					write_const(fdout, "Try to stop child...\n");
					kill(pid, SIGSTOP);
				}
				else {
					write_const(fdout, "Try to resume child...\n");
					kill(pid, SIGCONT);
				}
				state = !state;
			}
		}
	}
}


void signal_handler(int sig) {
	write_const(1, ">> Received sig 0x");
	write_int_hex(1, sig);
	write_const(1, "\n");
}


void test_handle_sig(int sig) {
	struct sigaction act;

	act.sa_handler = &signal_handler;
	sigemptyset(& act.sa_mask);
	act.sa_flags = 0;

	sigaction(sig, &act, NULL);
}


int usertest_main(int argc, char **argv) {
	// try to open "/dev/console"
	int fd;
	fd = open("/dev/console", 123);

	write_const(fd, "*** Hey! I'm a User process!\n*** I AM ALIVE!\n");

	int fd_serial;
	fd_serial = open("/dev/serial", 456);

	write_const(fd_serial, "*** Hi, dear serial terminal!\n");
	if(argc>0) {
		test_handle_sig(SIGINT);
		print_args(fd_serial, argc, argv);
		test_gettimeofday(fd);
	}

	// never return
	pid_t pid = fork();
	if(pid == 0) {
		write_const(fd_serial, "Child is not dead!\n");
		test_execve("/mnt/smem/test.elf");
		write_const(fd_serial, "execve failed!");
		exit(1);

		//test_copy_files(fd_serial, fd, 1);
	}
	else {
		// father, try to
		//test_copy_files(fd, fd_serial, 1);
		write_const(fd_serial, "Father is not dead!\n");
		while(1) {
			test_simple_signals(fd_serial, fd, pid);
		}
	}


	return 0;
}
