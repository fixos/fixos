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

struct _process_info;

/**
 * Add the given signal to pending signal set of proc, and try to wake it up
 * (involving possible switching from STATE_INTERRUPTIBLE to STATE_RUNNING
 * and ask rescheduling when possible if needed).
 */
void signal_raise(struct _process_info *proc, int sig);


/**
 * Check for pending signals for the current process, and if any, deliver the
 * first non-ignored one.
 * Delivering the signal involve forced context switch, so this function never
 * returns if a signal is delivered.
 */
void signal_deliver_pending();


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


#endif //_SYS_SIGNAL_H
