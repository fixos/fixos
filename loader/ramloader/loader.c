#include "loader.h"
#include <arch/sh/virtual_memory.h>
#include <sys/memory.h>
#include <utils/log.h>

int ramloader_load(void *data, size_t datalen, process_t *dest)
{
	int i;
	unsigned int cur_ppn;
	unsigned int cur_vpn;
	int pc_offset;
	vm_page_t page;
	void *pageaddr;

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
	pageaddr = mem_pm_get_free_page(MEM_PM_CACHED);
	if(pageaddr == NULL) {
		printk("ramloader: no physical page\n");
		return -1;
	}

	page.ppn = PM_PHYSICAL_PAGE(pageaddr);
	// warning : the first valid stack address is in the previous page
	// of the 'base stack address'!
	page.vpn = VM_VIRTUAL_PAGE(ARCH_UNEWPROC_DEFAULT_STACK) - 1;
	vm_add_entry(&(dest->vm), &page);

	// set kernel stack address, for now any physical memory
	// TODO use virtual memory? (issue if TLB usage during exception handling)
	pageaddr = mem_pm_get_free_page(MEM_PM_CACHED);
	if(pageaddr == NULL) {
		printk("ramloader: no physical page\n");
		return -1;
	}


	// kernel stack begins at the end of pageaddr (contains the first
	// context struct)
	dest->kernel_stack = pageaddr + PM_PAGE_BYTES; 
	dest->acnt = dest->kernel_stack - sizeof(struct _context_info);
	dest->acnt->reg[15] = ARCH_UNEWPROC_DEFAULT_STACK;
	dest->acnt->pc = ARCH_UNEWPROC_DEFAULT_TEXT + pc_offset;
	dest->acnt->sr = ARCH_UNEWPROC_DEFAULT_SR;
	dest->acnt->previous = NULL;

	return 0;
}
