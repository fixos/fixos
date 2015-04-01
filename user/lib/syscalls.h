/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include <fixos/time.h>
#include <fixos/times.h>

#define NULL	__KERNEL_NULL

// fix types definitions
typedef __kernel_size_t		size_t;
typedef __kernel_ssize_t	ssize_t;
typedef __kernel_off_t		off_t;


// classic plateform-independant types
typedef __kernel_uint32		uint32;
typedef __kernel_int32		int32;
typedef __kernel_uint16 	uint16;
typedef __kernel_int16		int16;
typedef __kernel_uint8		uint8;
typedef __kernel_int8		int8;

// Process IDendifier
typedef __kernel_pid_t		pid_t;


// devices identifier and macros for major/minor decomposition
typedef __kernel_dev_t		dev_t;
#define major(x)			__kernel_major(x)
#define minor(x)			__kernel_minor(x)
#define makedev(maj, min)	__kernel_makedev(maj, min)

typedef __kernel_ino_t		ino_t;
typedef __kernel_mode_t		mode_t;

// time representation
typedef __kernel_clock_t	clock_t;
typedef __kernel_time_t		time_t;



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
extern int gettimeofday(struct timespec *tv, struct timezone *tz);

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

extern int nanosleep(const struct timespec *req, struct timespec *rem);

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
