#ifndef _SYSCALL_ARCH_SYSCALL_H
#define _SYSCALL_ARCH_SYSCALL_H


#define SYSCALL_NUMBER		16

// for kernel-part syscall handling
extern void* _syscall_funcs[];

/**
 * Returns the syscall function associated to sysnum, or NULL if the given
 * syscall is not implemented.
 */
void* syscall_get_function(int sysnum);

#endif //_SYSCALL_ARCH_SYSCALL_H
