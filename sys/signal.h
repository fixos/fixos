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

#endif //_SYS_SIGNAL_H
