#ifndef _FIXOS_INTERFACE_SYSCALLS_H
#define _FIXOS_INTERFACE_SYSCALLS_H

/**
 * Syscall numbers and symbolic names.
 */

//extern int open(const char *file, int mode);
#define SYSCALL_OPEN	1

//extern ssize_t read(int fd, char *dest, size_t nb);
#define SYSCALL_READ	2

//extern ssize_t write(int fd, const char *source, size_t nb);
#define SYSCALL_WRITE	3

//extern pid_t fork()
#define SYSCALL_FORK	4


//extern void exit(int status)
#define SYSCALL_EXIT	5

//extern pid_t getpid()
#define SYSCALL_GETPID	6

//extern pid_t getppid()
#define SYSCALL_GETPPID	7

//extern pid_t wait(int *status)
#define SYSCALL_WAIT	8

//extern int execve(const char *filename, char *const argv[], char *const envp[])
#define SYSCALL_EXECVE	9

//extern int gettimeofday(struct timeval *tv, struct timezone *tz)
#define SYSCALL_GETTIMEOFDAY	10

//extern clock_t times(struct tms *buf); 
#define SYSCALL_TIMES	11

//extern int sigaction(int sig, const struct sigactionœ* act, struct sigaction* oact);
#define SYSCALL_SIGACTION	12

//extern int kill(pid_t pid, int sig);
#define SYSCALL_KILL		13

//extern int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
#define SYSCALL_SIGPROCMASK	14

//extern int sigreturn();
#define SYSCALL_SIGRETURN	15

//extern int pipe2(int pipefd[2], int flags);
#define SYSCALL_PIPE2		16

//extern int ioctl(int fd, int request, void *arg);
#define SYSCALL_IOCTL		17


//extern int dynbind(const char *symbol, void **dest);
#define SYSCALL_DYNBIND		18


// extern void *sbrk(int incr);
#define SYSCALL_SBRK		19

// extern int sysctl_read(const int *name, size_t name_len, void *buf,
//			size_t *len);
#define SYSCALL_SYSCTL_READ	20

// extern int sysctl_write(const int *name, size_t name_len,
//			const void *buf, size_t *len);
#define SYSCALL_SYSCTL_WRITE	21

// extern int nanosleep(const struct hr_time *req, struct hr_time *rem);
#define SYSCALL_NANOSLEEP	22

// extern int sysctl_mibname(const char *strname, int *name, int *name_len);
#define SYSCALL_SYSCTL_MIBNAME	23

// extern int lseek(int fd, off_t offset, int whence);
#define SYSCALL_LSEEK		24

// extern int fstat(int fd, struct stat *buf);
#define SYSCALL_FSTAT		25

// extern int stat(const char *path, struct stat *buf);
#define SYSCALL_STAT		26

// extern int getdents(int fd, struct fixos_dirent *buf, size_t len);
#define SYSCALL_GETDENTS	27

// extern int close(int fd);
#define SYSCALL_CLOSE		28

// extern int chdir(const char *path);
#define SYSCALL_CHDIR		29

// extern int fchdir(int fd);
#define SYSCALL_FCHDIR		30

// extern int setpgid(pid_t pid, pid_t pgid);
#define SYSCALL_SETPGID		31

// extern pid_t getpgid(pid_t pid);
#define SYSCALL_GETPGID		32

//extern int dup(int oldfd);
#define SYSCALL_DUP			33

//extern int dup2(int oldfd, int newfd);
#define SYSCALL_DUP2		34

#endif //_FIXOS_INTERFACE_SYSCALLS_H
