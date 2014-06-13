#include "syscall.h"

#include <sys/files.h>
#include <sys/process.h>
#include <sys/scheduler.h>
#include <sys/time.h>
#include <sys/signal.h>

void* _syscall_funcs[SYSCALL_NUMBER] = {
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
	&sys_pipe2
};


void* syscall_get_function(int sysnum) {
	if(sysnum >= 0 && sysnum < SYSCALL_NUMBER)
		return _syscall_funcs[sysnum];

	return NULL;
}