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

#ifndef _SYS_SCHEDULER_H
#define _SYS_SCHEDULER_H

/**
 * Process scheduler, maintains the list of running tasks and allow to manage
 * current process/tasks.
 *
 * For now, task is a struct process, but maybe it will change later, if something
 * like thread are implemented.
 */

#include <sys/time.h>


struct process;

/**
 * Initialize scheduler, to be ready to run the first process later.
 */
void sched_init();

/**
 * Add the given task to scheduler.
 */
void sched_add_task(struct process *task);

/*
 * Function used by sys_fork() to save the context exactly as
 * arch_sched_preempt_task, but without calling context_saved_next() immediatly.
 * Instead, returns a value, used to differenciate primary return, and
 * re-execution from saved context (first one is the parent, second is the
 * child process).
 * Returns 0 for parent process, 1 for child process (and negative value
 * in error case).
 * dest is the struct process used for the child process, and kstack the kernel
 * stack used by the dest process (must be an exact copy of the parent's one,
 * and given pointer must be equivalent to the current parent stack position!)
 */
//extern int arch_sched_preempt_fork(struct process *dest, void *kstack);


/**
 * Save context of the current task and run the next one based on priorities
 * and scheduling algorithm used.
 */
void sched_schedule();


/**
 * Start scheduling (in theory, this function is called only once by the kernel,
 * once everything is initilized and 'init' process was added to scheduler).
 * This function never returns if everything is all right.
 */
void sched_start();


/**
 * Called at each clock tick, to check if the scheduled task must be changed.
 * If it's quantum is ellapsed, the function return normally, and the next
 * call to sched_if_needed() will change the current running task.
 */
void sched_check();


/**
 * Must be called at the end of exceptions/interrupts handlers, to schedule
 * instead returning if needed (lazy scheduling, or if a high-priority task
 * must be executed, like bottom-half of interrupt handler)
 */
void sched_if_needed();


/**
 * sched_preempt_block() and sched_preempt_unblock() are used to define code
 * section that should not be preempted (but can be interrupted by hardware
 * interrupt/exceptions).
 * This kind of "critical section" an be nested (each call to block() should
 * be associated to exactly one call to unblock(), and the preemption is
 * possible only when the last unblock() is called).
 * It is possible to get the number of needed unblock() calls before preemption
 * is enable again by calling sched_preempt_level().
 */
void sched_preempt_block();
void sched_preempt_unblock();
int sched_preempt_level();

/**
 * Wake up a process, when signal is received or any waiting condition is
 * reached.
 * If the process was in any sleeping state, either interruptible or not, it
 * will be switched to STATE_RUNNING.
 * In addition, scheduler will ask rescheduling if woken up process has a
 * higher priority than the current process.
 */
void sched_wake_up(struct process *proc);


/**
 * Remove the given process from running queue, and put it in "stopped" state.
 * The signal number may be used if parent wait for stopped process (waitpid()).
 */
void sched_stop_proc(struct process *proc, int sig);


/**
 * Put a previously stopped process in running state and in the scheduler running
 * queue.
 */
void sched_cont_proc(struct process *proc);


/**
 * FIXME Temporary feature, probably removed soon.
 * Find a process in current queues (waiting/running) from its ASID.
 */
struct process *sched_find_pid(pid_t pid);


/**
 * wait for a child terminating, returns its pid and set status to child
 * exit status
 */
pid_t sys_wait(int *status);

/**
 * sleep for the given req time, with a nanosecond precision value
 * TODO allow signal to interrupt the syscall, use rem to store the remaining
 * time if needed, and add special case of busy-loop waiting if a process
 * is using real time scheduling policy with a small time amount.
 */
int sys_nanosleep(const struct timespec *req, struct timespec *rem);


#endif //_SYS_SCHEDULER_H
