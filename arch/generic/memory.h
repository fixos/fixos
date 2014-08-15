#ifndef _ARCH_GENERIC_MEMORY_H
#define _ARCH_GENERIC_MEMORY_H

/**
 * Misc physical and virtual memory abstractions.
 */

#include <arch/memory.h>



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
void* arch_pm_get_free_page(int flags);


/**
 * Release a previously allocated physical page.
 * page *must* be exacly the same as the value returned by arch_pm_get_free_page()
 */
void arch_pm_release_page(void *page);

#endif //_ARCH_GENERIC_MEMORY_H
