#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

/**
 * List of syscalls number and prototypes.
 * Depending the architecture, syscall are called by different way.
 * On SH3, FiXos uses assembly instruction "trapa #<syscall ID>".
 */

#include <interface/syscalls.h>



#define SYSCALL_NUMBER		17

// for kernel-part syscall handling
extern void* _syscall_funcs[];

/**
 * Returns the syscall function associated to sysnum, or NULL if the given
 * syscall is not implemented.
 */
void* syscall_get_function(int sysnum);


#endif //_SYS_SYSCALL_H
