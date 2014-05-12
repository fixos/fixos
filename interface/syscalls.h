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


#endif //_FIXOS_INTERFACE_SYSCALLS_H
