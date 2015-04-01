/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
