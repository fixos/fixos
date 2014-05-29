#ifndef _FIXOS_INTERFACE_SIGSET_H
#define _FIXOS_INTERFACE_SIGSET_H

/**
 * sigset_t type definition and some inline functions to play with it.
 */

#include "types.h"


typedef uint32 sigset_t;


// not exactly the POSIX specification, but should be usefull :

extern inline int sigemptyset(sigset_t *set) {
	*set = 0;
	return 0;
}

extern inline int sigfillset(sigset_t *set) {
	*set = 0xFFFFFFFF;
	return 0;
}

extern inline int sigaddset(sigset_t *set, int sig) {
	*set |= (1<<sig);
	return 0;
}

extern inline int sigdelset(sigset_t *set, int sig) {
	*set &= ~(1<<sig);
	return 0;
}

extern inline int sigismember(sigset_t *set, int sig) {
	return (*set & (1<<sig)) != 0;
}


#endif //_FIXOS_INTERFACE_SIGSET_H
