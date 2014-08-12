#ifndef _SYSCALLS_SYSCALLS_H
#define _SYSCALLS_SYSCALLS_H

/**
 * List of syscalls number and prototypes.
 * Depending the architecture, syscall are called by different way.
 * On SH3, FiXos uses assembly instruction "trapa #<syscall ID>".
 */

#include <fixos/types.h>
#include <fixos/syscalls.h>
#include <fixos/signal.h>
#include <fixos/stat.h>
#include <fixos/dirent.h>


extern int open(const char *file, int mode);

extern ssize_t read(int fd, char *dest, size_t nb);

extern ssize_t write(int fd, const char *source, size_t nb);

extern pid_t fork(); 

extern void exit(int status);

extern pid_t getpid();

extern pid_t getppid();

extern pid_t wait(int *status);

extern int execve(const char *filename, char *const argv[], char *const envp[]);

struct timezone;
extern int gettimeofday(struct hr_time *tv, struct timezone *tz);

extern clock_t times(struct tms *buf); 


extern int sigaction(int sig, const struct sigaction* act, struct sigaction* oact);

extern int kill(pid_t pid, int sig);

extern int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

extern int pipe2(int pipefd[2], int flags);

extern int ioctl(int fd, int request, void *arg);
 
extern int dynbind(const char *symbol, void **dest);

extern void* sbrk(int incr);


extern int sysctl_read(const int *name, size_t name_len, void *buf, size_t *len);

extern int sysctl_write(const int *name, size_t name_len,
		const void *buf, size_t *len);

extern int nanosleep(const struct hr_time *req, struct hr_time *rem);

extern int sysctl_mibname(const char *strname, int *name, int *name_len);

extern int fstat(int fd, struct stat *buf);

extern int stat(const char *path, struct stat *buf);

extern int getdents(int fd, struct fixos_dirent *buf, size_t len);

extern int close(int fd);

extern int chdir(const char *path);

extern int fchdir(int fd);

extern int setpgid(pid_t pid, pid_t pgid);

extern pid_t getpgid(pid_t pid);

extern int dup(int oldfd);

extern int dup2(int oldfd, int newfd);

#endif //_SYSCALLS_SYSCALLS_H
