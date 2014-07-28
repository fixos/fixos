/**
 * Test-purpose user process, using ELF loader.
 */

#include "lib/syscalls.h"
#include <display.h>
#include <fxkeyboard.h>
#include <fcntl.h>
#include <sysctl.h>
#include <process.h>
#include <dirent.h>
#include "sharedtest/test.h"

#define write_const(fd, msg) write((fd), (msg), sizeof(msg)-1)


// need division implementation...
static void write_int_dec(int fd, int n)
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

static void write_int_hex(int fd, unsigned int val) {
	int cpt = 0;
	int i = 0;
	char c;
	char buf[8];
	static const char hex_char[] = "0123456789ABCDEF"; 

	while((val & (0xF << ((7-cpt)*4) )) == 0 && cpt<8)
			cpt++;
	while(cpt<8) {
		c = (val & (0xF << ((7-cpt)*4) )) >> ((7-cpt)*4);
		buf[i] = hex_char[(int)c];
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


static void write_int_oct(int fd, unsigned int val) {
	int cpt = 0;
	int i = 0;
	char c;
	char buf[11];

	while((val & (0x7 << ((10-cpt)*3) )) == 0 && cpt<11)
			cpt++;
	while(cpt<11) {
		c = (val >> ((10-cpt)*3)) & 0x7;
		buf[i] = c + '0';
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


static void test_gettimeofday(int fd) {
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


static void print_args(int fd, int argc, char **argv) {
	int argnb;
	int car;
	write_const(fd, "Args:\n");
	for(argnb=0; argnb<argc; argnb++) {
		for(car=0; argv[argnb][car] != '\0'; car++);
		write(fd, "\"", 1);
		write(fd, argv[argnb], car);
		write(fd, "\"\n", 2);
		
		/*
		write_const(fd, "  0x");
		write_int_hex(fd, (unsigned int)argv[argnb]);
		write_const(fd, "\n");
		*/
	}
}


// 0 < ibs <= 128 
static void test_copy_files(int fdin, int fdout, int ibs) {
	int nbread;
	char buf[128];

	// never return
	while(1) {
		if((nbread = read(fdin, buf, ibs)) > 0) {
			write(fdout, buf, nbread);
		}
	}

}


static void test_execve(const char *path) {
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
/*
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


// try to create a pipe, then to fork
// the parent process will copy each byte in fdin to pipe input, and
// the child process will copy each byte in pipe output to fdout
void test_fork_pipe(int fdin, int fdout) {
	pid_t pid;
	int pipefd[2];
	char buf;

	if(pipe2(pipefd, 0) != 0) {
		write_const(fdout, "pipe failed!\n");
		exit(1);
	}

	pid = fork();
	if(pid == 0) {
		// child process
		write_const(fdout, "And I am his son!\n");
		while(1) {
			if(read(pipefd[1], &buf, 1) > 0)
				write(fdout, &buf, 1);
		}
	}
	else {
		// parent process
		write_const(fdout, "I'm the father!\n");
		while(1) {
			if(read(fdin, &buf, 1) > 0)
				write(pipefd[0], &buf, 1);
		}
	}
}
*/


// test direct display using /dev/display device
static int test_display(int fdout) {
	int disp;
	struct display_info info;
	char *vram;

	disp = open("/dev/display", O_RDWR);

	ioctl(disp, DISPCTL_INFO, &info);
	write_const(fdout, "info :\n  w=0x");
	write_int_hex(fdout, info.width);
	write_const(fdout, "\n  h=0x");
	write_int_hex(fdout, info.height);
	write_const(fdout, "\n  size=0x");
	write_int_hex(fdout, info.vram_size);
	write_const(fdout, "\n");


	// simple "FPS" test
	if(ioctl(disp, DISPCTL_SETMODE, (void*)(DISPMODE_ACTIVATE)) == 0) {
		if(ioctl(disp, DISPCTL_MAPVRAM, &vram) == 0) {
			struct hr_time prev;
			unsigned char val = 0xFF;
			int i;
			int fps = 0;

			gettimeofday(&prev, NULL);
			while(1) {
				struct hr_time cur;
				for(i=0; i<info.vram_size; i++)
					vram[i] = val;

				if(val == 0x00)
					val = 0xFF;
				else
					val = 0x00;

				ioctl(disp, DISPCTL_DISPLAY, NULL);

				fps++;

				gettimeofday(&cur, NULL);
				if(cur.sec - prev.sec >= 1) {
					prev.sec = cur.sec;
					prev.nano = cur.nano;
					write_const(fdout, "fps: 0x");
					write_int_hex(fdout, fps);
					write_const(fdout, "\n");
					fps = 0;
				}
		
			}
		}
	}


	while(1);
	return 0;
}


static void test_sharedlib() {
	name("rst call");
	name("Second call");
	tada(10);
}


static void test_sbrk(int fdout) {
	int *cur_sbrk;
	int i;
	
	cur_sbrk = sbrk(0);
	write_const(fdout, "Current brk=");
	write_int_hex(fdout, (unsigned int)cur_sbrk);
	write_const(fdout, "\n");

	for(i=0; i<1234; i++) {
		int *cur;
		cur = sbrk(sizeof(int));
		*cur = i;
	}

	int ok = 1;
	for(i=0; i<1234; i++) {
		if(cur_sbrk[i] != i) ok = 0;
	}

	if(ok) {
		write_const(fdout, "Alloc seems to work\n");
	}
	else {
		write_const(fdout, "Alloc problem!\n");
	}

	cur_sbrk = sbrk(0);
	write_const(fdout, "After alloc brk=");
	write_int_hex(fdout, (unsigned int)cur_sbrk);
	write_const(fdout, "\n");

	cur_sbrk = sbrk(- sizeof(int) * 1234);

	cur_sbrk = sbrk(0);
	write_const(fdout, "After freeing brk=");
	write_int_hex(fdout, (unsigned int)cur_sbrk);
	write_const(fdout, "\n");
}

static void test_fxkeyboard(int fdout) {
	int kdb;
	struct fxkey_event evt;

	kdb = open("/dev/keyboard", O_RDONLY | O_NONBLOCK);
	while(1) {
		if(read(kdb, (char*)&evt, sizeof(evt)) == sizeof(evt)) {
			if(evt.event == K_EVENT_PRESSED)
				write_const(fdout, "+0x");
			else
				write_const(fdout, "-0x");

			write_int_hex(fdout, evt.key);
			write_const(fdout, "\n");
		}
		else {
			write_const(fdout, "#");
		}
	}
}

static void test_sysctl(int fdout) {
	char buf[20];
	int mib_ostype[2] = {CTL_KERN, KERN_OSTYPE};
	size_t len;

	len = 20;
	if(sysctl_read(mib_ostype, 2, buf, &len) == 0) {
		int i;
		write_const(fdout, "kern.ostype = '");
		
		for(i=0; buf[i] != '\0'; i++);
		write(fdout, buf, i);

		write_const(fdout, "' len=0x");
		write_int_hex(fdout, len);
		write_const(fdout, "\n");
	}
	else {
		write_const(fdout, "kern.ostype (error), len=0x");
		write_int_hex(fdout, len);
		write_const(fdout, "\n");
	}

	while(1);
}


static struct proc_uinfo _proc[20];

static void test_print_all_proc(int fdout) {
//	char line[100];
	int mib[3] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};

	size_t size = 0;
/*
	sysctl_read(mib, 3, NULL, &size);

	// allocate the needed size
	static struct proc_uinfo *proc = NULL;
	proc = realloc(proc, size);
*/
	size = sizeof(_proc);
	if(sysctl_read(mib, 3, _proc, &size) == 0) {
		int i;
		int nb;

		nb = size/sizeof(*_proc);
		for(i=0; i<nb; i++) {
			write_const(fdout, "pid ");
			write_int_dec(fdout, _proc[i].pid);

			write_const(fdout, " : cpu=");
			write_int_dec(fdout, _proc[i].cpu_usage/100);

			write_const(fdout, "%, {");
			write_int_dec(fdout, _proc[i].uticks);

			write_const(fdout, ",");
			write_int_dec(fdout, _proc[i].kticks);

			write_const(fdout, "}\n");
/*			
			int nbchar = snprintf(line, 100, "pid %d : cpu=%d%%, {%d,%d}\n",
					_proc[i].pid, _proc[i].cpu_usage/100, _proc[i].uticks,
					_proc[i].kticks);
			if(nbchar > 0) {
				write(fdout, line, nbchar);
			}
*/
		}
	}
	else {
		write_const(fdout, "sysctl() error, with size=");
		write_int_dec(fdout, size);
		write_const(fdout, "\n");
	}
}

static void test_sysctl_proc(int fdout) {
	struct hr_time val = { .sec=0, .nano=500000000 };

	while(1) {
		test_print_all_proc(fdout);
		nanosleep(&val, NULL);
	}
}


static int sysctl_rdbyname(const char *strname, void *oldbuf, size_t *oldlen) {
	int name[5];
	int name_len;

	// try to find the integer name
	name_len = 5;
	if(sysctl_mibname(strname, name, &name_len) == 0) {
		return sysctl_read(name, name_len, oldbuf, oldlen);
	}
	write_const(0, "{???}");
	return 1;
}

static void test_print_sysinfo(int fdout) {
	char infobuf[64];
	size_t size;

	size=64;
	if(!sysctl_rdbyname("kern.ostype", infobuf, &size))
		write(fdout, infobuf, size-1);
	write_const(fdout, " ");

	size=64;
	if(!sysctl_rdbyname("kern.osrelease", infobuf, &size))
		write(fdout, infobuf, size-1);
	write_const(fdout, " (");

	size=64;
	if(!sysctl_rdbyname("kern.osbuilddate", infobuf, &size))
		write(fdout, infobuf, size-1);
	write_const(fdout, ") ");

	size=64;
	if(!sysctl_rdbyname("hw.machine", infobuf, &size))
		write(fdout, infobuf, size-1);
	write_const(fdout, "\n");
}

static void test_print_stat(int fdout, const char *name, struct stat *s) {
	int size;
	
	for(size=0; name[size]!='\0'; size++);

	// like ls -l mode/type display (3*3 chars for mode and 1 char for type)
	char mode[10];
	switch(s->st_mode & S_IFMT) {
		case S_IFDIR:
			mode[0] = 'd'; break;
		case S_IFCHR:
			mode[0] = 'c'; break;
		case S_IFREG:
			mode[0] = '-'; break;
		default:
			mode[0] = '?';
	}
	mode[1] = s->st_mode & S_IRUSR ? 'r' : '-';
	mode[2] = s->st_mode & S_IWUSR ? 'w' : '-';
	mode[3] = s->st_mode & S_IXUSR ? 'x' : '-';
	mode[4] = '-';
	mode[5] = '-';
	mode[6] = '-';
	mode[7] = '-';
	mode[8] = '-';
	mode[9] = '-';

	write(fdout, mode, 10);

	write_const(fdout, " (");
	if((s->st_mode & S_IFMT) == S_IFCHR) {
		write_int_dec(fdout, major(s->st_rdev));
		write_const(fdout, ", ");
		write_int_dec(fdout, minor(s->st_rdev));
	}
	else {
		write_int_dec(fdout, s->st_size);
	}
	write_const(fdout, ") ");
	write(fdout, name, size);
	write_const(fdout, "\n");
}


static void test_stat(int fdout) {
	struct stat s;

	if(stat("/mnt/smem/test.elf", &s) == 0) {
		test_print_stat(fdout, "/mnt/smem/test.elf", &s);
	}
	if(fstat(fdout, &s) == 0) {
		test_print_stat(fdout, "{fdout}", &s);
	}
}


static char dirent_buf[512];

static void test_dirent(int fdout, const char *path) {
	struct fixos_dirent *dirent;
	int len;
	int dirfd;

	dirfd = open(path, O_RDONLY);

	write_const(fdout, "Directory entries in '");
	int i;
	for(i=0; path[i]!='\0'; i++);
	write(fdout, path, i);
	write_const(fdout, "' :\n");

	do {
		dirent = (struct fixos_dirent *)dirent_buf;
		len = getdents(dirfd, dirent, 512);

		if(len>0) {
			int pos;

			// first 512 bytes of entries in current directory
			pos = 0;
			while(pos<len) {
				for(i=0; dirent->d_name[i]!='\0'; i++);
				write(fdout, dirent->d_name, i);
				write_const(fdout, " (@0x");
				write_int_hex(fdout, dirent->d_ino);
				write_const(fdout, ")\n");
				pos += dirent->d_off;
				dirent = (struct fixos_dirent *)((void*)dirent + dirent->d_off);
			}
			write_const(fdout, "[total lenght = ");
			write_int_dec(fdout, len);
			write_const(fdout, "]\n");
		}
	} while(len >= 0);

	if(dirfd >= 0)
		close(dirfd);
}


//char nawak[64*1024] = {};

int usertest_main(int argc, char **argv) {
	// try to open "/dev/console"
	int fd;
	fd = open("/dev/console", O_RDWR);

	write_const(fd, "*** Hey! I'm a User process!\n*** I AM ALIVE!\n");

	int fd_serial;
	fd_serial = open("/dev/serial", O_RDWR);

	write_const(fd_serial, "*** Hi, dear serial terminal!\n");

	int tty1 = open("/dev/tty1", O_RDWR);

	write_const(tty1, "Welcome!\n");
	test_print_sysinfo(tty1);


	//test_sharedlib();

	//test_sbrk(fd);
	//test_fxkeyboard(fd);
	//test_sysctl(fd);

	pid_t pid;
	pid = fork();
	if(pid) {
		int tty2 = open("/dev/tty2", O_RDWR);

		write_const(tty2, "Welcome!\n");
		test_print_sysinfo(tty2);
		test_stat(fd);

		test_dirent(fd, "/");
		test_dirent(fd, "/mnt/smem");

		write_const(tty1, "I'm writing on tty1.\n");
		write_const(tty2, "I'm writing on tty2.\n");

	//	wait(NULL);

		test_copy_files(tty2, tty1, 1);
	}
	else {
	//	int i;
		//for(i=0; i<64*1024; i++) 
		//	nawak[64*1024-i-1] = nawak[i];

		//write_const(fd, "Child is dying...\n");
		//exit(1);

		test_sysctl_proc(fd);

		//test_copy_files(tty1, fd_serial, 1);
	}

	//test_display(fd_serial);
	
	//test_fork_pipe(fd, fd_serial);
/*
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
*/

	exit(0);
	return 0;
}
