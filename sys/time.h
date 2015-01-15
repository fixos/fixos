#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <utils/types.h>
#include <arch/time.h>
#include <interface/fixos/time.h>
#include <interface/fixos/times.h>

/**
 * Real time informations, using arch-dependant functions if RTC is present.
 * All time_t used here are representing seconds elapsed from Epoch.
 */


// used to compute a time in ticks, in a less config-dependant way
// do not forget it's not realy reliable if you need accuracy!
#define TICKS_MSEC_NOTNULL(msec) ((msec) * TICK_HZ / 1000 <= 0 ? 1 : \
			(msec) * TICK_HZ / 1000)


/**
 * Init time subsystem, including both real time and internal clock for CPU
 * time.
 */
void time_init();


/**
 * Returns the number of clock ticks from the startup of the time subsystem.
 */
clock_t time_monotonic_ticks();

void time_monotonic_time(struct timespec *t);

void time_real_time(struct timespec *t);


/**
 * Routine to call for each clock tick (add ellapsed time to counters,
 * manage process CPU time and other time-related events, like scheduler)
 */
void time_do_tick();


/**
 * Add time described in t1 and t2, and write the result in res.
 * res may points to the same location as t1 or t2.
 */
extern inline void time_add_hr(const struct timespec *t1,
		const struct timespec *t2, struct timespec *res)
{
	res->tv_sec = t1->tv_sec + t2->tv_sec;
	res->tv_nsec = t1->tv_nsec + t2->tv_nsec;
	if(res->tv_nsec >= (1000*1000*1000)) {
		res->tv_nsec -= 1000*1000*1000;
		res->tv_sec++;
	}
}


/**
 * gettimeofday() system call.
 * Returns the current time since Epoch in tv and the timezone used in tz.
 * (for now, tz is not set by the implementation)
 */
struct timezone;
int sys_gettimeofday(struct timespec *tv, struct timezone *tz);


clock_t sys_times(struct tms *buf); 


#endif //_SYS_TIME_H
