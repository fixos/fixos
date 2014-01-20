#ifndef _SYS_SCHEDULER_H
#define _SYS_SCHEDULER_H

/**
 * Process scheduler, maintains the list of running tasks and allow to manage
 * current process/tasks.
 *
 * For now, task is a process_t, but maybe it will change later, if something
 * like thread are implemented.
 */

#include <sys/process.h>

typedef process_t task_t;


void sched_init();

/**
 * Add the given task to scheduler.
 */
void sched_add_task(task_t *task);


extern void arch_sched_preempt_task();

/*
 * Function used by sys_fork() to save the context exactly as
 * arch_sched_preempt_task, but without calling context_saved_next() immediatly.
 * Instead, returns a value, used to différenciate primary return, and
 * re-execution from saved context (first one is the parent, second is the
 * child process).
 * Returns 0 for parent process, 1 for child process (and negative value
 * in error case).
 * dest is the process_t used for the child process, and kstack the kernel
 * stack used by the dest process (must be an exact copy of the parent's one,
 * and given pointer must be equivalent to the current parent tack position!)
 */
extern int arch_sched_preempt_fork(process_t *dest, void *kstack);


/**
 * Save context of the current task and run the next one based on priorities
 * and scheduling algorithm used.
 */
//void sched_next_task() __attribute__ ((alias ("_arch_sched_preempt_task")));
#define sched_next_task arch_sched_preempt_task


/**
 * Start scheduling (in theory, this function is called only once by the kernel,
 * once everything is initilized and 'init' process was added to scheduler).
 * This function never returns if everything is all right.
 */
void sched_start();



#endif //_SYS_SCHEDULER_H
