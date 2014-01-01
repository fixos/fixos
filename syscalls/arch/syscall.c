#include "syscall.h"

#include <sys/files.h>

void* _syscall_funcs[SYSCALL_NUMBER] = {
	NULL,
	&sys_open,
	&sys_read,
	&sys_write
};


void* syscall_get_function(int sysnum) {
	if(sysnum >= 0 && sysnum < SYSCALL_NUMBER)
		return _syscall_funcs[sysnum];

	return NULL;
}
