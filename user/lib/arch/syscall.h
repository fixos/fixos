#ifndef _SYSCALL_ARCH_SYSCALL_H
#define _SYSCALL_ARCH_SYSCALL_H

// fast userland syscall definition
// dirty solution (temp) because these functions need to be defined in USER code
#define _SYSCALL_(name,id) \
	void name(int a, int b, int c, int d) \
	{ \
		asm volatile ("trapa #" #id ";" ); \
	}

#endif //_SYSCALL_ARCH_SYSCALL_H