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


/**
 * Save context before to call next() function.
 * The current context is saved in cur_proc context (to avoid to look
 * for the current process whereas PID and ASID may changed to the
 * new values).
 * This function return only when the context is scheduled again.
 */
void arch_sched_preempt_task(struct _process_info *cur_proc, void (*next)());


#endif //_ARCH_GENERIC_PROCESS_H
