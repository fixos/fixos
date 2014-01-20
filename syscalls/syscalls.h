#ifndef _SYSCALLS_SYSCALLS_H
#define _SYSCALLS_SYSCALLS_H

/**
 * List of syscalls number and prototypes.
 * Depending the architecture, syscall are called by different way.
 * On SH3, FiXos uses assembly instruction "trapa #<syscall ID>".
 */

#include <utils/types.h>


//extern int open(const char *file, int mode);
#define SYSCALL_OPEN	1

//extern ssize_t read(int fd, char *dest, size_t nb);
#define SYSCALL_READ	2

//extern ssize_t write(int fd, const char *source, size_t nb);
#define SYSCALL_WRITE	3

//extern pid_t fork()
#define SYSCALL_FORK	4

#endif //_SYSCALLS_SYSCALLS_H
