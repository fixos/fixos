#ifndef _SYS_SIGNAL_H
#define _SYS_SIGNAL_H

/**
 * Signal management functions.
 * To be memory efficient, and to keep signal number to the usual Linux value,
 * translation should be done between signal number (used for sys_kill()) and
 * signal index in process signal array (so unused signal don't use memory in
 * each process struct).
 */

#include <interface/signal.h>
#include <utils/types.h>


// number of signals implemented
#define SIGNAL_INDEX_MAX		10

// sigset_t of pending signals
#define signal_pending(proc)	((proc)->sig_pending & ~((proc)->sig_blocked))

struct _process_info;

/**
 * Add the given signal to pending signal set of proc, and try to wake it up
 * (involving possible switching from STATE_INTERRUPTIBLE to STATE_RUNNING
 * and ask rescheduling when possible if needed).
 */
void signal_raise(struct _process_info *proc, int sig);

/**
 * Raise a signal for a group of processes.
 */
void signal_pgid_raise(pid_t pgid, int sig);


/**
 * Check for pending signals for the current process, and if any, deliver the
 * first non-ignored one.
 * Delivering the signal involve forced context switch, so this function never
 * returns if a signal is delivered.
 */
void signal_deliver_pending();


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



/**
 * sigaction() syscall implementation, change the way the signal sig is handled,
 * using the action described in act, and store the previous action in oact is
 * not NULL.
 * If act is NULL, only store the current action in oact.
 */
int sys_sigaction(int sig, const struct sigaction * act, struct sigaction * oact);


/**
 * kill() syscall implementation, deliver the signal sig to process(es) described
 * by pid.
 * For now, only positive pid value is accepted (select a single process).
 */
int sys_kill(pid_t pid, int sig);


/**
 * sigprocmask() syscall implementation, allowing to check, block and
 * unblock signals.
 */
int sys_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);


/**
 * sigreturn() syscall implementation, restore the context of the process as
 * it was before the signal interuption in execution flow.
 * This syscall is only designed to be called in the signal trampoline, and
 * should not be used in other places...
 * This syscall never return.
 */
int sys_sigreturn();


#endif //_SYS_SIGNAL_H
