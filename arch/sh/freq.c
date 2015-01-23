#include "freq.h"
#include "7705.h"
#include "rtc.h"
#include "timer.h"
#include <utils/types.h>
#include <utils/log.h>


static unsigned int _freq_ckio_hz = 0;



int freq_change(int ckio_mul, int ifc, int pfc) {
	int new_stc = ckio_mul == FREQ_STC_SAME ? CPG.FRQCR.BIT.STC : ckio_mul & 0x3 ;
	int new_ifc = ifc == FREQ_DIV_SAME ? CPG.FRQCR.BIT.IFC : ifc & 0x3 ;
	int new_pfc = pfc == FREQ_DIV_SAME ? CPG.FRQCR.BIT._PFC : pfc & 0x3 ;


	if(new_stc != CPG.FRQCR.BIT.STC) {
		// set watchdog before changing multiplier!
		WDT.WTCSR.WRITE = 0xA587;
		WDT.WTCNT.WRITE = 0x5A80;
	}

	CPG.FRQCR.WORD = (CPG.FRQCR.WORD & 0xFC00) | (new_pfc<<0) | (new_ifc<<4) 
		| (new_stc<<8) ;

	//printk(LOG_DEBUG, "WTCNT = %d\n", WDT.WTCNT.READ);
	//TODO wait for watchdog if needed?
	
	return 0;
}


// TODO this function should not stop the RTC interrupts (direct consequence
// is system time not updated for 4-8 ticks)...
int freq_time_calibrate() {
	unsigned char old_rcr2;
	unsigned int tmu0_time;

	// take the control of the RTC for the calibration, disable interrupts
	// and exceptions, to use interrupt flag in a sequential way
	printk(LOG_DEBUG, "freq: Start calib...");

	interrupt_inhibit_all(1);
	old_rcr2 = RTC.RCR2.BYTE;

	timer_init_tmu0(0xFFFFFFFF, TIMER_PRESCALER_16, NULL);

	RTC.RCR2.BIT.PES = RTC_PERIOD_64_HZ;
	RTC.RCR2.BIT.PEF = 0;

	// wait for RTC interrupt (we just clear PEF flag, so our 1/64e seconds
	// will begin exactly the next time PEF will be set to 1)
	while(! RTC.RCR2.BIT.PEF);
	RTC.RCR2.BIT.PEF = 0;

	timer_start_tmu0(0);
	while(! RTC.RCR2.BIT.PEF);

	timer_stop_tmu0();
	tmu0_time = 0xFFFFFFFF - TMU0.TCNT;

	// prescaler == 16, in 1/64e seconds
	_freq_ckio_hz = tmu0_time * 64 * 16 
		* (CPG.FRQCR.BIT._PFC+1) / (CPG.FRQCR.BIT.STC+1) ;
	
	RTC.RCR2.BYTE = old_rcr2;
	
	interrupt_inhibit_all(0);

	printk(LOG_DEBUG, "done!\n");
	return 0;
}


unsigned int freq_get_ckio_hz() {
	return _freq_ckio_hz;
}


unsigned int freq_get_peripheral_hz() {
	return _freq_ckio_hz * (CPG.FRQCR.BIT.STC+1) / (CPG.FRQCR.BIT._PFC+1);
}


unsigned int freq_get_internal_hz() {
	return _freq_ckio_hz * (CPG.FRQCR.BIT.STC+1) / (CPG.FRQCR.BIT.IFC+1);
}
