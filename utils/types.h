#ifndef FIXOS_SYS_TYPES_H
#define FIXOS_SYS_TYPES_H

/**
  * This header contain the FiXos generic types definitions.
  * Even if FiXos doesn't aim to be ported on other architectures
  * that SuperH 32bit, it's really cleaner.
  */

#include <interface/fixos/types.h>

// Address Space IDentifier
typedef uint8 asid_t;

/**
 * offsetof() compute the byte offset between the beginning of a structure
 * and the given field.
 */
#define offsetof(type, field) \
	( (unsigned int)( &((type *)0)->field) )


/**
 * container_of() is used to access to a structure from an arbitrary field
 * address (used mainly for linked list and other data set).
 */
#define container_of(element, type, field) \
	( (type *) ((void*)(element) - offsetof(type, field)) )


// special control characters (^A, ^B,... ^[)
#define ASCII_CTRL(c) \
	((c) - '@')
#define ASCII_UNCTRL(c) \
	((c) + '@')

// temporary location
// TODO move them to a more consistant place
//lseek constants
#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

#endif // FIXOS_SYS_TYPES_H
