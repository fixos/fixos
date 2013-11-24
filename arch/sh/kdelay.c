#include "kdelay.h"
#include "7705_Casio.h"


// sleep time of kdelay
#define KDELAY_USECONDS_TIME	140


void kdelay(int time) {
	unsigned short counter;

	time = time <= 0 ? 1 : (time > 40 ? 40 : time);

	counter = (time * 92) / 16; // shift right 4
	counter = (~counter) & 0xFF; // byte size complement

	INTC.IPRB.BIT._WDT = 0; // inhibit wachdog interrupt


	WDT.WTCSR.WRITE = 0xA500; // write 0x00

	WDT.WTCNT.WRITE = 0xA500 | counter; // write counter in counter

	WDT.WTCSR.WRITE = 0xA505; // CLOCK_DIVIDE_256
	WDT.WTCSR.WRITE = 0xA585; // CLOCK_DIVIDE_256 | TIMER_ENABLED

	
	while( ((WDT.WTCSR.READ.BYTE) & 0x08) == 0); // WDT_OVERFLOW_OCCURS

	WDT.WTCSR.WRITE = (((WDT.WTCSR.READ.BYTE) & 0xF7) | 0xA500); // clear WDT_OVERFLOW_OCCURS
	WDT.WTCSR.WRITE = 0xA500;

	WDT.WTCNT.WRITE = 0x5A00;
}



void kusleep(int time) {
	int kdelay_units = time / KDELAY_USECONDS_TIME;

	while(kdelay_units > 0) {
		int nbunits = kdelay_units < 40 ? kdelay_units : 40;
		kdelay(nbunits);
		kdelay_units -= nbunits;
	}
}
