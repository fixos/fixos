
/**
 * Place for arch-specific time related functions.
 */

#include <utils/types.h>
#include <sys/time.h>
#include <arch/sh/rtc.h>


// interrupt handler for RTC interrupt, update all the time subsystem
//static void periodic_rtc_interrupt();

void arch_time_init() {
		rtc_set_interrupt(&time_do_tick, RTC_PERIOD_256_HZ);
}



