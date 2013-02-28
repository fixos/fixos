#include "exception.h"

#include "7705.h"
#include "interrupt_codes.h"
#include "mmu.h"


#include <sys/terminal.h>
#include <sys/process.h>

extern void syscall_entry();


void exception_handler() __attribute__ ((interrupt_handler, section(".handler.exception")));

void tlbmiss_handler() __attribute__ ((interrupt_handler, section(".handler.tlb")));


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
		terminal_write("[I] Initial MMU page write.\n");
		break;

	case EXP_CODE_TLB_PROTECT_R:
	case EXP_CODE_TLB_PROTECT_W:
		terminal_write("Fatal:\nTLB protection violation.\n");
		break;

	case EXP_CODE_TLB_READ:
	case EXP_CODE_TLB_WRITE:
		terminal_write("Fatal:\nTLB error.\n");
		break;

	default:
		break;
	}
}



/**
 * This handler is very important when Virtual Memory, it has to check
 * if the current process could or not access to the given page, and to load
 * the corresponding TLB entry faster than possible.
 */
void tlbmiss_handler()
{
	// for now, no difference between read and write miss
	// TODO : it's possible to be faster by using TTB register
	// to store the table for the current process (but that assume only 1 task at a time)
	vm_page_t *page;
	process_t *curpr;
	
	// find the process wich cause the miss :
	curpr = process_from_asid(MMU.PTEH.BIT.ASID);

	// find the corresponding page, if exists
	page = vm_find_vpn(&(curpr->vm), MMU.PTEH.BIT.VPN);
	if(page != (void*)0) {
		unsigned int register flags;
		
		//fill flags
		flags = TLB_VALID | TLB_NOTSHARED | TLB_PROT_U_RW;
		if(page->size)
			flags |= TLB_SIZE_4K;
		if(page->cache)
			flags |= TLB_CACHEABLE;

		// load the TLB entry!
		mmu_tlb_fillload(page->ppn, flags);
		terminal_write("Sucessfully mapped page!i\n");
	}
	else
	{	
		terminal_write("Fatal:\nAccess to forbiden page.\n");
		while(1);
	}
}
