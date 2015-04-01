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

