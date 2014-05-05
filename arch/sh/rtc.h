#ifndef _ARCH_SH_RTC_H
#define _ARCH_SH_RTC_H

/**
 * Misc functions for RTC usage.
 */

#include "interrupt.h"

// frequency for RTC periodic interrupt
#define RTC_PERIOD_DISABLE		0b000
#define RTC_PERIOD_256_HZ		0b001
#define RTC_PERIOD_64_HZ		0b010
#define RTC_PERIOD_16_HZ		0b011
#define RTC_PERIOD_4_HZ			0b100
#define RTC_PERIOD_2_HZ			0b101
#define RTC_PERIOD_1_HZ			0b110
#define RTC_PERIOD_0_5_HZ		0b111


/**
 * Initialialize hardware RTC.
 */
void rtc_init();


/**
 * Set the given callback to be called at periodic rate given by period (one
 * of the RTC_PERIOD_xxx).
 * If period is RTC_PERIOD_DISABLE, interrupt generation is disabled, and
 * the callback value doesn't matter.
 */
void rtc_set_interrupt(interrupt_callback_t callback, int period);


#endif //_ARCH_SH_RTC_H
