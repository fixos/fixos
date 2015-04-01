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

#ifndef _BOOTLOADER_ELF_LOADER_H
#define _BOOTLOADER_ELF_LOADER_H

#include <loader/elfloader/elf.h>

struct smem_file;

/**
 * Check if given elf header is as expected (ELF magic bytes, big endian,
 * sh-arch, etc...).
 * Return 0 if ELF file seems to be correct.
 */
int elf_check_header(struct elf_header *h);


/**
 * Load given segment directly in memory.
 * Be careful, this function trust ELF segment header (physical memory
 * may be overwritten at any address, caller have to check that no
 * data from bootloader will be changed!).
 */
int elf_load_segment(struct smem_file *filep, const struct elf_prog_header *ph);



/**
 * Load all segments of the given ELF file, after checking everything is
 * correct.
 * If cmdline is not NULL, its content is copied at cmd_addr, overwriting any
 * content set before. Note that cmdline is expected to be a C string, and will
 * be truncated if its size is greater than cmd_max.
 * In success case, jump to the entry point immediately after kernel is loaded
 * (and never return, of course).
 * Else, a negative value is returned.
 */
int elf_load_kernel(const char *path, const char *cmdline, void *cmd_addr,
		int cmd_max);

#endif //_BOOTLOADER_ELF_LOADER_H
