#include "scheduler.h"


#ifndef offsetof
#define offsetof(st, m) ((int)(&((st *)0)->m))
#endif

#define SCHED_MAX_TASKS		10

// for now it's a realy stupid implementation : no priority, no FIFO...
// a simple array contains all tasks (or NULL if no task), and each one is
// executed after the previous...
static task_t *_tasks[SCHED_MAX_TASKS];

static int _cur_task;

void sched_init() {
	int i;
	
	for(i=0; i<SCHED_MAX_TASKS; i++)
		_tasks[i] = NULL;
	_cur_task = -1;
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
void context_saved_next() {
	// find the next thread to execute
	int i;

	//  start to search from the next process
	for(i=1; i<SCHED_MAX_TASKS && _tasks[(i+_cur_task)%SCHED_MAX_TASKS]==NULL; i++);

	if(i<SCHED_MAX_TASKS) {
		_cur_task = (i+_cur_task)%SCHED_MAX_TASKS;
		process_contextjmp(_tasks[_cur_task]);
	}
}

/*
void sched_next_task() {
	// save previous context in process structure
	
	// do exactly as if the context was the instruction just after the call of
	// this function (r0~r7 are callee-saved, but we must save r8~r15, gbr, etc... 
	asm volatile (	"sts.l pr, @-r15;"
					"mov.l proc_get_cur, r0;"
					"jsr @r0;"
					"nop;"
					"add %0, r0;"
					"mov.l @r15+, r1;"
					"mov.l r15, @(60, r0);"
					"mov.l r14, @(56, r0);"
					"mov.l r13, @(52, r0);"
					"mov.l r12, @(48, r0);"
					"mov.l r11, @(44, r0);"
					"mov.l r10, @(40, r0);"
					"mov.l r9, @(36, r0);"
					"mov.l r8, @(32, r0);"
					"add #64, r0;"
					"stc sr, r2;"
					"mov.l r2, @(16, r0);"
					"stc gbr, r2;"
					"mov.l r2, @(0, r0);"
					"mov.l r1, @(12, r0);"
					"sts mach, r2;"
					"mov.l r2, @(8, r0);"
					"sts macl, r2;"
					"mov.l r2, @(4, r0);"
					""
					"mov.l context_saved_next, r0;"
					"jmp @r0;"
					""
					".align 4;"
					"proc_get_cur: ; .long _process_get_current;"
					"context_saved_next: ; .long _context_saved_next;"
					: : "n"(offsetof(process_t, acnt)) : );
}
*/



void sched_start() {
	// look for the first task
	int i;

	for(i=0; i<SCHED_MAX_TASKS && _tasks[i]==NULL; i++);

	if(i<SCHED_MAX_TASKS) {
		_cur_task = i;
		process_contextjmp(_tasks[i]);
	}
}
