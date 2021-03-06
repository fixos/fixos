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

#ifndef _SYS_KDEBUG_H
#define _SYS_KDEBUG_H

/**
 * Kernel Debug routines, mainly for printing traces when something go wrong.
 * Some of the trace routines are arch-specific (stack trace...).
 */

#include <utils/types.h>

struct symbol_entry {
	uint32 val;
	const char *name;
};

struct process;


/**
 * Kernel panic routine, called when nothing may be done to avoid the
 * kernel crash.
 * If errstr is not NULL, the given message is printed before other debug
 * info.
 */
extern void kdebug_oops(const char *errstr);


/**
 * Display the memory map of the given process.
 */
void kdebug_print_vmspace(struct process *proc);


/**
 * Print better available representation of the given address (if possible
 * in the form <{symbol_name} + {displacement}> ({address})).
 * The result is printed using printk() without any line feed.
 */
void kdebug_print_symbol(void *addr);


/**
 * Return the symbol entry matching addr if any (NULL if not found or symbol
 * names not available).
 */
extern const struct symbol_entry *kdebug_nearest_symbol(void *addr);


/**
 * Print current stack info and call trace if possible.
 */
extern void kdebug_print_trace();

#endif //_SYS_KDEBUG_H
