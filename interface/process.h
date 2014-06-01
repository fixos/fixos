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

#endif //_FIXOS_INTERFACE_PROCESS_H
