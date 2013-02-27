#include "exception.h"

#include "7705.h"
#include "interrupt_codes.h"

#include <sys/terminal.h>

extern void syscall_entry();


void exception_handler() __attribute__ ((interrupt_handler, section(".handler.exception")));


// Exception handler, since SR.MD bit is set to 1, all the calling code
// must be *ABSOLUTLY* exception-safe (if exception occurs, cpu may resets)
void exception_handler()
{
	int evt = INTC.EXPEVT;
	unsigned int tea = TEA;

	(void)(tea);

	switch(evt) {
	case EXP_CODE_ACCESS_READ:
	case EXP_CODE_ACCESS_WRITE:
		terminal_write("Fatal:\nCPU Access Violation (R/W)\n");
		while(1);
		break;

	case EXP_CODE_TRAPA:
		terminal_write("TRAPA catched!\n");
		//TODO syscall_entry();
		break;

	case EXP_CODE_BAD_INSTR:
	case EXP_CODE_BAD_SLOTINSTR:
		terminal_write("Fatal:\nIllegal instruction.\n");
		while(1);
		break;

	case EXP_CODE_USER_BREAK:
		break;

	case EXP_CODE_DMA_ERROR:
		break;

	case EXP_CODE_TLB_INITWRITE:
		break;

	case EXP_CODE_TLB_PROTECT_R:
	case EXP_CODE_TLB_PROTECT_W:
		break;

	case EXP_CODE_TLB_READ:
	case EXP_CODE_TLB_WRITE:
		break;

	default:
		break;
	}

}
