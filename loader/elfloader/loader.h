#ifndef _LOADER_ELFLOADER_LOADER_H
#define _LOADER_ELFLOADER_LOADER_H

/**
 * ELF program loader, allowing to prepare a process from an ELF file.
 * For now, the ELF implementation is realy basic (no dynamic and relocation
 * related things, only usage of program header...).
 */

#include <sys/process.h>
#include <fs/file.h>

int elfloader_load(struct file *filep, process_t *dest);

#endif //_LOADER_ELFLOADER_LOADER_H
