#ifndef FIXOS_SYS_TYPES_H
#define FIXOS_SYS_TYPES_H

/**
  * This header contain the FiXos generic types definitions.
  * Even if FiXos doesn't aim to be ported on other architectures
  * that SuperH 32bit, it's really cleaner.
  */

#include <interface/fixos/types.h>

/**
 * Types used in user space interface are re-defined here without their prefix.
 */
typedef __kernel_size_t		size_t;
typedef __kernel_ssize_t	ssize_t;
typedef __kernel_off_t		off_t;


// classic plateform-independant types
typedef __kernel_uint32		uint32;
typedef __kernel_int32		int32;
typedef __kernel_uint16 	uint16;
typedef __kernel_int16		int16;
typedef __kernel_uint8		uint8;
typedef __kernel_int8		int8;

// Process IDendifier
typedef __kernel_pid_t		pid_t;


// devices identifier and macros for major/minor decomposition
typedef __kernel_dev_t		dev_t;
#define major(x)			__kernel_major(x)
#define minor(x)			__kernel_minor(x)
#define makedev(maj, min)	__kernel_makedev(maj, min)

typedef __kernel_ino_t		ino_t;
typedef __kernel_mode_t		mode_t;

// time representation
typedef __kernel_clock_t	clock_t;
typedef __kernel_time_t		time_t;


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
