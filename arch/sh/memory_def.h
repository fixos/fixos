#ifndef _ARCH_SH_MEMORY_DEF_H
#define _ARCH_SH_MEMORY_DEF_H

/**
 * Architecture and plateform specific definitions about memory.
 * TODO find a better way to manage that (this is not really arch-dependant, and
 * may be non constants on some plateforms?)
 */


// size of a physical page
#define PM_PAGE_BYTES 1024
// log2(PM_PAGE_BYTES)
#define PM_PAGE_ORDER 10

// get the physical page number from any (non translatable of course) address
// with this CPU, physical address space is 29bit length, so the 3rd bits are
// removed.
#define PM_PHYSICAL_PAGE(addr) ( ((int)(addr) & 0x1FFFFFFF) >> PM_PAGE_ORDER )
#define PM_PHYSICAL_ADDR(pagenum) ((void*) ((pagenum) << PM_PAGE_ORDER) )



#endif //_ARCH_SH_MEMORY_DEF_H
