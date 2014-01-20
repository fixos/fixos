#include "exception.h"

#include "7705.h"
#include "interrupt_codes.h"
#include "mmu.h"
#include "interrupt.h"


#include <sys/process.h>
#include <syscalls/arch/syscall.h>
#include <utils/log.h>

extern void syscall_entry();

/*
 * This variable is used to store/save r0~r7 of user process in case of trapa exception.
 * 
 */
int *g_user_registers = NULL; 

// used to refer to last stack-saved BANK0 register (array of {r0, r1, ..., r7}
extern int *_bank0_context;


//void exception_handler() __attribute__ ((interrupt_handler, section(".handler.exception")));
void exception_handler() __attribute__ ((section(".handler.exception")));

//void tlbmiss_handler() __attribute__ ((interrupt_handler, section(".handler.tlb")));
void tlbmiss_handler() __attribute__ ((section(".handler.tlb")));


// Exception handler, since SR.BL bit is set to 1, all the calling code
// must be *ABSOLUTLY* exception-safe (if exception occurs, cpu may resets)
void exception_handler()
{
	int evt = INTC.EXPEVT;
	unsigned int tea = TEA;
	int tra;
	void *spcval;
	void *stackval;
	
	(void)(tea);
	asm volatile ("stc spc, %0" : "=r"(spcval) );
	asm volatile ("mov r15, %0" : "=r"(stackval) );

	switch(evt) {
	case EXP_CODE_ACCESS_READ:
	case EXP_CODE_ACCESS_WRITE:
		printk("Fatal:\nCPU Access Violation (R/W)\n  Address : %p\n", (void*)tea);
		printk("SPC Value = %p\nStack = %p\n", spcval, stackval);
		while(1);
		break;

	case EXP_CODE_TRAPA:
		tra = INTC.TRA >> 2; 
		//printk("TRAPA (%d) catched!\n", tra);
		//TODO syscall_entry();

		{
			void *func;
			func = syscall_get_function(tra);
			if(func == NULL) {
				printk("Not a reconized syscall!\n");
			}
			else {
				// set BL bit or status register, allowing interrupt/exception
				// to be generated inside a syscall processing!
				interrupt_inhibit_all(0);
				int **bank0_context_ptr = &_bank0_context;

				// call given function
				asm volatile ("mov.l @%0, r0;"
						"mov.l @(16, r0), r4;"
						"mov.l @(20, r0), r5;"
						"mov.l @(24, r0), r6;"
						"mov.l @(28, r0), r7;"
						"jsr @%1;"
						"nop;"
						"mov.l @%0, r1;"
						"mov.l r0, @(0, r1);" : : "r"(bank0_context_ptr), "r"(func) : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r15");

				interrupt_inhibit_all(1);

			}
		}
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
		while(1);
		break;

	case EXP_CODE_TLB_READ:
	case EXP_CODE_TLB_WRITE:
		printk("Fatal:\nTLB error.\n");
		while(1);
		break;

	default:
		break;
	}
	
	asm volatile ("stc spc, %0" : "=r"(spcval) );

	// avoid verbosity for TRAPA
	if(evt != EXP_CODE_TRAPA)
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
		int spcval;
		void *stack;

		asm volatile("stc spc, %0":"=r"(spcval));
		asm volatile("mov r15, %0":"=r"(stack));
		printk("Fatal:\nAccess to forbiden page.\nAt address %p\nWith PC=%p\nStack=%p\n", (void*)TEA, (void*)spcval, stack);
		while(1);
	}
}
