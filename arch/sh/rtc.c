#include "rtc.h"
#include "7705.h"
#include <utils/types.h>


void rtc_set_interrupt(interrupt_callback_t callback, int period) {
	if(period < 0 || period > 7) {

	}
	else {
		RTC.RCR2.BIT.PES = period;
		if(period == RTC_PERIOD_DISABLE) {
			INTERRUPT_PRIORITY_RTC = 0;
			interrupt_set_callback(INT_RTC_PERIODIC, NULL);
		}
		else {
			interrupt_set_callback(INT_RTC_PERIODIC, callback);
			INTERRUPT_PRIORITY_RTC = INTERRUPT_PVALUE_HIGH;
		}
		RTC.RCR2.BIT.PEF = 0;
	}
}
