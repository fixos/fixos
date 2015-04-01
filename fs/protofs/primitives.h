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

#ifndef _FS_PROTOFS_PRIMITIVES_H
#define _FS_PROTOFS_PRIMITIVES_H


#define PROTOFS_MAX_NAME	16

// special type flags to indicate a free node
#define PROTOFS_TYPE_EMPTY	(1<<14)

// root node identifier
#define PROTOFS_ROOT_NODE		0xFFFFFFFF
#define PROTOFS_INVALID_NODE	0xEEEEEEEE

// structure of the protofs node
struct _protofs_node {
	char name[PROTOFS_MAX_NAME];
	void *parent;
	
	uint16 type_flags;
	uint16 mode;

	union {
		dev_t dev;
	} special;
};

typedef struct _protofs_node protofs_node_t;

#define PROTOFS_PER_PAGE	(PM_PAGE_BYTES/sizeof(protofs_node_t))

#define PROTOFS_NODE_NB(addr) ((int)(addr) - (int)_protofs_page) / sizeof(protofs_node_t)

#define PROTOFS_NODE_ADDR(nb) ((void*) ((nb) == PROTOFS_ROOT_NODE ? 0 : (nb) * sizeof(protofs_node_t) + (int)_protofs_page) )

#endif //_FS_PROTOFS_PRIMITIVES_H
