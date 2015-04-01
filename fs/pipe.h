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

#ifndef _FS_PIPE_H
#define _FS_PIPE_H

/**
 * Provide file operations needed to use pipes, and pipe()-like way to create
 * a new pipe.
 */

#include "file_operations.h"


extern struct file_operations pipe_file_ops;


/**
 * Create the two files representing input and output of the pipe.
 */
int pipe_create(struct file *files[2]);

ssize_t pipe_read(struct file *filep, void *dest, size_t len);

ssize_t pipe_write(struct file *filep, void *source, size_t len);

int pipe_release(struct file *filep);



#endif //_FS_PIPE_H
