#include "physical_memory.h"
#include <utils/log.h>
#include <utils/types.h>

// this file implements arch_pm_xxx functions from arch/generic/memory.h
#include <arch/generic/memory.h>

// symbols and const for computing number of pages...
extern void * end_stack;
// default size of the RAM, as a log2 of bytes (used if unable to auto-detect)
#define RAM_DEFAULT_ORDER (17)
#define RAM_START_ADDRESS 0x08000000


// global symbol for the first free page :
static pm_freepage_head_t *pm_first_free = (void*)0;

// mainly for debuging
static int pm_nbfree = 0;

// variable used to detect RAM size
static unsigned int pm_watermark = 0;
#define WATERMARK_START		0x12345678
#define WATERMARK_CHECK		0xF0F00F0F
#define WATERMARK_CONFIRM	0x0F0FF0F0


// try to guess the RAM size by taking advantage of partial address decoding
// return the log2(RAM size)
static int pm_discover_ram_order() {
	int cur_order;
	int confirmed = 0;
	volatile unsigned int *watermark;

	watermark = P2_SECTION_BASE + SECTION_OFFSET(&pm_watermark);
	*watermark = WATERMARK_START;

	for(cur_order = PM_PAGE_ORDER; !confirmed && cur_order<26 ; cur_order++) {
		volatile unsigned int *tocheck;

		tocheck = ((void*)watermark) + (1<<cur_order);
		// test if watermark can be accessed by the two ptr
		if(*tocheck == WATERMARK_START) {
			*watermark = WATERMARK_CHECK;

			if(*tocheck == WATERMARK_CHECK) {
				*watermark = WATERMARK_CONFIRM;
				if(*tocheck == WATERMARK_CONFIRM)
					confirmed = cur_order;
			}

			if(!confirmed)
				*watermark = WATERMARK_START;
		}

	}

	if(!confirmed) {
		printk("pm_discover: unable to discover RAM size!\n");
		// default value
		return (RAM_DEFAULT_ORDER);
	}

	return confirmed;
}



void pm_init_pages()
{
	pm_freepage_head_t *page;
	
	unsigned int phy_end;
	int i;
	int nb_pages;
	int ram_order;

	phy_end = (unsigned int)(&end_stack) & 0x1FFFFFFF;

	ram_order = pm_discover_ram_order();
	printk("Total RAM size about %dKio...\n", 1<<(ram_order-10));

	// go to the first *fully* free page
	page = (void*)( ((phy_end-1) & (0xFFFFFFFF << PM_PAGE_ORDER))
			+ (1<<PM_PAGE_ORDER) );

	nb_pages = ( ((1<<ram_order) + RAM_START_ADDRESS) - (unsigned int)(page) )
		/ PM_PAGE_BYTES;

	// don't forget to 'translate' page pointer to P1 or P2 area!
	page = (void*)((int)page + P1_SECTION_BASE);

	for(i=0; i<nb_pages; i++) {
		if(i+1 == nb_pages) {
			page->next = (void*)0;
		}
		else
			page->next = (void*)((int)page + PM_PAGE_BYTES);
		if(i == 0)
			pm_first_free = page;
		
		page = page->next;
	}

	pm_nbfree = nb_pages;
	printk("%d pages of %dB\n", nb_pages, PM_PAGE_BYTES);
}



int pm_get_free_page(unsigned int *ppn)
{
	if(pm_first_free != (void*)0)
	{
		*ppn = PM_PHYSICAL_PAGE(pm_first_free);
		pm_first_free = pm_first_free->next;
		printk("pm: get page %p\n", PM_PHYSICAL_ADDR(*ppn));
		pm_nbfree--;
		return 0;
	}
	
	printk("pm: error: no more physical page!\n  (in theory, %d pages free)", pm_nbfree);
	while(1);
	return -1;
}


void pm_free_page(unsigned int ppn)
{
	// not only NULL address, but it's better to avoid this case
	if(ppn != 0) {
		// for now, no tests on the PPN value
		pm_freepage_head_t *page;

		page = PM_PHYSICAL_ADDR(ppn);
		page = (void*)((int)page + P1_SECTION_BASE);

		printk("free pm: %p (->%p)\n", page, pm_first_free);

		page->next = pm_first_free;
		pm_first_free = page;

		pm_nbfree++;
	}
	else {
		printk("free pm: error: trying to free ppn=0\n");
		while(1);
	}
}




// implementation of functions defined in arch/generic/memory.h

void* arch_pm_get_free_page(int flags) {
	unsigned int ppn;
	void *ret = NULL;

	if(pm_get_free_page(&ppn) == 0) {
		// use P1 or P2 area depending of flags
		ret = (flags & MEM_PM_UNCACHED) ? P2_SECTION_BASE : P1_SECTION_BASE;
		ret += (unsigned int)PM_PHYSICAL_ADDR(ppn);
	}
	return ret;
}


void arch_pm_release_page(void *page) {
	pm_free_page(PM_PHYSICAL_PAGE(page));
}


/* TODO
void mem_pm_fill_stats(struct mem_stats *stats) {
	
}
*/
