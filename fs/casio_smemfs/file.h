#ifndef _FS_SMEMFS_FILE_H
#define _FS_SMEMFS_FILE_H

#include <fs/file_operations.h>
#include <fs/file.h>
#include <interface/fixos/stat.h>
#include <sys/mem_area.h>

/**
 * Implementation of file operations for the Casio SMEM FS.
 */

extern const struct file_operations smemfs_file_operations;

// for memory-mapped files
extern const struct mem_area_ops smemfs_mem_ops;


int smemfs_release (struct file *filep);

ssize_t smemfs_read (struct file *filep, void *dest, size_t len);

off_t smemfs_lseek (struct file *filep, off_t offset, int whence);


// memory-mapped operations
union pm_page smemfs_area_pagefault(struct mem_area *area, void *addr_fault);

int smemfs_area_resize(struct mem_area *area, const struct mem_area *new_area);

void smemfs_area_release(struct mem_area *area);


#endif //_FS_SMEMFS_FILE_H
