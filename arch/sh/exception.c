#include "exception.h"

#include "7705.h"
#include "interrupt_codes.h"
#include "mmu.h"


#include <sys/process.h>
#include <utils/log.h>

extern void syscall_entry();


void exception_handler() __attribute__ ((interrupt_handler, section(".handler.exception")));

void tlbmiss_handler() __attribute__ ((interrupt_handler, section(".handler.tlb")));


// Exception handler, since SR.MD bit is set to 1, all the calling code
// must be *ABSOLUTLY* exception-safe (if exception occurs, cpu may resets)
void exception_handler()
{
	int evt = INTC.EXPEVT;
	unsigned int tea = TEA;
	int tra;
	void *spcval;
	
	(void)(tea);
	asm volatile ("stc spc, %0" : "=r"(spcval) );

	switch(evt) {
	case EXP_CODE_ACCESS_READ:
	case EXP_CODE_ACCESS_WRITE:
		printk("Fatal:\nCPU Access Violation (R/W)\n  Adress : %p\n", (void*)tea);
		printk("SPC Value = %p\n", spcval);
		while(1);
		break;

	case EXP_CODE_TRAPA:
		tra = INTC.TRA >> 2; 
		printk("TRAPA (%d) catched!\n", tra);
		//TODO syscall_entry();
		break;

	case EXP_CODE_BAD_INSTR:
	case EXP_CODE_BAD_SLOTINSTR:
		printk("Fatal:\nIllegal %sinstruction.\n", evt == EXP_CODE_BAD_SLOTINSTR ?
				"slot " : "");
		printk("TEA value = %p\n", (void*)tea);
		printk("SPC Value = %p\n", spcval);
		while(1);
		break;

	case EXP_CODE_USER_BREAK:
		printk("Unexpected (blocking):\nUser Break exception.\n");
		while(1);
		break;

	case EXP_CODE_DMA_ERROR:
		printk("Fatal:\nDMA error.\n");
		while(1);
		break;

	case EXP_CODE_TLB_INITWRITE:
		printk("[I] Initial MMU page write.\n");
		break;

	case EXP_CODE_TLB_PROTECT_R:
	case EXP_CODE_TLB_PROTECT_W:
		printk("Fatal:\nTLB protection violation.\n");
		break;

	case EXP_CODE_TLB_READ:
	case EXP_CODE_TLB_WRITE:
		printk("Fatal:\nTLB error.\n");
		break;

	default:
		break;
	}
	
	asm volatile ("stc spc, %0" : "=r"(spcval) );
	printk("@ end of exception\nSPC Value = %p\n", spcval);

	return;
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
	curpr = process_from_asid(mmu_getasid());


	//printk("[I] process id = %d, ptr=%p\n[I] vpn = %d\n", mmu_getasid(), curpr, MMU.PTEH.BIT.VPN);
	// find the corresponding page, if exists
	page = vm_find_vpn(&(curpr->vm), MMU.PTEH.BIT.VPN);
	if(page != (void*)0) {
		unsigned int register flags;
		
		//fill flags, don't forget the dirty flag for now ;)
		flags = TLB_VALID | TLB_NOTSHARED | TLB_PROT_U_RW | TLB_DIRTY;
		if(page->size)
			flags |= TLB_SIZE_4K;
		if(page->cache)
			flags |= TLB_CACHEABLE;
		
		// load the TLB entry!
		mmu_tlb_fillload(page->ppn, flags);
		printk("Sucessfully mapped page!\nVirtual page @(%p)\n", (void*)(page->vpn << 10));
	}
	else
	{	
		int spcval = 0x1234;

		asm volatile ("stc spc, r0");
		asm volatile("mov r0, %0":"=r"(spcval)::);
		printk("Fatal:\nAccess to forbiden page.\nAt address %p\nWith PC=%p\n", (void*)TEA, (void*)spcval);
		while(1);
	}
}
