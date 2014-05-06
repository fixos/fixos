#include "keymatrix.h"
#include <utils/log.h>
#include <arch/sh/7705.h>
#include <sys/stimer.h>
#include <sys/time.h>


// saved status, 10 lines of 7 bits (MSB unused)
static uint8 _hwkbd_status[10];

static hwkbd_key_handler _hwkbd_pressed = NULL;

static hwkbd_key_handler _hwkbd_released = NULL;


static void periodic_keycheck_timer(void *data) {
	(void)data;
	
	hwkbd_update_status();
	// set a new timer in aprox. 30ms
	stimer_add(&periodic_keycheck_timer, NULL, TICKS_MSEC_NOTNULL(30));
}


void hwkbd_init() {
	int i;
	for(i=0; i<10; i++)
		_hwkbd_status[i] = 0xFF;

	// we use PA as input (with pull-up MOS enabled), and PB/PM as output
	PFC.PBCR.WORD = 0x5555;
	PFC.PMCR.WORD = (0x5555 & 0x000F) | (PFC.PMCR.WORD & ~0x000F);	
	PFC.PACR.WORD = 0xAAAA;
}


void hwkbd_start_periodic_update() {
	// call it manually the first time
	// TODO manage current job pointer to ensure only 1 timer is used at a time
	// and to give a chance to disable it
	periodic_keycheck_timer(NULL);

}


// TODO better waiting control (depending of the clock speed...)
static volatile int __stupid_job;
static void micdelay(int t) {
	for(__stupid_job = 0; __stupid_job<t; __stupid_job++);
}


// FIXME in some cases (many keys pressed in a same line and in a same column)
// not-pressed key around seems to be pressed. This is an hardware issue, but
// there is maybe a way to avoid this problem?
void hwkbd_update_status() {
	int curline;

	// check each line :
	for(curline = 0; curline<10; curline++) {
		uint8 cols;
		uint8 xored;

		// set all the rows in input excepting the current one, in
		// output and set to value 0
		if(curline < 8) {
			PFC.PBCR.WORD = (0xFFFDFFFF>> (16-curline*2));
			PFC.PMCR.WORD = 0x000F | (PFC.PMCR.WORD & ~0x000F);	
			PM.DR.BYTE = 0x00;
		}
		else {
			PFC.PBCR.WORD = 0xFFFF;
 			PFC.PMCR.WORD = (curline == 8 ? 0x000D : 0x0007) | (PFC.PMCR.WORD & ~0x000F);	
			PB.DR.BYTE = 0x00;
		}

		micdelay(10);
		cols = PA.DR.BYTE;

		// use a XOR to check if any differences exist between old
		// and current state
		xored = cols ^ _hwkbd_status[curline];
		if(xored != 0x00) {
			int curcol;

			for(curcol = 0; curcol<7; curcol++) {
				if(xored & (1<<curcol)) {
					// check if the diff is a key pressed or released
					if( (cols & (1<<curcol)) && _hwkbd_released != NULL) {
						// if bit is now 1, the key is released
						_hwkbd_released( (curcol << 4) | curline);
					}
					else if(_hwkbd_pressed != NULL) {
						// if bit is now 0, the key is pressed
						_hwkbd_pressed( (curcol << 4) | curline);
					}
				}
			}
			_hwkbd_status[curline] = cols;
		}
	}
}


void hwkbd_set_kpressed_callback(hwkbd_key_handler handler) {
	_hwkbd_pressed = handler;
}



void hwkbd_set_kreleased_callback(hwkbd_key_handler handler) {
	_hwkbd_released = handler;
}



int hwkbd_keydown(int code) {
	int line = code & 0x0F;

	// do not forget, a 0 is a key pressed
	if(line < 10)
		return !(_hwkbd_status[line] & (1 << (code>>4)));

	return 0;
}



int hwkbd_real_keydown(int code) {
	int curline = code & 0x0F;

	// set all the rows in input excepting the current one, in
	// output and set to value 0
	if(curline < 8) {
		PFC.PBCR.WORD = (0xFFFDFFFF>> (16-curline*2));
		PFC.PMCR.WORD = 0x000F | (PFC.PMCR.WORD & ~0x000F);	
		PM.DR.BYTE = 0x00;
	}
	else {
		PFC.PBCR.WORD = 0xFFFF;
		PFC.PMCR.WORD = (curline == 8 ? 0x000D : 0x0007) | (PFC.PMCR.WORD & ~0x000F);	
		PB.DR.BYTE = 0x00;
	}

	micdelay(10);
	return !(PA.DR.BYTE & (1 << (code>>4)));
}
