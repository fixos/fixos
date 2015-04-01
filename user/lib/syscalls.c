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

#include <fixos/syscalls.h>
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
_SYSCALL_(sbrk,			SYSCALL_SBRK)
_SYSCALL_(sysctl_read,	SYSCALL_SYSCTL_READ)
_SYSCALL_(sysctl_write,	SYSCALL_SYSCTL_WRITE)
_SYSCALL_(nanosleep,	SYSCALL_NANOSLEEP)
_SYSCALL_(sysctl_mibname,	SYSCALL_SYSCTL_MIBNAME)
_SYSCALL_(fstat,		SYSCALL_FSTAT)
_SYSCALL_(stat,			SYSCALL_STAT)
_SYSCALL_(getdents,		SYSCALL_GETDENTS)
_SYSCALL_(close,		SYSCALL_CLOSE)
_SYSCALL_(chdir,		SYSCALL_CHDIR)
_SYSCALL_(fchdir,		SYSCALL_FCHDIR)
_SYSCALL_(setpgid,		SYSCALL_SETPGID)
_SYSCALL_(getpgid,		SYSCALL_GETPGID)
_SYSCALL_(dup,			SYSCALL_DUP)
_SYSCALL_(dup2,			SYSCALL_DUP2)
