#include "kdebug.h"
#include <utils/log.h>
#include <sys/process.h>


void kdebug_print_vmspace(struct process *proc) {
	struct page_dir *dir;

	printk(LOG_DEBUG, "Memory Map :\n");
	for(dir = proc->dir_list; dir != NULL; dir = dir->next) {
		int i;
		printk(LOG_DEBUG, "DIR(%p~%p):\n  ", MEM_PAGE_ADDRESS(dir->dir_id, 0),
				MEM_PAGE_ADDRESS(dir->dir_id, MEM_DIRECTORY_PAGES));
		for(i=0; i<MEM_DIRECTORY_PAGES; i++) {
			if(dir->pages[i].private.flags & MEM_PAGE_PRIVATE) {
				printk(LOG_DEBUG, dir->pages[i].private.flags & MEM_PAGE_VALID ?
						"v" : "u");
			}
			else
				printk(LOG_DEBUG, "S");
		}
		printk(LOG_DEBUG, "\n");
	}

	printk(LOG_DEBUG, "(End of Memory Map)\n");
}


void kdebug_print_symbol(void *addr) {
	const struct symbol_entry *symbol;

	// try to get symbol name and offset if registered
	symbol = kdebug_nearest_symbol(addr);

	if(symbol != NULL)
		printk(LOG_DEBUG, "<%s + %d> (%p)", symbol->name, (uint32)addr - symbol->val, addr);
	else
		printk(LOG_DEBUG, "<%p>", addr);
}

