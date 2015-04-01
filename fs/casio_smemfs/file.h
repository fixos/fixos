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
int smemfs_map_area(struct file *filep, struct mem_area *area);

union pm_page smemfs_area_pagefault(struct mem_area *area, void *addr_fault);

int smemfs_area_resize(struct mem_area *area, const struct mem_area *new_area);

void smemfs_area_release(struct mem_area *area);

int smemfs_area_duplicate(struct mem_area *orig, struct mem_area *copy);

#endif //_FS_SMEMFS_FILE_H
