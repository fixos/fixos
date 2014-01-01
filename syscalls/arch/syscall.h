#ifndef _SYSCALL_ARCH_SYSCALL_H
#define _SYSCALL_ARCH_SYSCALL_H

// fast userland syscall definition
// dirty solution (temp) because these functions need to be defined in USER code
#define _SYSCALL_(name,id) \
	void name(int a, int b, int c, int d) __attribute__ (( section(".user.text") )); \
	void name(int a, int b, int c, int d) \
	{ \
		asm volatile ("trapa #" #id ";" ); \
	}

#define SYSCALL_NUMBER		4

// for kernel-part syscall handling
extern void* _syscall_funcs[];

/**
 * Returns the syscall function associated to sysnum, or NULL if the given
 * syscall is not implemented.
 */
void* syscall_get_function(int sysnum);

#endif //_SYSCALL_ARCH_SYSCALL_H
