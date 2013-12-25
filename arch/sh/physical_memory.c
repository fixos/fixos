#include "physical_memory.h"
#include <utils/log.h>
#include <utils/types.h>

// this file implements mem_pm_xxx functions from sys/memory.h
#include <sys/memory.h>

// symbols and const for computing number of pages...
extern void * end_static_ram;
// size of RAM in bytes (2^16)
#define RAM_SIZE (1024<<6)
#define RAM_START_ADDRESS 0x08000000

// P1 is cacheable and non-translatable
#define P1_SECTION_BASE ((void*)0x80000000)
// P2 is non-cacheable and non-translatable
#define P2_SECTION_BASE ((void*)0xA0000000)

// TODO better solution
#define STACK_PAGES	4

// global symbol for the first free page :
static pm_freepage_head_t *pm_first_free = (void*)0;
// the last free page is used to join re-freed pages
static pm_freepage_head_t *pm_last_free = (void*)0;

void pm_init_pages()
{
	// TODO STAAAAACK!!! Not in free page list :'(
	
	pm_freepage_head_t *page;
	
	unsigned int phy_end;
	int i;
	int nb_pages;

	phy_end = (unsigned int)(&end_static_ram) & 0x1FFFFFFF;

	// go to the first *fully* free page
	page = (void*)( ((phy_end-1) & (0xFFFFFFFF << PM_PAGE_ORDER))
			+ (1<<PM_PAGE_ORDER) );

	nb_pages = ( (RAM_SIZE + RAM_START_ADDRESS) - (unsigned int)(page) )
		/ PM_PAGE_BYTES - STACK_PAGES;

	// don't forget to 'translate' page pointer to P1 or P2 area!
	page = (void*)((int)page + P1_SECTION_BASE);

	for(i=0; i<nb_pages; i++) {
		if(i+1 == nb_pages) {
			page->next = (void*)0;
			pm_last_free = page;
		}
		else
			page->next = (void*)((int)page + PM_PAGE_BYTES);
		if(i == 0)
			pm_first_free = page;
		
		page = page->next;
	}

	printk("%d pages of %dB\n", nb_pages, PM_PAGE_BYTES);
}



int pm_get_free_page(unsigned int *ppn)
{
	if(pm_first_free != (void*)0)
	{
		*ppn = PM_PHYSICAL_PAGE(pm_first_free);
		pm_first_free = pm_first_free->next;
		return 0;
	}
	
	return -1;
}


void pm_free_page(unsigned int ppn)
{
	// for now, no tests on the PPN value
	pm_freepage_head_t *page;

	page = PM_PHYSICAL_ADDR(ppn);
	page = (void*)((int)page + P1_SECTION_BASE);
	
	page->next = (void*)0;
	// we assume pm_first_free == NULL => pm_last_free == NULL
	if(pm_first_free == (void*)0)
		pm_first_free = page;
	else
		pm_last_free->next = page;

	pm_last_free = page;
}




// implementation of functions defined in sys/memory.h

void* mem_pm_get_free_page(int flags) {
	unsigned int ppn;
	void *ret = NULL;

	if(pm_get_free_page(&ppn) == 0) {
		// use P1 or Pé area depending of flags
		ret = (flags | MEM_PM_UNCACHED) ? P2_SECTION_BASE : P1_SECTION_BASE;
		ret += (unsigned int)PM_PHYSICAL_ADDR(ppn);
	}
	return ret;
}


void mem_pm_release_page(void *page) {
	pm_free_page(PM_PHYSICAL_PAGE(page));
}
