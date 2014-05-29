#include <sys/process.h>
#include <sys/scheduler.h>
#include <sys/interrupt.h>
#include "signal.h"

#include <utils/log.h>


#define _SIGUNDEF 		0xFF

#define INDEX_SIGINT	0
#define INDEX_SIGQUIT	1
#define INDEX_SIGILL	2
#define INDEX_SIGKILL	3
#define INDEX_SIGSEGV	4
#define INDEX_SIGALRM	5
#define INDEX_SIGCHLD	6
#define INDEX_SIGCONT	7
#define INDEX_SIGSTOP	8


// default actions for each signal
#define SIGDFL_IGN		0
#define SIGDFL_STOP		1
#define SIGDFL_TERM		2
#define SIGDFL_CONT		3

/*
// translation array, from index to signal number
static uint8 _trans_index2number[SIGNAL_INDEX_MAX] = {
	SIGINT,
	SIGQUIT,
	SIGILL,
	SIGKILL,
	SIGSEGV,
	SIGALRM,
	SIGCHLD,
	SIGCONT,
	SIGSTOP
};*/


// translation array, from signal number to signal index
static uint8 _trans_number2index[SIGNAL_MAX] = {
	_SIGUNDEF,		// 0
	_SIGUNDEF,
	INDEX_SIGINT,
	INDEX_SIGQUIT,
	INDEX_SIGILL,	// 4
	_SIGUNDEF,
	_SIGUNDEF,
	_SIGUNDEF,
	_SIGUNDEF,
	INDEX_SIGKILL,	// 9
	_SIGUNDEF,
	INDEX_SIGSEGV,
	_SIGUNDEF,
	_SIGUNDEF,
	INDEX_SIGALRM,	// 14
	_SIGUNDEF,
	_SIGUNDEF,
	INDEX_SIGCHLD,
	INDEX_SIGCONT,
	INDEX_SIGSTOP,	// 19
	_SIGUNDEF,
	_SIGUNDEF,
	_SIGUNDEF,
	_SIGUNDEF,
	_SIGUNDEF,		// 24
	_SIGUNDEF,
	_SIGUNDEF,
	_SIGUNDEF,
	_SIGUNDEF,
	_SIGUNDEF,		// 29
	_SIGUNDEF,
	_SIGUNDEF		// 32
};


// get default action from implemented signal index
static uint8 _trans_index2default[SIGNAL_INDEX_MAX] = {
	SIGDFL_TERM, //SIGINT
	SIGDFL_TERM, //SIGQUIT
	SIGDFL_TERM, //SIGILL
	SIGDFL_TERM, //SIGKILL
	SIGDFL_TERM, //SIGSEGV
	SIGDFL_TERM, //SIGALRM
	SIGDFL_IGN, //SIGCHLD
	SIGDFL_CONT, //	SIGCONT
	SIGDFL_STOP //	SIGSTOP
};


// check if the given signal is not to ignore, and if not, prepare process
// to handle it and remove it from the pending signals
static void try_deliver_one(struct _process_info *proc, int sig) {
	int sigindex;
	struct sigaction *action;

	sigindex = _trans_number2index[sig];
	if(sigindex == _SIGUNDEF) {
		sigdelset(& proc->sig_pending, sig);
		printk("signal: unimplemented SIG %d ignored\n", sig);
	}
	else {
		action = &(proc->sig_array[sigindex]);
		if(action->sa_handler == SIG_IGN
				|| (action->sa_handler == SIG_DFL
					&& _trans_index2default[sigindex] == SIGDFL_IGN) )
		{
			// ignored signal
			sigdelset(& proc->sig_pending, sig);
		}
		else {
			if(action->sa_handler == SIG_DFL) {
				switch(_trans_index2default[sigindex]) {
				case SIGDFL_TERM:
					// TODO
					break;
				case SIGDFL_STOP:
					// TODO
					break;
				case SIGDFL_CONT:
					// TODO
					break;
				}
			}
			else {
				// this is a user-defined handler
				arch_process_prepare_sigcontext(proc, action, sig);
				sigdelset(& proc->sig_pending, sig);
				proc->sig_blocked |= (action->sa_mask & ~(SIGKILL | SIGSTOP));
				arch_kernel_contextjmp(proc->acnt, & proc->acnt);
				//process_contextjmp(proc);
			}
		}
	}
}


void signal_raise(struct _process_info *proc, int sig) {
	sched_preempt_block();
	sigaddset(& proc->sig_pending, sig);
	
	// wake up destination process if any signal is pending
	if((proc->sig_pending & proc->sig_blocked)
			&& (proc->state == PROCESS_STATE_INTERRUPTIBLE))
	{
		sched_wake_up(proc);
	}

	sched_preempt_unblock();
}



void signal_deliver_pending() {
	sigset_t todeliver;
	process_t *proc;

	proc = process_get_current();
	todeliver = proc->sig_pending & proc->sig_blocked;

	if(todeliver != 0) {
		int atomicstate;
		int cursig;

		interrupt_atomic_save(&atomicstate);

		for(cursig=0; cursig<SIGNAL_MAX; cursig++) {
			if(sigismember(&todeliver, cursig)) {
				try_deliver_one(proc, cursig);
			}
		}
		
		interrupt_atomic_restore(atomicstate);
	}
}

