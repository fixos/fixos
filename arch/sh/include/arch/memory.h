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

#ifndef _ARCH_SH_INCLUDE_MEMORY_H
#define _ARCH_SH_INCLUDE_MEMORY_H

/**
 * Architecture and plateform specific definitions about memory.
 * TODO find a better way to manage that (this is not really arch-dependant, and
 * may be non constants on some plateforms?)
 */

// address space data (for now only ASID)
struct addr_space {
	unsigned char asid;
};

// log2(PM_PAGE_BYTES)
#define PM_PAGE_ORDER 10
// size of a physical page
#define PM_PAGE_BYTES (1<<PM_PAGE_ORDER)

// get the physical page number from any (non translatable of course) address
// with this CPU, physical address space is 29bit length, so the 3rd bits are
// removed.
#define PM_PHYSICAL_PAGE(addr) ( ((int)(addr) & 0x1FFFFFFF) >> PM_PAGE_ORDER )
#define PM_PHYSICAL_ADDR(pagenum) ((void*) ((pagenum) << PM_PAGE_ORDER) )

// realy arch-dependant :(
// P1 is cacheable and non-translatable
#define P1_SECTION_BASE ((void*)0x80000000)
// P2 is non-cacheable and non-translatable
#define P2_SECTION_BASE ((void*)0xA0000000)
#define SECTION_OFFSET(addr) ((unsigned int)(addr) & 0x1FFFFFFF)




#endif //_ARCH_SH_INCLUDE_MEMORY_H
