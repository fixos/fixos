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

#ifndef _ARCH_GENERIC_SIGNAL_H
#define _ARCH_GENERIC_SIGNAL_H

struct process;

/**
 * Save the user-mode context, and set the saved context to the signal handler
 * defined in action.
 * The user stack is used to save previous context, and a trampoline is set to
 * do appropriate cleanup after the end of the signal handler.
 */
void arch_signal_prepare_sigcontext(struct process *proc,
		struct sigaction *action, int sig);


/**
 * Restore the user-mode context saved by arch_signal_prepare_sigcontext(),
 * assuming the user stack is preserved...
 */
void arch_signal_restore_sigcontext(struct process *proc);



#endif //_ARCH_GENERIC_SIGNAL_H
