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

#include "memory.h"
#include <utils/pool_alloc.h>
#include <utils/strutils.h>


// memory pool for Page Directory
static struct pool_alloc _dir_pool = POOL_INIT(struct page_dir);


union pm_page *mem_find_page(struct page_dir *dir_list, void *addr) {
	struct page_dir *cur;
	uint32 dir_id;

	dir_id = MEM_ADDRESS_DIRECTORY(addr);
	// dir_list is ordered
	for(cur = dir_list; cur != NULL && cur->dir_id < dir_id; cur = cur->next);

	if(cur != NULL && cur->dir_id == dir_id)
		return &(cur->pages[MEM_ADDRESS_INDEX(addr)]);

	return NULL;
}



int mem_insert_page(struct page_dir **dir_list, union pm_page *page, void *addr) {
	struct page_dir *cur;
	uint32 dir_id;

	dir_id = MEM_ADDRESS_DIRECTORY(addr);
	for(cur = *dir_list; cur != NULL && cur->dir_id < dir_id
			&& cur->next != NULL; cur = cur->next);

	if(cur == NULL || cur->dir_id != dir_id) {
		// we need to create a new page_dir and to insert it before the current
		struct page_dir *newdir;
		int i;

		newdir = pool_alloc(&_dir_pool);
		if(newdir == NULL)
			return -1;

		// prepare new page dir
		newdir->dir_id = dir_id;
		for(i=0; i<MEM_DIRECTORY_PAGES; i++)
			newdir->pages[i].private.flags = MEM_PAGE_PRIVATE; // no 'valid' flag
		
		// insert it
		if(cur == NULL) {
			newdir->next = NULL;
			newdir->prev = NULL;
			*dir_list = newdir;
		}
		else {
			if(cur->dir_id > dir_id) {
				newdir->next = cur;
				newdir->prev = cur->prev;
				if(cur->prev == NULL)
					*dir_list = newdir;
				else
					cur->prev->next = newdir;
				cur->prev = newdir;
			}
			else {
				newdir->next = cur->next;
				newdir->prev = cur;
				if(cur->next != NULL)
					cur->next->prev = newdir;
				cur->next = newdir;
			}
		}

		cur = newdir;
	}

	cur->pages[MEM_ADDRESS_INDEX(addr)] = *page;
	return 0;
}


int mem_copy_page(union pm_page *orig, struct page_dir **dir_list, void *addr) {
	union pm_page page;

	if(orig->private.flags & MEM_PAGE_PRIVATE) {
		void *dest;

		page.private.flags = orig->private.flags;

		dest = arch_pm_get_free_page(page.private.flags & MEM_PAGE_CACHED ?
				MEM_PM_CACHED : MEM_PM_UNCACHED);

		if(dest != NULL) {
			void *origaddr;

			// copy the content from the original page
			if(page.private.flags & MEM_PAGE_CACHED)
				origaddr = P1_SECTION_BASE;
			else
				origaddr = P2_SECTION_BASE;


			memcpy(dest, origaddr + (unsigned int)PM_PHYSICAL_ADDR(orig->private.ppn), PM_PAGE_BYTES);

			page.private.ppn = PM_PHYSICAL_PAGE(dest);

			mem_insert_page(dir_list, &page, addr);
			return 0;
		}
	}

	return -1;
}



void mem_release_page(union pm_page *page) {
	if((page->private.flags & MEM_PAGE_PRIVATE)
			&& (page->private.flags & MEM_PAGE_VALID) )
	{
		// maybe a flag that avoid a page content to be free'd is a good idea?
		arch_pm_release_page(PM_PHYSICAL_ADDR(page->private.ppn));
		page->private.flags &= ~MEM_PAGE_VALID;
	}
	else {
		// TODO shared page
	}
}



void mem_release_dir(struct page_dir *dir) {
	int i;
	for(i=0; i<MEM_DIRECTORY_PAGES; i++)
		mem_release_page(& dir->pages[i]);

	pool_free(&_dir_pool, dir);
}
