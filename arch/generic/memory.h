/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ARCH_GENERIC_MEMORY_H
#define _ARCH_GENERIC_MEMORY_H

/**
 * Misc physical and virtual memory abstractions.
 */

/**
 * Should export many memory-related macros, and the "struct addr_space" data structure
 * for implementing address spaces.
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


/**
 * Virtual memory management functions.
 * We assume there is a concept of "Address Space" for a machine with MMU.
 * Address Space is totaly abstracted to allow any kind of hardware implementation.
 * For a sh3/4, the hardware support an Address Space IDentifier (ASID) to speed
 * up MMU context switch, but if this is not supported in hardware, the "minimalist"
 * way to implements this is a software identifier, with full TLB flush each time
 * the address space should change (to avoid inferences with old TLB entries).
 */

/**
 * Initialize a new address space.
 */
int arch_adrsp_init(struct addr_space *adrsp);


/**
 * Switch from the current address space to the given one.
 * If needed, adrsp may change to have a "clean" address space identifier.
 */
int arch_adrsp_switch_to(struct addr_space *adrsp);


/**
 * Release an address space (no more usage of the represented address space).
 */
int arch_adrsp_release(struct addr_space *adrsp);

#endif //_ARCH_GENERIC_MEMORY_H
