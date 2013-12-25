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




#endif //_SYS_MEMORY_H
