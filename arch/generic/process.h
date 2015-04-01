/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ARCH_GENERIC_PROCESS_H
#define _ARCH_GENERIC_PROCESS_H

/**
 * This machine specific header must define at least the struct _context_info.
 */
#include <arch/process.h>


struct process;


// static inline void arch_kernel_contextjmp(struct _context_info *cnt)
//	__attribute__ ((noreturn)) ;

void arch_kernel_contextjmp(struct _context_info *cnt, struct _context_info **old_cnt);

/**
 * Return 1 if the "saved context" of the given process was executed in user
 * mode, 0 if it was in kernel mode (or negative value in error case).
 */
int arch_process_mode(struct process *proc);


/**
 * Initialize the idle task, should be called just before the scheduler start.
 */
void arch_init_idle(struct process *proc);


/**
 * Save context before to call next() function.
 * The current context is saved in cur_proc context (to avoid to look
 * for the current process whereas PID and ASID may changed to the
 * new values).
 * This function return only when the context is scheduled again.
 */
void arch_sched_preempt_task(struct process *cur_proc, void (*next)());


#endif //_ARCH_GENERIC_PROCESS_H
