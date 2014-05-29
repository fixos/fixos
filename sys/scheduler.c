#include "scheduler.h"
#include <sys/memory.h>
#include <sys/interrupt.h>
#include <utils/log.h>


#define SCHED_MAX_TASKS		10

// TODO better place (arch/config specific)
#define SCHED_QUANTUM_TICKS	4	

// for now it's a realy stupid implementation : no priority, no FIFO...
// a simple array contains all tasks (or NULL if no task), and each one is
// executed after the previous...
static task_t *_tasks[SCHED_MAX_TASKS];

static int _cur_task;

// number of clock ticks before current task's quantum is elapsed (0 means the
// next clock tick should preempt the current task)
static int _cur_quantum_left;

// boolean used for lazy-scheduling (set by sched_check() and related)
// its value is checked by sched_if_needed(), which must be called at the end
// of exceptions/interrupts handlers
static int _need_reschedule;


// temp stub to be sure sched_check() and sched_if_needed() do nothing
// before scheduler is started
static int _started = 0;


// kernel preemption is allowed when _preempt_level is zero
static int _preempt_level = 0;

void sched_init() {
	int i;
	
	for(i=0; i<SCHED_MAX_TASKS; i++)
		_tasks[i] = NULL;
	_cur_task = -1;

	_cur_quantum_left = SCHED_QUANTUM_TICKS;
	_need_reschedule = 0;
	_preempt_level = 0;
	_started = 0;
}


void sched_add_task(task_t *task) {
	// look for the first slot in task list
	int i;

	for(i=0; i<SCHED_MAX_TASKS && _tasks[i]!=NULL; i++);

	if(i<SCHED_MAX_TASKS)
		_tasks[i] = task;
	else {
		// error
	}
}

// called once sched_next_task() saved the current process context
static void context_saved_next() {
	// find the next task to execute
	int i;

	if(_tasks[_cur_task]->state == PROCESS_STATE_RUNNING)
		_tasks[_cur_task]->state = PROCESS_STATE_IDLE;

	//  start to search from the next process
	for(i=1; i<SCHED_MAX_TASKS && ( _tasks[(i+_cur_task)%SCHED_MAX_TASKS]==NULL
				|| _tasks[(i+_cur_task)%SCHED_MAX_TASKS]->state == PROCESS_STATE_ZOMBIE) ; i++);

	_cur_quantum_left = SCHED_QUANTUM_TICKS;
	_need_reschedule = 0;

	if(i<SCHED_MAX_TASKS) {
		_cur_task = (i+_cur_task)%SCHED_MAX_TASKS;
		process_contextjmp(_tasks[_cur_task]);
	}
}


void sched_schedule() {
	int atomicsaved;
	interrupt_atomic_save(&atomicsaved);
	arch_sched_preempt_task(process_get_current(), &context_saved_next);
	interrupt_atomic_restore(atomicsaved);
}


void sched_start() {
	// look for the first task
	int i;

	for(i=0; i<SCHED_MAX_TASKS && _tasks[i]==NULL; i++);

	if(i<SCHED_MAX_TASKS) {
		_cur_task = i;
		_started = 1;
		process_contextjmp(_tasks[i]);
	}
}


void sched_check() {
	if(_started) {
		// decrement the current quantum counter and set schedule needed
		_cur_quantum_left--;
		if(_cur_quantum_left <= 0) {
			_need_reschedule = 1;
		}
	}
}




void sched_if_needed() {
	// TODO it seems a good place to check for "soft interrupts", like
	// linux Tasklet, and schedule one of them if needed
	if(_started && _need_reschedule && _preempt_level <= 0) {
		context_saved_next();
	}
}


void sched_preempt_block() {
	_preempt_level++;
}


void sched_preempt_unblock() {
	_preempt_level--;
	if(_preempt_level < 0) {
		printk("[W] Preemption level < 0\n");		
		_preempt_level = 0;
	}
}


int sched_preempt_level() {
	return _preempt_level;
}


void sched_wake_up(process_t *proc) {
	if(proc->state == PROCESS_STATE_INTERRUPTIBLE ||
			proc->state == PROCESS_STATE_UNINTERRUPTIBLE)
	{
		proc->state = PROCESS_STATE_RUNNING;
		// should be added to the waiting queue when available...
	}

	if(proc->state == PROCESS_STATE_RUNNING) {
		// TODO check for priority when implemented
		// if(getpriority(proc) > getpriority(process_get_current())) {
		//		_need_reschedule = 1;
		// }
	}
}


pid_t sys_wait(int *status) {
	// do not return before a child is in Zombie state
	
	int ret = 0;
	pid_t ppid = process_get_current()->pid;

	while(ret == 0) {
		int i;
		int child_found = 0;
		
		sched_preempt_block();
		for(i=0; i<SCHED_MAX_TASKS; i++) {
			//printk("task %d", i);
			//printk("->%p\n", _tasks[i]);
			if(_tasks[i] != NULL && _tasks[i]->ppid == ppid) {
				if(_tasks[i]->state == PROCESS_STATE_ZOMBIE) {
					ret = _tasks[i]->pid;
					*status = _tasks[i]->exit_status;

					// destroy the waiting process
					// do not forget kernel_stack is set to the first byte of the
					// next page, not on the real allocated page
					mem_pm_release_page(_tasks[i]->kernel_stack-1);

					// release ASID
					process_release_asid(_tasks[i]);

					process_free(_tasks[i]);
					_tasks[i] = NULL;
				}
				child_found=1;
			}
		}

		if(!child_found) {
			ret = -1;
		}

		sched_preempt_unblock();

		if(ret == 0)
			sched_schedule();
	}

	return ret;
}
