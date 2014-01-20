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
 * In success case, jump to the entry point immediately after kernel is loaded
 * (and never return, of course).
 * Else, a negative value is returned.
 */
int elf_load_kernel(const char *path);

#endif //_BOOTLOADER_ELF_LOADER_H