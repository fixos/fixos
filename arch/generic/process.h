#ifndef _ARCH_GENERIC_PROCESS_H
#define _ARCH_GENERIC_PROCESS_H

/**
 * This machine specific header must define at least the struct _context_info.
 */
#include <arch/process.h>


struct _process_info;


// extern inline void arch_kernel_contextjmp(struct _context_info *cnt)
//	__attribute__ ((noreturn)) ;

void arch_kernel_contextjmp(struct _context_info *cnt, struct _context_info **old_cnt);

/**
 * Return 1 if the "saved context" of the given process was executed in user
 * mode, 0 if it was in kernel mode (or negative value in error case).
 */
int arch_process_mode(struct _process_info *proc);


/**
 * Initialize the idle task, should be called just before the scheduler start.
 */
void arch_init_idle(struct _process_info *proc);

#endif //_ARCH_GENERIC_PROCESS_H
