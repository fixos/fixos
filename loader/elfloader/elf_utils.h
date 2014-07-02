#ifndef _LOADER_ELFLOADER_ELF_UTILS_H
#define _LOADER_ELFLOADER_ELF_UTILS_H

/**
 * ELF file manipulation functions.
 */

#include "elf.h"

struct file;

/**
 * Get the ".symtab" section header.
 */
int elf_get_symtab(struct file *filep, const struct elf_header *header,
		struct elf_section_header *symtab);


/**
 * Get the section corresponding to given index.
 */
int elf_get_section(struct file *filep, const struct elf_header *header,
		uint16 index, struct elf_section_header *section);


/**
 * 
 */
int elf_get_string(struct file *filep, const struct elf_section_header *strtab,
		uint32 index, char *dest, size_t max);


int elf_get_symbol(struct file *filep, const struct elf_section_header *symtab,
		uint32 index, struct elf_symbol *symbol);

int elf_symbol_lookup(struct file *filep, const struct elf_section_header *symtab,
		const struct elf_header *header, const char *name,
		struct elf_symbol *symbol);


int elf_get_header(struct file *filep, struct elf_header *header);

#endif //_LOADER_ELFLOADER_ELF_UTILS_H
