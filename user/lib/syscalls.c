#include <syscalls.h>
#include "arch/syscall.h"


_SYSCALL_(open,			SYSCALL_OPEN)
_SYSCALL_(read,			SYSCALL_READ)
_SYSCALL_(write,		SYSCALL_WRITE)
_SYSCALL_(fork,			SYSCALL_FORK)
_SYSCALL_(exit,			SYSCALL_EXIT)
_SYSCALL_(getpid,		SYSCALL_GETPID)
_SYSCALL_(getppid,		SYSCALL_GETPPID)
_SYSCALL_(wait,			SYSCALL_WAIT)
_SYSCALL_(execve,		SYSCALL_EXECVE)
_SYSCALL_(gettimeofday,	SYSCALL_GETTIMEOFDAY)
_SYSCALL_(times,		SYSCALL_TIMES)
_SYSCALL_(sigaction,	SYSCALL_SIGACTION)
_SYSCALL_(kill,			SYSCALL_KILL)
_SYSCALL_(sigprocmask,	SYSCALL_SIGPROCMASK)
_SYSCALL_(pipe2,		SYSCALL_PIPE2)
_SYSCALL_(ioctl,		SYSCALL_IOCTL)
_SYSCALL_(dynbind,		SYSCALL_DYNBIND)
