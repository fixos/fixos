#include "mem_area.h"
#include <sys/process.h>
#include <utils/pool_alloc.h>
#include <fs/vfs_file.h>
#include <utils/log.h>
#include <utils/strutils.h>


// pool allocation data
static struct pool_alloc _mem_area_pool = POOL_INIT(struct mem_area);


// anonymous area operations
static union pm_page anon_area_pagefault(struct mem_area *area, void *addr_fault);

static int anon_area_resize(struct mem_area *area, const struct mem_area *new_area);

static void anon_area_release(struct mem_area *area);

static struct mem_area_ops _anon_area_ops = {
	.area_pagefault = anon_area_pagefault
};



void mem_area_init() {
	printk(LOG_DEBUG, "mem_area: area/page=%d\n", _mem_area_pool.perpage);
}


struct mem_area *mem_area_alloc() {
	struct mem_area *ret;

	ret = pool_alloc(&_mem_area_pool);
	if(ret != NULL) {
		ret->flags = 0;
		ret->address = NULL;
		ret->max_size = 0;
	}

	return ret;
}

void mem_area_free(struct mem_area *area) {
	pool_free(&_mem_area_pool, area);
}



void mem_area_set_anon(struct mem_area *area, void *vmaddr, size_t size) {
	// nothing more, for now...
	area->flags = MEM_AREA_TYPE_ANON;
	area->address = vmaddr;
	area->max_size = size;
	area->ops = &_anon_area_ops;
}


struct mem_area *mem_area_find(struct process *proc, void *address) {
	// for now, the list is not sorted, so check for each
	struct list_head *cur;
	struct mem_area *ret = NULL;

	list_for_each(cur, &(proc->mem_areas)) {
		// check if address is inside this area
		struct mem_area *area = container_of(cur, struct mem_area, list);
		if(address >= area->address && address < (area->address + area->max_size)) {
			ret = area;
			break;
		}
	}

	return ret;
}


/**
 * internal function, check if an area is defined in the given address range
 * Return 0 if this range is empty, 1 if an area exists.
 */
static int mem_area_check_range(struct process *proc, void *addr_begin, size_t size) {
	struct list_head *cur;

	list_for_each(cur, &(proc->mem_areas)) {
		// check if address range intersects with this area
		struct mem_area *area = container_of(cur, struct mem_area, list);
		if(area->address < (addr_begin + size)
				&& (area->address + area->max_size) > addr_begin)
			return 1;
	}

	return 0;
}


int mem_area_insert(struct process *proc, struct mem_area *area) {
	// check for any overlayed area
	if(!mem_area_check_range(proc, area->address, area->max_size)) {
		// not sorted, insert in front
		list_push_front(& proc->mem_areas, & area->list);
		return 0;
	}

	printk(LOG_ERR, "mem_area: unable to insert area (overlay)\n");
	return -1;
}


int mem_area_resize(struct mem_area *area, size_t new_size, struct process *proc) {
	int ret = 0;
	size_t old_size = area->max_size;

	// first, call area-specific resize callback if any
	if(area->ops != NULL && area->ops->area_resize != NULL)
		ret = area->ops->area_resize(area, new_size);
	else
		area->max_size = new_size;

	new_size = area->max_size;

	// if no error occurs and decreasing the size of the area, release uneeded
	// pages
	if(ret == 0 && area->max_size < old_size) {
		void *old_last_page = MEM_PAGE_BEGINING(area->address + old_size);
		void *new_last_page = MEM_PAGE_BEGINING(area->address + new_size);
		union pm_page *page;
		
		for( ; new_last_page < old_last_page; new_last_page += 1024) {
			printk(LOG_DEBUG, "mem_area: release page @%p\n", new_last_page);

			page = mem_find_page(proc->dir_list, new_last_page);
			if(page != NULL) {
				mem_release_page(page);
			}
		}
	}

	return ret;
}


size_t mem_area_fill_partial_page(struct mem_area *area, size_t offset, void *dest) {
	size_t readsize = PM_PAGE_BYTES;

	if(area->flags & MEM_AREA_TYPE_FILE && area->flags & MEM_AREA_PARTIAL) {
		// fill with 0 if needed
		if(area->file.infile_size < offset + readsize) {
			size_t zeroed_size;

			// partially
			if(area->file.infile_size > offset) {
				readsize = area->file.infile_size - offset;
				zeroed_size = (offset + PM_PAGE_BYTES) - area->file.infile_size;
			}
			else {
				zeroed_size = readsize;
				readsize = 0;
			}

			memset(dest + readsize, 0, zeroed_size);
		}
	}

	return readsize;
}


// anonymous area operations :
static union pm_page anon_area_pagefault(struct mem_area *area, void *addr_fault) {
	union pm_page pmpage = { .private.ppn = 0, .private.flags = MEM_PAGE_PRIVATE };
	size_t offset = addr_fault - area->address;
	//size_t size = PM_PAGE_BYTES;

	// anonymous area, the current implementation only allocate a page
	if(offset < area->max_size) {
		void *pmaddr;

		// allocate a physical memory page
		// FIXME UNCACHED due to temporary hack to be sure nothing is retained in cache
		pmaddr = arch_pm_get_free_page(MEM_PM_UNCACHED);
		if(pmaddr != NULL) {
			pmpage.private.ppn = PM_PHYSICAL_PAGE(pmaddr);
			pmpage.private.flags = MEM_PAGE_PRIVATE | MEM_PAGE_VALID; // | MEM_PAGE_CACHED;
		}
		// FIXME what to do if out of memory?
	}
	else {
		printk(LOG_ERR, "mem_area: attempt to copy extra bytes (anon)\n");
	}

	return pmpage;
}
