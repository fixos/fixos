#include "rtc.h"
#include "7705.h"
#include <utils/types.h>
#include <sys/time.h>

#include <arch/generic/time.h>


void rtc_init() {
	RTC.RCR1.BYTE = 0x00;
	RTC.RCR2.BYTE = 0x0B; // reset, start and crystal run
}


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



// functions defined in arch/generic/time.h

int arch_time_get_hw(struct hr_time *t) {
	time_t ret;
	int tmp;
	int isleap;

	// used to compute days elapsed from begin of year to 1st day of a month
	const static unsigned short mdays_elapsed[12] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
	};
	const static unsigned short mdays_elapsed_leap[12] = {
		0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335
	};

	tmp = (RTC.RYRCNT.BIT.Y1000 * 1000 + RTC.RYRCNT.BIT.Y100 * 100
			+ RTC.RYRCNT.BIT.Y10 * 10 + RTC.RYRCNT.BIT.Y1);
	// POSIX.1 2001 formula interpretation to compute 100/400 leap years
	isleap = (tmp % 4 == 0 && tmp % 100 != 0) || (tmp % 400 == 0);
	tmp -= 1900;
	ret = (tmp - 70) * (3600*24*365)
		+ (tmp - 69)/4 * (3600*24)
		- (tmp - 1)/100 * (3600*24)
		+ (tmp + 299)/400 * (3600*24);

	tmp = (RTC.RMONCNT.BIT.M10 * 10 + RTC.RMONCNT.BIT.M1);
	tmp = tmp<1 ? 0 : tmp>12 ? 11 : tmp-1;
	ret += (isleap ? mdays_elapsed_leap[tmp] : mdays_elapsed[tmp]) * (3600*24);

	tmp = RTC.RDAYCNT.BIT.D10 * 10 + RTC.RDAYCNT.BIT.D1;
	ret += (tmp-1) * (3600*24);

	ret += ((RTC.RHRCNT.BIT.H10 * 10 + RTC.RHRCNT.BIT.H1) * 60 
			+ RTC.RMINCNT.BIT.M10 * 10 + RTC.RMINCNT.BIT.M1) * 60
			+ RTC.RSECCNT.BIT.S10 * 10 + RTC.RSECCNT.BIT.S1;

	t->sec = ret;
	t->nano = 0;

	return 0;
}

int arch_time_set_hw(const struct hr_time *t) {
	return -1;
}
