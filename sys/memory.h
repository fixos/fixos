#ifndef _SYS_MEMORY_H
#define _SYS_MEMORY_H

/**
 * Physical and virtual memory allocation and management functions, not
 * depending of architecture.
 * Most of the functions are designed as an interface to be implemented in
 * arch-specific code.
 */

// TODO the only arch-dependant code is the next include, find a better way to
// do that
#include <arch/sh/memory_def.h>


// hints for physical memory page get
// ask for a cached or uncached memory area if possible
#define MEM_PM_CACHED		0
#define MEM_PM_UNCACHED		1


/**
 * Get any free page of physical memory.
 * flags is a combination of MEM_PM_xxx options. If MEM_PM_CACHED is used,
 * the allocated page will be in cached area if possible, but may not be
 * cached if cache is not supported by arch, or some other case.
 * Returns NULL if allocation fails.
 */
void* mem_pm_get_free_page(int flags);


/**
 * Release a previously allocated physical page.
 * page *must* be exacly the same as the value returned by mem_pm_get_free_page()
 */
void mem_pm_release_page(void *page);


struct _vm_page;

#define MEM_VM_MIRROR			0
#define MEM_VM_ALLOCATE_PAGE	1
#define MEM_VM_COPY_DATA		2
#define MEM_VM_COPY_ONWRITE		3

/**
 * Copy a virtual memory page.
 * Depending value of type, differents kind of operations may be done.
 * With MEM_VM_MIRROR, the physical address is the same as the original one
 * (so any modification one one will do the same on the other)
 * With MEM_VM_ALLOCATE_PAGE, a new physical page is allocated instead of
 * using the one of the original VM page.
 * With MEM_VM_COPY_DATA, in addition, the new physical page is filled with
 * values in the original one.
 * With MEM_VM_COPY_ONWRITE, do the same as MEM_VM_COPY_DATA but use
 * copy-on-write functionality if implemented (real copy of the page will
 * be done at the first write operation on any page)
 */
void mem_vm_copy_page(struct _vm_page *src, struct _vm_page *dest, int type);


#endif //_SYS_MEMORY_H
