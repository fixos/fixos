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

#include "syscall.h"

#include <sys/files.h>
#include <sys/process.h>
#include <sys/scheduler.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/sysctl.h>

#include <loader/elfloader/loader.h>

void* const _syscall_funcs[SYSCALL_NUMBER] = {
	NULL,
	&sys_open,
	&sys_read,
	&sys_write,
	&sys_fork,
	&sys_exit,
	&sys_getpid,
	&sys_getppid,
	&sys_wait,
	&sys_execve,
	&sys_gettimeofday,
	&sys_times,
	&sys_sigaction,
	&sys_kill,
	&sys_sigprocmask,
	&sys_sigreturn,
	&sys_pipe2,
	&sys_ioctl,
	&sys_dynbind,
	&sys_sbrk,
	&sys_sysctl_read,
	&sys_sysctl_write,
	&sys_nanosleep,
	&sys_sysctl_mibname,
	&sys_lseek,
	&sys_fstat,
	&sys_stat,
	&sys_getdents,
	&sys_close,
	&sys_chdir,
	&sys_fchdir,
	&sys_setpgid,
	&sys_getpgid,
	&sys_dup,
	&sys_dup2
};


void* syscall_get_function(int sysnum) {
	if(sysnum >= 0 && sysnum < SYSCALL_NUMBER)
		return _syscall_funcs[sysnum];

	return NULL;
}
