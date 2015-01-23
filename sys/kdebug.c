#include "kdebug.h"
#include <utils/log.h>
#include <sys/process.h>


void kdebug_print_vmspace(struct process *proc) {
	struct page_dir *dir;
	char pages_state[MEM_DIRECTORY_PAGES + 1];

	pages_state[MEM_DIRECTORY_PAGES] = '\0';
	printk(LOG_EMERG, "Memory Map :\n");
	for(dir = proc->dir_list; dir != NULL; dir = dir->next) {
		int i;
		printk(LOG_EMERG, "DIR(%p~%p):\n  ", MEM_PAGE_ADDRESS(dir->dir_id, 0),
				MEM_PAGE_ADDRESS(dir->dir_id, MEM_DIRECTORY_PAGES));

		for(i=0; i<MEM_DIRECTORY_PAGES; i++) {
			if(dir->pages[i].private.flags & MEM_PAGE_PRIVATE) {
				pages_state[i] = dir->pages[i].private.flags & MEM_PAGE_VALID ?
						'v' : 'u';
			}
			else
				pages_state[i] = 'S';
		}

		printk(LOG_EMERG, "%s\n", pages_state);
	}

	printk(LOG_EMERG, "(End of Memory Map)\n");
}


void kdebug_print_symbol(void *addr) {
	const struct symbol_entry *symbol;

	// try to get symbol name and offset if registered
	symbol = kdebug_nearest_symbol(addr);

	if(symbol != NULL)
		printk(LOG_EMERG, "<%s + %d> (%p)", symbol->name, (uint32)addr - symbol->val, addr);
	else
		printk(LOG_EMERG, "<%p>", addr);
}

