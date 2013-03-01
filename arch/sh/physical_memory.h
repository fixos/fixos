#ifndef _ARCH_SH_PHYSICAL_MEMORY_H
#define _ARCH_SH_PHYSICAL_MEMORY_H

/**
 * Utilities for physical memory manipulation.
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

/**
 * Linked list formed using this struct in the begining of each free page.
 */
struct _pm_freepage_head {
	struct _pm_freepage_head *next; // next free page head
};

typedef struct _pm_freepage_head pm_freepage_head_t;


/**
 * Initialize all the physical RAM (only the RAM!), constructing the free
 * pages linked list.
 */
void pm_init_pages();


/**
 * Try to obtain a free page, if success, ppn will be set to the physical page
 * number and the return will be 0.
 * Else, return -1
 * TODO : allocation of physical page*s* (like the linux way? -> a list for each
 * page order, with split and merge...)
 */
int pm_get_free_page(unsigned int *ppn);

/**
 * Free the given page (WARNING : no verifiactions are done here, be careful...)
 */
void pm_free_page(unsigned int ppn);


#endif //_ARCH_SH_PHYSICAL_MEMORY_H
