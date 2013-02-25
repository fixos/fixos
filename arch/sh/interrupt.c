#include "interrupt.h"
#include "interrupt_codes.h"
#include "7705.h"
#include <stddef.h>



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

void interrupt_store_context(interrupt_priorities_t ipr_storage) {
	ipr_storage[0] = INTC.IPRA.WORD;
	ipr_storage[1] = INTC.IPRB.WORD;
	ipr_storage[2] = INTX.IPRC.WORD;
	ipr_storage[3] = INTX.IPRD.WORD;
	ipr_storage[4] = INTX.IPRE.WORD;
	ipr_storage[5] = INTX.IPRF.WORD;
	ipr_storage[6] = INTX.IPRG.WORD;
	ipr_storage[7] = INTX.IPRH.WORD;
}


void interrupt_restore_context(const interrupt_priorities_t ipr_storage) {
	INTC.IPRA.WORD = ipr_storage[0];
	INTC.IPRB.WORD = ipr_storage[1];
	INTX.IPRC.WORD = ipr_storage[2];
	INTX.IPRD.WORD = ipr_storage[3];
	INTX.IPRE.WORD = ipr_storage[4];
	INTX.IPRF.WORD = ipr_storage[5];
	INTX.IPRG.WORD = ipr_storage[6];
	INTX.IPRH.WORD = ipr_storage[7];
}
 

/*
// This function is used to map each interrupt with the corresponding
// callback function. These functions' addresses may be set through the
// set_callback() function.
// TODO write these routines in assembly!
void interrupt_handler() __attribute__ ((interrupt_handler));

void interrupt_handler() {
	int evt = INTX.INTEVT2;	// interrupt code

	char str[30];
	Bdisp_AllClr_VRAM();
	locate(1, 2);
	Print("Interrupt Handler");
	locate(1,3);
	Print("INTEVT =");
	Print(intToString(INTC.INTEVT, str));
	locate(1,4);
	Print("INTEVT2=");
	Print(intToString(evt, str));
	Bdisp_PutDisp_DD();

	switch(evt) {
	case INT_CODE_TMU0:
		if (g_handler_tmu0 != NULL) g_handler_tmu0();
		TMU.TSTR.BIT.STR0 = 0;
		TMU0.TCR.BIT.UNF=0;
		TMU.TSTR.BIT.STR0 = 1;
		break;
	case INT_CODE_TMU1:
		if (g_handler_tmu1 != NULL) g_handler_tmu1();
		TMU.TSTR.BIT.STR1 = 0;
		TMU1.TCR.BIT.UNF=0;
		TMU.TSTR.BIT.STR1 = 1;
		break;
	case INT_CODE_TMU2:
		if (g_handler_tmu2 != NULL) g_handler_tmu2();
		TMU.TSTR.BIT.STR2 = 0;
		TMU2.TCR.BIT.UNF=0;
		TMU.TSTR.BIT.STR2 = 1;
		break;	
	default:
		break;
	}
	return;
}*/


void interrupt_set_callback(unsigned int interruptID, interrupt_callback_t address) {
	if(interruptID < INT__NUMBER) g_interrupt_callback[interruptID] = address;
}

interrupt_callback_t interrupt_get_callback(unsigned int interruptID) {
	if(interruptID < INT__NUMBER) return g_interrupt_callback[interruptID];
	return (void*)0;
}
