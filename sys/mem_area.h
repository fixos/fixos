#ifndef _SYS_MEM_AREA_H
#define _SYS_MEM_AREA_H

#include <utils/types.h>
#include <utils/list.h>

/**
 * Generic memory-mapping interface, using segmentation-based mapping.
 */


// generic flags :
// type of area
#define MEM_AREA_TYPE_FILE		(1<<0)
#define MEM_AREA_TYPE_ANON		(1<<1)

// some kind of area are allowed to grow
#define MEM_AREA_MAYGROW		(1<<2)
// flag used if area grows bottom-up instead of top-down (which is the default)
#define MEM_AREA_GROW_UP		(1<<3)


struct mem_area {
	int flags;

	// begin address of this area in process address space, and maximum size
	void *address;
	size_t max_size;

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
			size_t size;
		} anon;

		// device type?
	};

	struct list_head list;
};


struct process;

/**
 * Initialize memory area subsystem.
 */
void mem_area_init();


/**
 * Return a newly allocated memory area, not initialized.
 */
struct mem_area *mem_area_alloc();
void mem_area_free(struct mem_area *area);


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
 * Copy a part of this area content to the given *physical* address.
 * This function *DO NOT* deal with virtual addresses, and don't check for page
 * boundaries. The caller should check if given address range still belong
 * to the appropriate process, and is already "allocated" and usable!
 * Offset is relative to the beginning of this area.
 * Return non-zero in error case.
 */
int mem_area_copy_raw(struct mem_area *area, size_t offset, void *dest, size_t size);

#endif //_SYS_MEM_AREA_H
