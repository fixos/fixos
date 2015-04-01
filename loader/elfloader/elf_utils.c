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

#include "elf_utils.h"
#include <fs/vfs_file.h>
#include <utils/log.h>
#include <utils/strutils.h>


int elf_get_symtab(struct file *filep, const struct elf_header *header,
		struct elf_section_header *symtab)
{
	int cursec;

	for(cursec=0; cursec < header->shnum; cursec++) {
		if(elf_get_section(filep, header, cursec, symtab) != 0) {
			return -1;
		}
		else if(symtab->type == ELFS_TYPE_SYMTAB) {
			printk(LOG_DEBUG, "elf: SYMTAB offset : 0x%x\n", symtab->offset);
			break;
		}
	}

	return cursec < header->shnum ? 0 : -1;
}



int elf_get_section(struct file *filep, const struct elf_header *header,
		uint16 index, struct elf_section_header *section)
{
	vfs_lseek(filep, header->shoff + index*sizeof(*section), SEEK_SET);
	if(vfs_read(filep, section, sizeof(*section)) != sizeof(*section)) {
		return -1;
	}
	return 0;
}


int elf_get_string(struct file *filep, const struct elf_section_header *strtab,
		uint32 index, char *dest, size_t max)
{
	size_t read;

	vfs_lseek(filep, strtab->offset + index, SEEK_SET);
	read = vfs_read(filep, dest, max-1);
	if(read <= 0) {
		return -1;
	}

	dest[read] = '\0';
	return 0;
}


int elf_get_symbol(struct file *filep, const struct elf_section_header *symtab,
		uint32 index, struct elf_symbol *symbol)
{
	vfs_lseek(filep, symtab->offset + index*sizeof(*symbol), SEEK_SET);
	if(vfs_read(filep, symbol, sizeof(*symbol)) != sizeof(*symbol)) {
		return -1;
	}
	return 0;
}



int elf_symbol_lookup(struct file *filep, const struct elf_section_header *symtab,
		const struct elf_header *header, const char *name,
		struct elf_symbol *symbol)
{
	int cursym;
	char buf[30];

	struct elf_section_header last_strtab;

	elf_get_section(filep, header, symtab->link, &last_strtab);

	for(cursym=0; cursym < symtab->size/sizeof(*symbol); cursym++) {
		elf_get_symbol(filep, symtab, cursym, symbol);
		if(symbol->name != 0) {
			elf_get_string(filep, &last_strtab, symbol->name, buf, 30);
			//printk(LOG_DEBUG, "elfloader: symbol '%s' @%p\n", absname, (void*)sym.value);
			if(strcmp(buf, name) == 0) {
				return 0;	
			}
		}
	}
	return -1;
}


int elf_get_header(struct file *filep, struct elf_header *header) {
	vfs_lseek(filep, 0, SEEK_SET);
	if(vfs_read(filep, header, sizeof(*header)) != sizeof(*header)) {
		printk(LOG_WARNING, "elf: unable to read header\n");
		return -1;
	}
	return 0;
}

