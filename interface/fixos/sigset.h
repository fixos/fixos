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

#ifndef _FIXOS_INTERFACE_SIGSET_H
#define _FIXOS_INTERFACE_SIGSET_H

/**
 * sigset_t type definition and some inline functions to play with it.
 */

#include <fixos/types.h>


typedef __kernel_uint32 sigset_t;


// not exactly the POSIX specification, but should be usefull :

static inline int sigemptyset(sigset_t *set) {
	*set = 0;
	return 0;
}

static inline int sigfillset(sigset_t *set) {
	*set = 0xFFFFFFFF;
	return 0;
}

static inline int sigaddset(sigset_t *set, int sig) {
	*set |= (1<<sig);
	return 0;
}

static inline int sigdelset(sigset_t *set, int sig) {
	*set &= ~(1<<sig);
	return 0;
}

static inline int sigismember(sigset_t *set, int sig) {
	return (*set & (1<<sig)) != 0;
}


#endif //_FIXOS_INTERFACE_SIGSET_H
