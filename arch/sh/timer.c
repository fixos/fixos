#include "timer.h"

//
// TMU0
//

void timer_init_tmu0(unsigned int time, int prescaler, interrupt_callback_t callback) {
	TMU.TSTR.BIT.STR0 = 0;	// Stop counting down
	TMU0.TCR.BIT.UNF = 0;
	TMU0.TCR.BIT.UNIE = 0;	// No longer interrupt from this TMU
	TMU0.TCOR = time;
	TMU0.TCNT = time;
	TMU0.TCR.BIT.CKEG = 0b01;
	TMU0.TCR.BIT.TPSC = prescaler;

	interrupt_set_callback(INT_TMU0, callback);
	INTC.IPRA.BIT._TMU0 = 0xF;
}


void timer_start_tmu0(int interrupting) {
	TMU.TSTR.BIT.STR0 = 0;
	if(interrupting)
		TMU0.TCR.BIT.UNIE = 1;
	TMU.TSTR.BIT.STR0 = 1;
}

void timer_stop_tmu0() {
	// Just stop tmu
	TMU.TSTR.BIT.STR0 = 0;
	TMU0.TCR.BIT.UNIE = 0;
}

/*
//
// TMU1
//

void init_tmu1(unsigned int time, int prescaler, interrupt_callback_t callback) {
	TMU.TSTR.BIT.STR1 = 0;	// Stop counting down
	struct st_tmu0 tmp;
	tmp.TCOR = time;
	tmp.TCNT = time;
	tmp.TCR.BIT.UNF = 0;
	tmp.TCR.BIT.UNIE = 0;
	tmp.TCR.BIT.CKEG = 0b01;
	tmp.TCR.BIT.TPSC = prescaler;

	TMU1 = tmp;

	interrupt_set_callback(INT_TMU1, callback);
	INTC.IPRA.BIT._TMU1 = 0xF;
}


void start_tmu1() {
	// Just start tmu
	TMU.TSTR.BIT.STR1 = 0;	// Stop counting down
	TMU1.TCR.BIT.UNIE = 1;
	TMU.TSTR.BIT.STR1 = 1;
}

void stop_tmu1() {
	// Just stop tmu
	TMU.TSTR.BIT.STR1 = 0;
	TMU1.TCR.BIT.UNIE = 0;
}


//
// TMU2
//

void init_tmu2(unsigned int time, int prescaler, interrupt_callback_t callback) {
	TMU.TSTR.BIT.STR2 = 0;	// Stop counting down
	struct st_tmu2 tmp;
	tmp.TCOR = time;
	tmp.TCNT = time;
	tmp.TCR.BIT.UNF = 0;
	tmp.TCR.BIT.UNIE = 0;
	tmp.TCR.BIT.ICPE = 0b00;
	tmp.TCR.BIT.CKEG = 0b01;
	tmp.TCR.BIT.TPSC = prescaler;

	TMU2 = tmp;

	interrupt_set_callback(INT_TMU2, callback);
	INTC.IPRA.BIT._TMU2 = 0xF;
}


void start_tmu2() {
	// Just start tmu
	TMU.TSTR.BIT.STR2 = 0;	// Stop counting down
	TMU2.TCR.BIT.UNIE = 1;
	TMU.TSTR.BIT.STR2 = 1;
}

void stop_tmu2() {
	// Just stop tmu
	TMU.TSTR.BIT.STR2 = 0;
	TMU2.TCR.BIT.UNIE = 0;
}
*/
