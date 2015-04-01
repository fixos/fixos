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

#ifndef _LOADER_ELFLOADER_LOADER_H
#define _LOADER_ELFLOADER_LOADER_H

/**
 * ELF program loader, allowing to prepare a process from an ELF file.
 * For now, the ELF implementation is realy basic (no dynamic and relocation
 * related things, only usage of program header...).
 */

#include <sys/process.h>
#include <fs/file.h>

int elfloader_load(struct file *filep, struct process *dest);


#ifdef CONFIG_ELF_SHARED
/**
 * Load a dynamic library in desy address space.
 * For now, only try <SMEM_root>/lib/<soname>.
 */
int elfloader_load_dynlib(const char *soname, struct process *dest);


/**
 * Try to resolve a symbol through loaded dynamic libraries in the given
 * process.
 * NULL is returned if symbol is not found.
 * FIXME if symbol is equal to NULL, it will seem unfound.
 */
void *elfloader_resolve_dynsymbol(const char *symbol, struct process *target);


/**
 * dynbind() syscall implementation, look for given symbol in loaded
 * dynamic libraries, and write its value in *dest if not NULL.
 * Returns 0 if symbol is found, negative value else.
 */
int sys_dynbind(const char *symbol, void **dest);
#else
static inline int sys_dynbind(const char *symbol, void **dest) {
	(void)symbol;
	(void)dest;
	return -1;
}
#endif //CONFIG_ELF_SHARED


#endif //_LOADER_ELFLOADER_LOADER_H
