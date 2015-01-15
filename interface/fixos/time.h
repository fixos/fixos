#ifndef _FIXOS_INTERFACE_TIME_H
#define _FIXOS_INTERFACE_TIME_H

/**
 * Define time-related structures and constants.
 */

#include <fixos/types.h>

struct timespec {
	__kernel_time_t tv_sec;
	long tv_nsec;
};


#endif //_FIXOS_INTERFACE_TIME_H
