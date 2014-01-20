#ifndef _ARCH_SH_PROCESS_H
#define _ARCH_SH_PROCESS_H

/**
 * Low level primitives for process manipulation.
 */

#include <utils/types.h>

// Default values for user mode process :
// MD, BL, RB = 0, Interrupt Mask = 0x0...
#define ARCH_UNEWPROC_DEFAULT_SR (0)
// begin of stack area (greatest address, never reached) :
#define ARCH_UNEWPROC_DEFAULT_STACK	0x40000000
// begin of process instructions
#define ARCH_UNEWPROC_DEFAULT_TEXT	0x10000000


extern void * g_process_current_kstack;


// information needed for process switch(store/load context)
struct _context_info {
	// registers r0~r15 (only the current bank for r0~r15) 
	uint32 reg[16];

	uint32 gbr;

	uint32 macl;
	uint32 mach;

	uint32 pc;
	uint32 sr;

	void *kernel_stack;
	int *bank0_saved;
};

// extern inline void arch_kernel_contextjmp(struct _context_info *cnt)
//	__attribute__ ((noreturn)) ;

// 
void arch_kernel_contextjmp(struct _context_info *cnt);

#endif //_ARCH_SH_PROCESS_H
