#ifndef _SYS_MEMORY_H
#define _SYS_MEMORY_H

/**
 * Physical and virtual memory allocation and management functions, not
 * depending of architecture.
 * Most of the functions are designed as an interface to be implemented in
 * arch-specific code.
 */
#include <utils/types.h>

// TODO the only arch-dependant code is the next include, find a better way to
// do that
#include <arch/sh/memory_def.h>


// 32 pages per directory entry
#define MEM_DIRECTORY_ORDER		5
#define MEM_DIRECTORY_PAGES		(1<<MEM_DIRECTORY_ORDER)

struct shared_page;


/**
 * These flags is used to do some black magic : as we expect shared_page to be
 * 4-bytes aligned, we know the 2 LSB are set to 0 if pm_page is a shared_page
 * pointer.
 * So the flag MEM_PAGE_PRIVATE should be set only if pm_page is private.
 */
#define MEM_PAGE_PRIVATE	(1<<0)
#define MEM_PAGE_VALID		(1<<1)

#define MEM_PAGE_CACHED		(1<<2)
#define MEM_PAGE_DIRTY		(1<<3)

union pm_page {
	struct {
		uint32 ppn			:(32-PM_PAGE_ORDER);
		// flags should be the least significant bits!
#if PM_PAGE_ORDER > 10
		uint32 _reserved 	:(PM_PAGE_ORDER-10);
#endif
		uint32 flags		:10;
	} private;
	struct shared_page *shared;
};


// convert a page directory and index into the first byte of the given page
#define MEM_PAGE_ADDRESS(dir,index) \
	(void*)(( ((dir) << MEM_DIRECTORY_ORDER) + (index)) << PM_PAGE_ORDER)

#define MEM_ADDRESS_DIRECTORY(addr) \
	( (uint32)(addr) >> (PM_PAGE_ORDER + MEM_DIRECTORY_ORDER))

#define MEM_ADDRESS_INDEX(addr) \
	( ((uint32)(addr) >> (PM_PAGE_ORDER)) & (MEM_DIRECTORY_PAGES - 1))


/**
 * Process address space is managed by a linked list of "pages directory".
 * Each directory describe a set of sequential virtual pages, and the virtual
 * address of a page inside a directory is, considering the page's index :
 * ((dir_id << MEM_DIRECTORY_ORDER) + index) << PM_PAGE_ORDER
 * The directories are maintained as a linked list, ordered by dir_id (and
 * so ordered by virtual address too).
 */
struct page_dir {
	// most significant bits from virtual address :
	uint32 dir_id;
	struct page_dir *next;
	struct page_dir *prev;
	union pm_page pages[32];	
};


/**
 * Find the page containing given virtual address if included in dir_list,
 * return NULL if dir_list does not contain this page.
 */
union pm_page *mem_find_page(struct page_dir *dir_list, void *addr);


/**
 * Insert the given page data in dir_list, by erasing previous content for this
 * page address if any (else a new page_dir is allocated and added into the
 * dir_list, with all other pages marked as 'invalid').
 * Negative value is returned if insert failed.
 */
int mem_insert_page(struct page_dir **dir_list, union pm_page *page, void *addr);


int mem_copy_page(union pm_page *orig, struct page_dir **dir_list, void *addr);


/**
 * Release the given page (if underlying physical memory is not used anymore,
 * it will be free'd).
 */
void mem_release_page(union pm_page *page);


/**
 * Release the given directory page (free it) and all its pages.
 */
void mem_release_dir(struct page_dir *dir);

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
