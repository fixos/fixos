#ifndef _LOADER_RAMLOADER_LOADER_H
#define _LOADER_RAMLOADER_LOADER_H

#include <utils/types.h>
#include <sys/process.h>

/**
 * Basic user-mode program loader.
 * The loaded program must reside in physical memory, and in only 1 "chunk"
 * which includes .text, .data .rodata and .bss sections.
 * Stack is allocated by the loader (1 physical page only for now).
 */

int ramloader_load(void *data, size_t datalen, process_t *dest);

#endif //_LOADER_RAMLOADER_LOADER_H
