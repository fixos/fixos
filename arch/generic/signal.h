#ifndef _ARCH_GENERIC_SIGNAL_H
#define _ARCH_GENERIC_SIGNAL_H

struct _process_info;

/**
 * Save the user-mode context, and set the saved context to the signal handler
 * defined in action.
 * The user stack is used to save previous context, and a trampoline is set to
 * do appropriate cleanup after the end of the signal handler.
 */
void arch_signal_prepare_sigcontext(struct _process_info *proc,
		struct sigaction *action, int sig);


/**
 * Restore the user-mode context saved by arch_signal_prepare_sigcontext(),
 * assuming the user stack is preserved...
 */
void arch_signal_restore_sigcontext(struct _process_info *proc);



#endif //_ARCH_GENERIC_SIGNAL_H
