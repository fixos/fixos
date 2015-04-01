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

#include <sys/process.h>
#include <sys/signal.h>
#include <arch/generic/signal.h>
#include <arch/generic/process.h>

#include "interrupt.h"
#include <sys/interrupt.h>

#include <utils/log.h>


extern uint32 sig_trampoline_begin;
extern uint32 sig_trampoline_end;


struct sigcontext {
	struct _context_info context;
	sigset_t oldmask;
};


void arch_signal_prepare_sigcontext(struct process *proc, struct sigaction *action,
		int sig) 
{
	// TODO securized user stack access...
	uint32 *ustack;
	struct sigcontext *ucontext;
//	uint32 *tramp;
	struct _context_info *cnt;
	int inter;


	interrupt_atomic_save(&inter);
	// we NEED to handle exceptions (write on user-land stack!!!)
	interrupt_inhibit_all(0);
	
	// first, store the full context on the top of user stack
	cnt = proc->acnt;
	ustack = (void*)(cnt->reg[15] - cnt->reg[15]%4);

	ucontext = ((void*)ustack) - sizeof(struct sigcontext);
	ustack = (void*)ucontext;

	printk(LOG_DEBUG, "signal: prepare %d, handler=%p, ucontext=%p, ustack=%p\n",
			sig, action->sa_handler, ucontext, ustack);


	ucontext->context = *cnt;
	ucontext->oldmask = proc->sig_blocked;

	
	// now prepare signal cleanup
	// stupid assertion : sig_trampoline code is 2 instructions...
	*(--ustack) = sig_trampoline_begin;
	cnt->pr = (uint32)ustack;

	// finally, prepare the context to execute the signal handler
	cnt->sr = ARCH_UNEWPROC_DEFAULT_SR;
	cnt->pc = (uint32)(action->sa_handler);
	
	cnt->reg[4] = sig;
	cnt->reg[15] = (uint32)ustack;
	// TODO other SA_xxx flags
	if(action->sa_flags & SA_SIGINFO) {
		// call sa_sigaction, with additionnal parameters (NULL for now)
		cnt->reg[5] = 0;
		cnt->reg[6] = 0;
	}

	printk(LOG_DEBUG, "signal: prepare() end\n");
	interrupt_atomic_restore(inter);
}



void arch_signal_restore_sigcontext(struct process *proc) {
	struct sigcontext *scnt;

	scnt = (void*)(proc->acnt->reg[15] + 4);
	
	*(proc->acnt) = scnt->context;
	// TODO check proc->acnt->sr to be *sure* its value is not corrupted by
	// user process
	proc->acnt->previous = NULL;
	proc->sig_blocked = scnt->oldmask;
}
