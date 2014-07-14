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
// arguments and environnement location (for parameterized main())
#define ARCH_UNEWPROC_DEFAULT_ARGS	0x50000000


// information needed for process switch(store/load context)
struct _context_info {
	// registers r0~r15 (only the current bank for r0~r15) 
	uint32 reg[16];

	uint32 gbr;

	uint32 macl;
	uint32 mach;

	uint32 pc;
	uint32 sr;

	uint32 pr;

	// previous context structure (may be NULL if this interrupted from user space)
	struct _context_info *previous;
};

// extern inline void arch_kernel_contextjmp(struct _context_info *cnt)
//	__attribute__ ((noreturn)) ;

void arch_kernel_contextjmp(struct _context_info *cnt, struct _context_info **old_cnt);


/**
 * Process used as "idle task", executed once scheduler is stared, when no other
 * task should be done.
 * As the scheduler is initialized after all the early init job, the stack
 * used may be the kernel init stack.
 */
extern struct _process_info _arch_idle_task;

/**
 * Initialize the idle task, should be called just before the scheduler start.
 */
void arch_init_idle();

#endif //_ARCH_SH_PROCESS_H
