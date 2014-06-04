#ifndef _FIXOS_INTERFACE_SIGNAL_H
#define _FIXOS_INTERFACE_SIGNAL_H

/**
 * Signal related definitions needed by user space programs.
 */

#include "sigset.h"


/**
 * POSIX signal implemented.
 * Signal number is the one used by Linux on SH when possible. 
 */
#define SIGINT		2	//Term	Interrupt from keyboard
#define SIGQUIT		3	//Core	Quit from keyboard
#define SIGILL		4	//Core	Illegal Instruction
#define SIGKILL		9	//Term	Kill signal
#define SIGSEGV		11	//Core	Invalid memory reference
#define SIGALRM		14	//Term	Timer signal from alarm(2)
#define SIGCHLD		17	//Ign	Child stopped or terminated
#define SIGCONT		18		//Continue if stopped
#define SIGSTOP		19	//Stop	Stop process
// Not implemented signals from POSIX.1
//SIGABRT	6	Core	Abort signal from abort(3)
//SIGFPE	8	Core	Floating point exception
//SIGPIPE	13	Term	Broken pipe: write to pipe with no readers
//SIGTERM	15	Term	Termination signal
//SIGUSR1	10	Term	User-defined signal 1
//SIGUSR2	12	Term	User-defined signal 2
//SIGTSTP	20	Stop	Stop typed at tty
//SIGTTIN	21	Stop	tty input for background process
//SIGTTOU	22	Stop	tty output for background process

// number of signals (maximum allowed signal is SIGNAL_MAX-1)
#define SIGNAL_MAX		32


// siginfo_t is not defined for now...
struct siginfo_s;
typedef struct siginfo_s siginfo_t;


/**
 * Struct for sigaction() syscall and process signal table
 */
struct sigaction {
	union { 
		void (*sa_handler)(int);
		void (*sa_sigaction)(int, siginfo_t *, void *);
	};
	sigset_t   sa_mask;
	int        sa_flags;
};


// callback value (sa_sigaction of sa_handler) to restore default handling
#define SIG_DFL		((void*)-1)
// callback value (sa_sigaction of sa_handler) ignore signal if possible
#define SIG_IGN		((void*)-3)


/**
 * siginfo sa_flags possible flags :
 */
#define SA_RESETHAND	(1<<0)
#define	SA_SIGINFO		(1<<1)

#define SA_ONESHOT		SA_RESETHAND


/**
 * Values for sigprocmask() 
 */
#define SIG_BLOCK		1
#define SIG_UNBLOCK		2
#define SIG_SETMASK		3


#endif //_FIXOS_INTERFACE_SIGNAL_H

