#ifndef _ARCH_GENERIC_TIME_H
#define _ARCH_GENERIC_TIME_H

/**
 * Should define TICK_HZ, the base clock frequency!
 */
#include <arch/time.h>

struct hr_time;


/**
 * Initialize machine time-related stuff, and set the "tick" interrupt.
 */
void arch_time_init();

/**
 * Try to read real time information from an external real time clock
 * if present, and fill the given struct with the current time.
 * Returns 0 in success case, negative value else (no RTC or error)
 */
int arch_time_get_hw(struct hr_time *t);

int arch_time_set_hw(const struct hr_time *t);


#endif //_ARCH_GENERIC_TIME_H
