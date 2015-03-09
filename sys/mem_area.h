#ifndef _SYS_MEM_AREA_H
#define _SYS_MEM_AREA_H

#include <utils/types.h>
#include <utils/list.h>
#include <sys/memory.h>

/**
 * Generic memory-mapping interface, using segmentation-based mapping.
 */


// generic flags :
// type of area
#define MEM_AREA_TYPE_FILE		(1<<0)
#define MEM_AREA_TYPE_ANON		(1<<1)

// protections, flag is set if given operation is permited
#define MEM_AREA_PROT_R			(1<<3)
#define MEM_AREA_PROT_W			(1<<4)
#define MEM_AREA_PROT_X			(1<<5)

// flag used to implement .data and .bss in a single area : size from origin
// file is limited by infile_size, and if max_size is greater, fill with 0
#define MEM_AREA_PARTIAL		(1<<6)

struct mem_area_ops;

struct mem_area {
	int flags;

	// begin address of this area in process address space, and maximum size
	void *address;
	size_t max_size;

	// callback functions to manage this area
	const struct mem_area_ops *ops;

	// type-specific data (determined by flags)
	union {
		struct {
			// file structure of the file mapped to this area
			struct file *filep;

			// origin's position in the file
			size_t base_offset;

			// size of the data effectively found in file, if not equals to
			// max_size, bytes between them *should* be initialized as 0!
			size_t infile_size;
		} file;

		struct {
			// current size (useful in case of growing areas)
			//size_t size;
		} anon;

		// device type?
	};

	struct list_head list;
};


/**
 * mem_area_ops is used to map a specific implementation to a given memory area.
 * It is expected to be specialized by each filesystem that support file
 * mapping, and each device which need to do so.
 * Any NULL field is interpreted as using the default implementation if any.
 *
 * Creating the area is done using device/filesystem specific call.
 */
struct mem_area_ops {
	/**
	 * Called each time a page that should reside in the given area range is
	 * used, but not present in memory.
	 * This is the most important callback, which determine how this area works
	 * and what it initialy contains.
	 * The corresponding physical page is returned, or NULL to indicate and
	 * error.
	 */
	union pm_page (*area_pagefault)(struct mem_area *area, void *addr_fault);

	/**
	 * Change the size of the given area, to the given new_size.
	 * Non-zero should be returned on error, 0 else (and area->max_pos should
	 * be set correctly).
	 * Any page which is out of the area after decreasing size will be removed
	 * from the corresponding process address space by the caller.
	 */
	int (*area_resize)(struct mem_area *area, size_t new_size);
	

	/**
	 * Called when the area is no longer used.
	 */
	void (*area_release)(struct mem_area *area);

	/**
	 * Called when the area is duplicated, like during a fork.
	 */
	int (*area_duplicate)(struct mem_area *orig, struct mem_area *copy);
};


struct process;

/**
 * Initialize memory area subsystem.
 */
void mem_area_init();


/**
 * Return a newly allocated memory area, not initialized.
 * This function is mainly designed for helper usage.
 */
struct mem_area *mem_area_alloc();

/**
 * Free an allocated memory area, *without* any clean checkup!
 * Should not be used unless you are sure the given area is not valid,
 * or already clean.
 * Use mem_area_release() in other cases!
 */
void mem_area_free(struct mem_area *area);


/**
 * Helper to create an anonymous memory area, at given address.
 */
struct mem_area *mem_area_make_anon(void *vmaddr, size_t size);


/**
 * Look for the given address in given process address space, and return the
 * corresponding memory area.
 * NULL is returned if address don't belong to its address space.
 */
struct mem_area *mem_area_find(struct process *proc, void *address);


/**
 * Insert a memory area inside the given process' address space.
 * Return 0 if inserted successfully, negative value else (e.g. overlay on other
 * areas).
 */
int mem_area_insert(struct process *proc, struct mem_area *area);


/**
 * Generic function called when a page fault occurs, inside a given area address
 * range.
 * The implementation specific operation is called to set the corresponding page.
 */
static inline union pm_page mem_area_pagefault(struct mem_area *area,
		void *addrfault)
{
	if(area->ops != NULL && area->ops->area_pagefault != NULL) {
		return area->ops->area_pagefault(area, addrfault);
	}
	else {
		union pm_page page = {.private.ppn = 0, .private.flags = MEM_PAGE_PRIVATE};
		return page;
	}
}


/**
 * Release, and free if necessary, the given area, after calling the
 * implementation-specific release callback if any.
 */
static inline void mem_area_release(struct mem_area *area) {
	if(area->ops != NULL && area->ops->area_release != NULL)
		area->ops->area_release(area);
	mem_area_free(area);
}

/**
 * Resize an area to the given new_size.
 * If size is decreased and some memory pages associated with corresponding
 * process are excluded of the given area, each one is released.
 * TODO the design is not so good, process should be implicit?
 * Return 0 for success, negative value else.
 */
int mem_area_resize(struct mem_area *area, size_t new_size, struct process *proc);


/**
 * Duplicate an area.
 */
static inline struct mem_area *mem_area_clone(struct mem_area *area) {
	struct mem_area *ret = mem_area_alloc();
	if(ret != NULL) {
		*ret = *area;
		if(area->ops != NULL && area->ops->area_duplicate != NULL)
			area->ops->area_duplicate(area, ret);
	}
	return ret;
}


/**
 * Helper for filesystem implementations, to handle MEM_AREA_PARTIAL.
 * Check for a given page to fill with 0 when needed, and return the number of
 * bytes the underlying implementation should read from origin file.
 * 0 may be returned, if all the page is filled with zeros...
 */
size_t mem_area_fill_partial_page(struct mem_area *area, size_t offset, void *dest);


#endif //_SYS_MEM_AREA_H
