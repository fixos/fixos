#ifndef _FIXOS_INTERFACE_TIMES_H
#define _FIXOS_INTERFACE_TIMES_H

/**
 * Define the struct tms as defined in sys/times.h by POSIX standard.
 */

#include <fixos/types.h>


struct tms {
	__kernel_clock_t tms_utime;
	__kernel_clock_t tms_stime;
	__kernel_clock_t tms_cutime;
	__kernel_clock_t tms_cstime;
};


#endif //_FIXOS_INTERFACE_TIMES_H
