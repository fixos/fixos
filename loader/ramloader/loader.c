#include "loader.h"
#include <arch/sh/virtual_memory.h>
#include <arch/sh/physical_memory.h>
#include <utils/log.h>

int ramloader_load(void *data, size_t datalen, process_t *dest)
{
	int i;
	unsigned int cur_ppn;
	unsigned int cur_vpn;
	int pc_offset;
	vm_page_t page;

	unsigned int kstack_ppn;
	
	vm_init_table(&(dest->vm));
	page.size = VM_PAGE_1K;
	page.cache = 1;
	page.valid = 1;

	// load virtual pages for process data :
	pc_offset = ((unsigned int)data % PM_PAGE_BYTES);
	cur_ppn = (PM_PHYSICAL_PAGE(data));
	cur_vpn = (VM_VIRTUAL_PAGE(ARCH_UNEWPROC_DEFAULT_TEXT));
	
	for(i=0; i<datalen; cur_ppn++, cur_vpn++) {
		page.ppn = cur_ppn;
		page.vpn = cur_vpn;
		vm_add_entry(&(dest->vm), &page);
		
		if(i==0)
			i = PM_PAGE_BYTES - pc_offset;
		else
			i += PM_PAGE_BYTES;
	}


	// alloc physical page and set it as the VM process stack
	if(pm_get_free_page(&cur_ppn)) {
		printk("ramloader: no physical page\n");
		return -1;
	}

	page.ppn = cur_ppn;
	// warning : the first valid stack address is in the previous page
	// of the 'base stack address'!
	page.vpn = VM_VIRTUAL_PAGE(ARCH_UNEWPROC_DEFAULT_STACK) - 1;
	vm_add_entry(&(dest->vm), &page);

	// set kernel stack address, for now any physical memory
	// TODO use virtual memory? (issue if TLB usage during exception handling)
	if(pm_get_free_page(&kstack_ppn)) {
		printk("ramloader: no physical page\n");
		return -1;
	}


	dest->acnt.kernel_stack = (void*)(0x80000000 + (unsigned int)PM_PHYSICAL_ADDR(kstack_ppn)); 
	dest->acnt.reg[15] = ARCH_UNEWPROC_DEFAULT_STACK;
	dest->acnt.pc = ARCH_UNEWPROC_DEFAULT_TEXT + pc_offset;
	dest->acnt.sr = ARCH_UNEWPROC_DEFAULT_SR;

	return 0;
}
