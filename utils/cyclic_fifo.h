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

#ifndef _UTILS_CYCLIC_FIFO_H
#define _UTILS_CYCLIC_FIFO_H

/**
 * Implementation of FIFO-like data structure, for character buffer.
 * To have as less copy as possible, the fixed-size buffer are used in a cyclic
 * way (the top of the FIFO is moving inside buffer each time some bytes are
 * read).
 */

#include <utils/types.h> 

struct cyclic_fifo {
	size_t max_size;
	size_t top;
	size_t size;
	char *buffer;
};


/**
 * Copy nb bytes from data into the FIFO. 
 */
void cfifo_push(struct cyclic_fifo *fifo, const char *data, size_t nb);

/**
 * Copy at most nb bytes from the FIFO to data.
 */
void cfifo_pop(struct cyclic_fifo *fifo, char *data, size_t nb);


#endif //_UTILS_CYCLIC_FIFO_H
