#ifndef _SYS_USER_H
#define _SYS_USER_H

#include <sys/memory.h>
#include <sys/process.h>
#include <utils/types.h>

/**
 * Functions for reading/writing safely in a given process address space.
 */

extern inline uint32 user_read_32(void *addr, struct process *proc) {
	union pm_page *page;

	page = mem_find_page(proc->dir_list, addr);
	if(page != NULL) {
		//TODO shared
		uint32 *pmaddr = NULL;

		if(page->private.flags & MEM_PAGE_PRIVATE) {
			if(page->private.flags & MEM_PAGE_VALID) {
				pmaddr = PM_PHYSICAL_ADDR(page->private.ppn) 
					+ (uint32) P1_SECTION_BASE + ((uint32)addr)%PM_PAGE_BYTES;
			}
		}

		if(pmaddr != NULL) {
			return *pmaddr;
		}
	}

	return 0;
}

extern inline void user_write_32(void *addr, uint32 value, struct process *proc) {
	union pm_page *page;

	page = mem_find_page(proc->dir_list, addr);
	if(page != NULL) {
		//TODO shared
		uint32 *pmaddr = NULL;

		if(page->private.flags & MEM_PAGE_PRIVATE) {
			if(page->private.flags & MEM_PAGE_VALID) {
				pmaddr = PM_PHYSICAL_ADDR(page->private.ppn) 
					+ (uint32) P1_SECTION_BASE + ((uint32)addr)%PM_PAGE_BYTES;
			}
		}

		if(pmaddr != NULL) {
			printk(LOG_DEBUG, "userw32: %p\n", pmaddr);
			*pmaddr = value;
		}
	}
}
#endif //_SYS_USER_H
