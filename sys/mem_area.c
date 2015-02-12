#include "mem_area.h"
#include <sys/process.h>
#include <utils/pool_alloc.h>
#include <fs/vfs_file.h>
#include <utils/log.h>


// pool allocation data
static struct pool_alloc _mem_area_pool = POOL_INIT(struct mem_area);


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


int mem_area_copy_raw(struct mem_area *area, size_t offset, void *dest, size_t size) {
	int ret = -1;

	if(area->flags & MEM_AREA_TYPE_ANON) {
		// anonymous area, the current implementation do nothing...
		if(offset + size > area->max_size) {
			printk(LOG_ERR, "mem_area: attempt to copy extra bytes (anon)\n");
		}
		else {
			if(offset + size > area->anon.size && (area->flags & MEM_AREA_MAYGROW)) {
				printk(LOG_DEBUG, "mem_area: extends area (%d->%d)\n",
						area->anon.size, offset + size);
			}
			ret = 0;
		}
	}
	else if(area->flags & MEM_AREA_TYPE_FILE) {
		// file mapped to memory, for now use a generic way to handle them
		// TODO improve this with map_area_ops struct to allow "override"
		size_t nbread;

		vfs_lseek(area->file.filep, area->file.origin + offset, SEEK_SET);
		nbread = vfs_read(area->file.filep, dest, size);
		ret = nbread == size ? 0 : -1;

		if(ret) {
			printk(LOG_ERR, "mem_area: failed loading %d bytes from offset 0x%x"
					" [absolute 0x%x] (read returns %d)\n",
					size, offset, area->file.origin + offset, nbread);
		}
		else {
			printk(LOG_DEBUG, "mem_area: loaded %d bytes @%p from file\n", size, dest);
		}

		//print_memory(LOG_DEBUG, dest, nbread);
	}

	return ret;
}

