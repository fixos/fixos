#include "exception.h"

#include "7705.h"
#include "interrupt_codes.h"
#include "mmu.h"
#include "interrupt.h"
#include <arch/generic/process.h>


#include <sys/scheduler.h>
#include <sys/process.h>
#include <sys/syscall.h>
#include <sys/memory.h>
#include <utils/log.h>
#include <sys/kdebug.h>
#include <sys/mem_area.h>


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
	struct process *cur;
	
	(void)(tea);
	asm volatile ("stc spc, %0" : "=r"(spcval) );
	asm volatile ("mov r15, %0" : "=r"(stackval) );

	cur = process_get_current();

	switch(evt) {
	case EXP_CODE_ACCESS_READ:
	case EXP_CODE_ACCESS_WRITE:
		printk(LOG_ERR, "> Address : %p\n", (void*)tea);
		printk(LOG_ERR, "> SPC Value = %p\n", spcval);
		kdebug_oops("CPU Access Violation (R/W)");
		break;

	case EXP_CODE_TRAPA:
		tra = INTC.TRA >> 2; 
		//printk(LOG_DEBUG, "TRAPA (%d) catched!\n", tra);
		{
			void *func;
			func = syscall_get_function(tra);
			if(func == NULL) {
				printk(LOG_DEBUG, "Not a reconized syscall!\n");
			}
			else {
				int *context = (void*)(cur->acnt);
				// set BL bit or status register, allowing interrupt/exception
				// to be generated inside a syscall processing!
				interrupt_inhibit_all(0);

				// call given function, and store return value in context-saved r0
				asm volatile ("mov %0, r0;"
						"mov.l @(16, r0), r4;"
						"mov.l @(20, r0), r5;"
						"mov.l @(24, r0), r6;"
						"mov.l @(28, r0), r7;"
						"jsr @%1;"
						"nop;"
						"mov.l r0, @(0, %0);" : : "r"(context), "r"(func) : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r15");

				interrupt_inhibit_all(1);

			}
		}
		break;

	case EXP_CODE_BAD_INSTR:
	case EXP_CODE_BAD_SLOTINSTR:
		printk(LOG_ERR, "> TEA value = %p\n", (void*)tea);
		printk(LOG_ERR, ">   *TEA = (%p)\n", (void*)(*(int*)(tea-(tea%4))));
		printk(LOG_ERR, "> SPC Value = %p\n", spcval);
		if(EXP_CODE_BAD_SLOTINSTR)
			kdebug_oops("Illegal slot instruction");
		else
			kdebug_oops("Illegal instruction");
		break;

	case EXP_CODE_USER_BREAK:
		printk(LOG_ERR, "Unexpected (blocking):\nUser Break exception.\n");
		while(1);
		break;

	case EXP_CODE_DMA_ERROR:
		kdebug_oops("DMA error");
		break;

	case EXP_CODE_TLB_INITWRITE:
		printk(LOG_DEBUG, "Initial MMU page write.\n");
		break;

	case EXP_CODE_TLB_PROTECT_R:
	case EXP_CODE_TLB_PROTECT_W:
		kdebug_oops("TLB protection violation");
		break;

	// TLB contains the needed address, but V bit is 0
	// These two codes are re-directed to tlbmiss_handler, and should never happen
	case EXP_CODE_TLB_READ:
	case EXP_CODE_TLB_WRITE:
		kdebug_oops("TLB miss");
		break;

	default:
		break;
	}
	
	asm volatile ("stc spc, %0" : "=r"(spcval) );

	// avoid verbosity for TRAPA
	if(evt != EXP_CODE_TRAPA)
		printk(LOG_DEBUG, "@ end of exception\nSPC Value = %p\n", spcval);

	// do not return, do direct context switch
	sched_if_needed();
	if(arch_process_mode(cur) == 1) {
		signal_deliver_pending();
	}
	// this line is reached only if sched was not needed and no pending signal
	arch_kernel_contextjmp(cur->acnt, &(cur->acnt));
}



/**
 * This handler is very important for Virtual Memory, it has to check
 * if the current process could or not access to the given page, and to load
 * the corresponding TLB entry faster than possible.
 */
void tlbmiss_handler()
{
	// for now, no difference between read and write miss
	// TODO : it's possible to be faster by using TTB register
	union pm_page *page;
	uint32 vpn;
	struct process *curpr;
	
	// the process which cause the TLB miss should be the current one
	curpr = _proc_current;

	// find the corresponding page, if exists
	vpn = MMU.PTEH.BIT.VPN; 
	page = mem_find_page(curpr->dir_list,
			(void*)(vpn << PM_PAGE_ORDER) );

	// if page is not in dir list (or is invalid), maybe it exists in memory
	// area (allocate it)
	// FIXME not working for shared pages
	if(page == NULL || !(page->private.flags & MEM_PAGE_VALID))  {
		struct mem_area *area;
		void *virtaddr;
		
		virtaddr = PM_PHYSICAL_ADDR(vpn);
		area = mem_area_find(curpr, virtaddr);
		if(area != NULL) {
			union pm_page pmpage;

			// 'major' page fault, create and fill it
			pmpage = mem_area_pagefault(area, virtaddr);
			if(pmpage.private.ppn != 0) {
				mem_insert_page(& curpr->dir_list, &pmpage, virtaddr);

				// not optimized, but ensure page points to a valid page struct
				page = mem_find_page(curpr->dir_list, virtaddr);

				printk(LOG_DEBUG, "tlb major fault: page inserted (virt %p -> phy %p)\n",
						virtaddr, PM_PHYSICAL_ADDR(page->private.ppn));
			}
		}
	}

	if(page != NULL) {
		unsigned int flags;
		uint32	ppn = 0;

		//fill flags, don't forget the dirty flag for now ;)
		flags = TLB_NOTSHARED | TLB_PROT_U_RW | TLB_DIRTY;

		if(page->private.flags & MEM_PAGE_PRIVATE) {
				if(page->private.flags & MEM_PAGE_VALID) {
					flags |= TLB_VALID;
					ppn = page->private.ppn;

					if(page->private.flags & MEM_PAGE_CACHED)
						flags |= TLB_CACHEABLE;
				}
				else {
					// not a valid page
				}
		}
		else {
			// TODO shared page
		}

		
		if(flags & TLB_VALID) {
			// load the TLB entry!
			mmu_tlb_fillload(ppn, flags);
			/*
			printk(LOG_DEBUG, "vm: [pid %d] page tr #(%p)->@(%p)\n", curpr->pid,
					(void*)(ppn << PM_PAGE_ORDER),
					(void*)(vpn << PM_PAGE_ORDER));
			*/
		}
		else {
			page = NULL; // temp stuff to have an error
		}
	}

	if(page == NULL) {	
		int spcval;
		void *stack;

		asm volatile("stc spc, %0":"=r"(spcval));
		asm volatile("mov r15, %0":"=r"(stack));
		printk(LOG_ERR, "> Dereference %p\n> With PC=%p\n",
				(void*)TEA, (void*)spcval);
		kdebug_oops("Access to a forbiden page");
	}
}
