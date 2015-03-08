#ifndef _FIXOS_INTERFACE_PROCESS_H
#define _FIXOS_INTERFACE_PROCESS_H

/**
 * Useful process-related definitions for userspace.
 */


// for process status inspecting (wait() and waitpid())
#define WIFEXITED(status)			(!((status) & 0xFFFFFF00))
#define WEXITSTATUS(status)			((status) & 0xFF)
#define _WSTATUS_EXITS(retval)		((retval) & 0xFF)

#define WIFSIGNALED(status)			(((status) & 0xFFFFFF00) == 0x100)
#define WTERMSIG(status)			((status) & 0xFF)
#define _WSTATUS_TERMSIG(signum)	( ((signum) & 0xFF) | 0x100)

#define WIFSTOPPED(status)			(((status) & 0xFFFFFF00) == 0x200)
#define WSTOPSIG(status)			((status) & 0xFF)
#define _WSTATUS_STOPSIG(signum)	( ((signum) & 0xFF) | 0x200)

#define WIFCONTINUED(status)		(((status) & 0xFFFFFF00) == 0x400)
#define _WSTATUS_CONT()				(0x400)


// waitpid() flags
#define WHNOHANG	(1<<0)
#define WUNTRACED	(1<<1)
#define WCONTINUED	(1<<2)


// FiXos specific definitions of process status
#define PROCESS_STATE_RUNNING			2
// used at process creation :
#define PROCESS_STATE_CREATE			3
// used after calling exit()
#define PROCESS_STATE_ZOMBIE			5
// sleeping states
#define PROCESS_STATE_STOPPED			4
#define PROCESS_STATE_INTERRUPTIBLE		6
#define PROCESS_STATE_UNINTERRUPTIBLE	7



// process information for user (through sysctl() interfaces)
struct proc_uinfo {
	__kernel_pid_t pid;
	__kernel_pid_t ppid;

	int state;
	int exit_status; // only valid when state is PROCESS_STATE_ZOMBIE

	__kernel_clock_t uticks;
	__kernel_clock_t kticks;

	// TODO real fixed point
	// 100 times the average CPU usage in percent
	int cpu_usage;
};

#endif //_FIXOS_INTERFACE_PROCESS_H
