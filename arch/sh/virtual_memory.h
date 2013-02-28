#ifndef _ARCH_SH_VIRTUAL_MEMORY_H
#define _ARCH_SH_VIRTUAL_MEMORY_H

/**
 * Definitions of main types for virtual-memory management, and tool
 * functions.
 */

// page sizes
#define VM_PAGE_1K	0
#define VM_PAGE_4K	1
// default page flag used
#define VM_DEFAULT_SIZE VM_PAGE_1K
// default size in bytes of a page
#define VM_DEFAULT_BYTES 1024

/**
 * Virtual memory Page information type, used
 * for load/store page informations
 */
struct _vm_page {
	unsigned int vpn	:22;
	unsigned int ppn	:19;
	unsigned char cache	:1;
	unsigned char size	:1;
	unsigned char valid	:1;
	unsigned char _reserved :4; // for futur usage
};

typedef struct _vm_page vm_page_t;


struct _vm_table_page;

/**
 * VM table for a processus-like, with several indirections levels.
 * In the aim to don't penalize tiny process, 3 pages may be accessed
 * directly.
 * If the process need more than 3 pages, the other entries must be
 * in a special table (using a full page) at the indir1 address.
 * An entry with valid=0 is considered as a deleted entry
 * indir1 is used only if not equal (void*)0 
 *
 * Note : the page used for indir1 *must* be TLB exception safe.
 * This is why indir1 must point to P1 or P2 area (non translatable),
 * and should be managed by the kernel part!
 */
struct _vm_table {
	vm_page_t direct[3];
	struct _vm_table_page *indir1;
};

typedef struct _vm_table vm_table_t;


struct _vm_table_page {
	// for now it's only a big array of page table
	vm_page_t direct[VM_DEFAULT_BYTES/sizeof(vm_page_t)];
};


/**
 * Initialize a vm_table_t structure.
 */
extern inline void vm_init_table(vm_table_t *table) {
	vm_page_t val = {.valid = 0};
	table->direct[0] = val;
	table->direct[1] = val;
	table->direct[2] = val;

	table->indir1 = (void*)0;
}


/**
 * Look for a VM entry in the given table, with a given VPN (1K page number)
 * If the entry is not found, return 0.
 * TODO optimize and inline it
 */
vm_page_t *vm_find_vpn(vm_table_t *table, unsigned int vpn);


/**
 * Try to add an entry in the VM table.
 * The entry will be added in the first invalid entry found.
 * If the direct entries are all valid, the Table Page will be created
 * if possible.
 * In error case, return -1, else return 0
 */
int vm_add_entry(vm_table_t *table, vm_page_t *page);


#endif //_ARCH_SH_VIRTUAL_MEMORY_H
