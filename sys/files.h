#ifndef _SYS_FILES_H
#define _SYS_FILES_H

/**
 * High level, process-related, file access.
 * Designed to be used as syscall target...
 */

#include <utils/types.h>


// FIXME add a "mode" argument
int sys_open(const char *name, int flags);

ssize_t sys_read(int fd, char *dest, int nb);

ssize_t sys_write(int fd, const char *source, int nb);

int sys_ioctl(int fd, int request, void *arg);


// pipe() is implemented by calling pipe2(..., 0)
int sys_pipe2(int pipefd[2], int flags);

#endif //_SYS_FILES_H
