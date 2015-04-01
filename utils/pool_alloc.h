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

#ifndef _UTILS_POOL_ALLOC_H
#define _UTILS_POOL_ALLOC_H

/**
 * Pool Allocator is a generic fixed-size "object" allocator, which allow to
 * allocate and free some of these object without dealing directly with
 * physical pages, memory optimizations, etc...
 * The system try to minimize the number of pages used by trying to fill as
 * much as possible some of these (the goal is to have less "hole", only
 * unused or fully used pages...)
 *
 * The few restrictions for using this allocator are :
 *  - all objects have the same size, defined at the allocator creation
 *  - size of an object must be at least 4 bytes, and never more than a half page
 *    of physical memory
 */

#include <utils/types.h>
// for PM_PAGE_BYTES
#include <sys/memory.h>



struct pool_emptyobj;

struct pool_page {
	struct pool_page *next;
	// first empty object *in this page*
	struct pool_emptyobj *first_empty;
	// number of empty objects in the page
	int nbempty;
};


/**
 * Represent a single pool allocator.
 */
struct pool_alloc {
	// size of managed objects in bytes
	uint16 objsz;
	// number of objects per page (precalculated to avoid division each time)
	uint16 perpage;

	// first page (or NULL if no page allocated) 
	struct pool_page *first_page;
	// first page with an empty object to use, or NULL if there is no current
	// empty object
	struct pool_page *first_nonfull;
};


// helper to initialize properly a pool_alloc struct statically
#define POOL_INIT(type) { \
		.objsz = sizeof(type), \
		.perpage = (PM_PAGE_BYTES - sizeof(struct pool_page)) / sizeof(type), \
		.first_page = NULL, \
		.first_nonfull = NULL \
	}


/**
 * Allocate a new object and return a pointer to it, or NULL if allocation failled.
 */
void * pool_alloc(struct pool_alloc *pool);

/**
 * Free an object previously allocated by pool_alloc().
 * The object pointer *MUST* be exactly the one returned by pool_alloc()!
 */
void pool_free(struct pool_alloc *pool, void *object);


#endif //_UTILS_POOL_ALLOC_H
