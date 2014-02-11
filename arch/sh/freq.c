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

	//printk("WTCNT = %d\n", WDT.WTCNT.READ);
	//TODO wait for watchdog if needed?
	
	return 0;
}



static volatile int _freq_calib_state;
static volatile unsigned int _freq_calib_tmu0_time;

static void _freq_rtc_calib() {
	if(_freq_calib_state == 0) {
		// use TMU as a counter, we do not expect any underflow
		timer_init_tmu0(0xFFFFFFFF, TIMER_PRESCALER_16, NULL);
		timer_start_tmu0(0);
	}
	else if(_freq_calib_state == 1) {
		timer_stop_tmu0();
		_freq_calib_tmu0_time = 0xFFFFFFFF - TMU0.TCNT;
	}
	_freq_calib_state++;
}


int freq_time_calibrate() {

	_freq_calib_tmu0_time = 0;
	_freq_calib_state = 0;

	printk("freq: Start calib...");
	rtc_set_interrupt(&_freq_rtc_calib, RTC_PERIOD_64_HZ);

	// wait for 2 consecutive RTC interrupt
	while(_freq_calib_state < 2);
	rtc_set_interrupt(NULL, RTC_PERIOD_DISABLE);
	printk("done!\n");

	// prescaler == 16, in 1/64e seconds
	_freq_ckio_hz = _freq_calib_tmu0_time * 64 * 16 
		* (CPG.FRQCR.BIT._PFC+1) / (CPG.FRQCR.BIT.STC+1) ;
	
	//printk("TMU0: 16Hz count %d ticks.\nEstimated freq = %d.%dMHz\n", tmu0_time, freq/1000000, (freq/100000)%10);
	
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
