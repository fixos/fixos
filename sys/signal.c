#include <sys/process.h>
#include <sys/scheduler.h>
#include <sys/interrupt.h>
#include <arch/generic/process.h>
#include "signal.h"

#include <utils/log.h>
#include <interface/fixos/errno.h>


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
static void try_deliver_one(struct process *proc, int sig) {
	int sigindex;
	struct sigaction *action;

	sigdelset(& proc->sig_pending, sig);

	sigindex = _trans_number2index[sig];
	if(sigindex == _SIGUNDEF) {
		printk(LOG_WARNING, "signal: unimplemented SIG %d ignored\n", sig);
	}
	else {
		action = &(proc->sig_array[sigindex]);
		if(action->sa_handler == SIG_IGN
				|| (action->sa_handler == SIG_DFL
					&& _trans_index2default[sigindex] == SIGDFL_IGN) )
		{
			// ignored signal
		}
		else {
			if(action->sa_handler == SIG_DFL) {
				switch(_trans_index2default[sigindex]) {
				case SIGDFL_TERM:
					process_terminate(proc, _WSTATUS_TERMSIG(sig));
					sched_schedule();
					// TODO be sure the process is *not* executed after this function ret
					break;

				case SIGDFL_STOP:
					sched_stop_proc(proc, sig);
					break;

				case SIGDFL_CONT:
					// TODO : maybe this specific SIGCONT signal is better handled
					// in the signal_raise() function, so it's working even if
					// process is not in a runnable state?
					break;
				}
			}
			else {
				// this is a user-defined handler
				arch_signal_prepare_sigcontext(proc, action, sig);
				proc->sig_blocked |= (action->sa_mask & ~(SIGKILL | SIGSTOP));
				arch_kernel_contextjmp(proc->acnt, & proc->acnt);
				//process_contextjmp(proc);
			}
		}
	}
}


void signal_raise(struct process *proc, int sig) {
	sched_preempt_block();
	sigaddset(& proc->sig_pending, sig);
	
	// wake up destination process if any signal is pending
	if((signal_pending(proc) != 0)
			&& (proc->state == PROCESS_STATE_INTERRUPTIBLE))
	{
		sched_wake_up(proc);
	}

	// SIGCONT when process is stopped
	if(sig==SIGCONT && proc->state == PROCESS_STATE_STOPPED) {
		sched_cont_proc(proc);
	}

	sched_preempt_unblock();
}


void signal_pgid_raise(pid_t pgid, int sig) {
	struct process *dest;

	for_each_process(dest) {
		//printk(LOG_DEBUG, "pgid_raise: pid %d, pgid %d\n", dest->pid, dest->pgid);
		if (dest->pgid == pgid)
			signal_raise(dest, sig);
	}
}


void signal_deliver_pending() {
	sigset_t todeliver;
	struct process *proc;

	proc = process_get_current();
	todeliver = signal_pending(proc);

	if(todeliver != 0) {
		int atomicstate;
		int cursig;

		interrupt_atomic_save(&atomicstate);

		for(cursig=0; cursig<SIGNAL_MAX; cursig++) {
			if(sigismember(&todeliver, cursig)) {
				printk(LOG_DEBUG, "signal: try to deliver %d\n", cursig);
				try_deliver_one(proc, cursig);
			}
		}
		
		interrupt_atomic_restore(atomicstate);
	}
}



int sys_sigaction(int sig, const struct sigaction *act, struct sigaction *oact) {
	if(sig>=0 && sig<SIGNAL_MAX) {
		int index;
		index = _trans_number2index[sig];

		if(index != _SIGUNDEF) {
			struct process *cur;
			cur = process_get_current();

			if(oact != NULL) {
				*oact = cur->sig_array[index];
			}

			if(act != NULL) {
				// can't change the actions for SIGKILL or SIGSTOP
				if(sig == SIGKILL || sig == SIGSTOP) {
					return -1;
				}
				cur->sig_array[index] = *act;
			}
		}
		else {
			return -1;
		}

	}
	else {
		return -1;
	}

	return 0;
}



int sys_kill(pid_t pid, int sig) {
	if(sig>=0 && sig<SIGNAL_MAX && _trans_number2index[sig] != _SIGUNDEF) {
		struct process *dest;

		printk(LOG_DEBUG, "signal: kill(%d, %d)\n", pid, sig);

		// depending pid value, this can be a gid or a pid
		if(pid > 0) {
			dest = process_from_pid(pid);
			if(dest != NULL)
				signal_raise(dest, sig);
			else
				return -EPERM;
		}
		else {
			// process group
			pid = pid==0 ? _proc_current->pgid : -pid;
			signal_pgid_raise(pid, sig);
		}
	}
	else {
		return -EINVAL;
	}

	return 0;
}


int sys_sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
	struct process *cur;

	cur = process_get_current();
	if(oldset != NULL) {
		*oldset = cur->sig_blocked;
	}

	if(set != NULL) {
		switch(how) {
		case SIG_SETMASK:
			cur->sig_blocked = *set & ~(SIGKILL | SIGSTOP);
			break;
		case SIG_BLOCK:
			cur->sig_blocked |= (*set & ~(SIGKILL | SIGSTOP));
			break;
		case SIG_UNBLOCK:
			cur->sig_blocked &= ~(*set);
			break;
		}
	}

	return 0;
}


int sys_sigreturn() {
	struct process *cur = process_get_current();

	sched_preempt_block();
	arch_signal_restore_sigcontext(cur);
	sched_preempt_unblock();

	process_contextjmp(cur);
	return -1;
}
