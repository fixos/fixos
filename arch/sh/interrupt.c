#include "interrupt.h"
#include "interrupt_codes.h"
#include "7705_Casio.h"
#include "process.h"

#include <utils/log.h>
#include <utils/types.h>
#include <sys/process.h>
#include <sys/scheduler.h>


// Globals callbacks functions address :
interrupt_callback_t g_interrupt_callback[INT__NUMBER] = {NULL};

void interrupt_set_vbr(void *vbr)
{
      asm("mov %0, r2"::"r"(vbr):"r2");
      asm("ldc r2,vbr");
}


	
void* interrupt_get_vbr(void)
{
      void *ptr;
      asm("stc vbr,r2":::"r2");
      asm("mov.l r2, %0":"=m"(ptr));
      return ptr;
}



void arch_int_weak_atomic_block(int mode) {
	// use I3~I0 of SR register to set interrupt priorities
	int value = mode == 0 ? INTERRUPT_PVALUE_LOW-1 : INTERRUPT_PVALUE_CRITICAL-1;

	asm volatile (	"stc sr, r1;"
					"mov %0, r0;"
					"and #0xF, r0;"
					"shll2 r0;"
					"shll2 r0;"
					"mov #0xF0, r2;"
					"extu.b r2, r2;"
					"not r2, r2;"
					"and r2, r1;"
					"or r0, r1;"
					"ldc r1, sr;"
					: : "r"(value) : "r0", "r1", "r2");
}


// This function is used to map each interrupt with the corresponding
// callback function. These functions' addresses may be set through the
// set_callback() function.
// TODO write these routines in assembly!
//void interrupt_handler() __attribute__ ((interrupt_handler, section(".handler.interrupt")));
void interrupt_handler() __attribute__ ((section(".handler.interrupt")));

void interrupt_handler() {
	int evt = INTX.INTEVT2;	// interrupt code
	interrupt_callback_t handler;
	process_t *cur;


	switch(evt) {
	case INT_CODE_TMU0_TUNI0:
		handler = g_interrupt_callback[INT_TMU0];
		if (handler != NULL) handler();
		TMU.TSTR.BIT.STR0 = 0;
		TMU0.TCR.BIT.UNF=0;
		TMU.TSTR.BIT.STR0 = 1;
		break;
	case INT_CODE_TMU1_TUNI1:
		handler = g_interrupt_callback[INT_TMU1];
		if (handler != NULL) handler();
		TMU.TSTR.BIT.STR1 = 0;
		TMU1.TCR.BIT.UNF=0;
		TMU.TSTR.BIT.STR1 = 1;
		break;
	case INT_CODE_TMU2_TUNI2:
		handler = g_interrupt_callback[INT_TMU2];
		if (handler != NULL) handler();
		TMU.TSTR.BIT.STR2 = 0;
		TMU2.TCR.BIT.UNF=0;
		TMU.TSTR.BIT.STR2 = 1;
		break;
	case INT_CODE_PINT_0_7:	
		handler = g_interrupt_callback[INT_PINT_0_7];
		if (handler != NULL) handler();
		INTX.IRR0.BIT.PINT0R = 0;
		break;
	case INT_CODE_PINT_8_15:	
		handler = g_interrupt_callback[INT_PINT_8_15];
		if (handler != NULL) handler();
		INTX.IRR0.BIT.PINT1R = 0;
		break;
	case INT_CODE_USB_USI0:
	case INT_CODE_USB_USI1:
		handler = g_interrupt_callback[INT_USB];
		if (handler != NULL) handler();
		// TODO add flag clear?
		break;
	case INT_CODE_RTC_PRI:	
	case INT_CODE_RTC_ATI:
		handler = g_interrupt_callback[INT_RTC_PERIODIC];
		if (handler != NULL) handler();
		RTC.RCR2.BIT.PEF = 0;
		break;
		

	// for now only hard-coded for SDHI SD Interrupt
	case INT_CODE_SDHI_SDI:
		printk("SDI interrupt [0x%x]!\n", SDHI.word_u14 & 0x00B8);
		// TODO reset interrupt bit...

		SDHI.word_u14 &= 0xFF47;


		break;

	default:
		printk("Unknown interrupt catched :\nEVT2=0x%x\n", evt);
		break;
	}

	// do not return, do direct context switch, or preempt if needed
	sched_if_needed();
	// if sched_if_needed() returns, keep current process
	cur = process_get_current();
	if(arch_process_mode(cur) == 1) {
		signal_deliver_pending();
	}
	arch_kernel_contextjmp(cur->acnt, &(cur->acnt));

	return;
}


void interrupt_set_callback(unsigned int interruptID, interrupt_callback_t address) {
	if(interruptID < INT__NUMBER) g_interrupt_callback[interruptID] = address;
}

interrupt_callback_t interrupt_get_callback(unsigned int interruptID) {
	if(interruptID < INT__NUMBER) return g_interrupt_callback[interruptID];
	return (void*)0;
}



// defined in sys/interrupt.h
void interrupt_atomic_save(int *state) {
	unsigned int sr;
	asm volatile (	"stc sr, %0" : "=r"(sr) : : );
	*state = sr & 0x000000F0; // only I3-I0 bits
	// re-write SR with highest priority
	sr = sr | 0x000000F0;
	asm volatile (	"ldc %0, sr" : : "r"(sr) : );
}


void interrupt_atomic_restore(int state) {
	unsigned int sr;
	asm volatile (	"stc sr, %0" : "=r"(sr) : : );
	sr = (sr & (~0x000000F0) ) | (state & 0x000000F0);
	asm volatile (	"ldc %0, sr" : : "r"(sr) : );

}


int interrupt_in_atomic() {
	unsigned int sr;
	asm volatile (	"stc sr, %0" : "=r"(sr) : : );

	return ( sr & 0x000000F0) == 0xF0;
}
