#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <utils/types.h>
#include <arch/time.h>

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

void time_monotonic_time(struct hr_time *t);

void time_real_time(struct hr_time *t);


/**
 * Routine to call for each clock tick (add ellapsed time to counters,
 * manage process CPU time and other time-related events, like scheduler)
 */
void time_do_tick();


/**
 * Add time described in t1 and t2, and write the result in res.
 * res may points to the same location as t1 or t2.
 */
extern inline void time_add_hr(const struct hr_time *t1,
		const struct hr_time *t2, struct hr_time *res)
{
	res->sec = t1->sec + t2->sec;
	res->nano = t1->nano + t2->nano;
	if(res->nano >= (1000*1000*1000)) {
		res->nano -= 1000*1000*1000;
		res->sec++;
	}
}


/**
 * gettimeofday() system call.
 * Returns the current time since Epoch in tv and the timezone used in tz.
 * (for now, tz is not set by the implementation)
 */
struct timezone;
int sys_gettimeofday(struct hr_time *tv, struct timezone *tz);


clock_t sys_times(struct tms *buf); 


#endif //_SYS_TIME_H
