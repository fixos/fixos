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

#ifndef _SYS_MEMORY_H
#define _SYS_MEMORY_H

/**
 * Physical and virtual memory allocation and management functions, not
 * depending of architecture.
 * Most of the functions are designed as an interface to be implemented in
 * arch-specific code.
 */
#include <utils/types.h>

#include <arch/generic/memory.h>


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


// get the first byte's address of the physical page of the given address
#define MEM_PAGE_BEGINING(addr) \
	(void*)((uint32)(addr) & ~(PM_PAGE_BYTES-1))


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


#endif //_SYS_MEMORY_H
