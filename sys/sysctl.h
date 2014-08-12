#ifndef _SYS_SYSCTL_H
#define _SYS_SYSCTL_H

/**
 * BSD-like sysctl() interface. This is the main way to access to kernel info
 * (no /proc file system).
 * Unlike BSD and (deprecated) linux implementation, there are two syscalls,
 * one to set and one to get the value (but a userspace library function
 * may hide this implementation detail).
 *
 * TODO for now, there is nothing done to convert MIB ASCII names to integer
 * representation, maybe a syscall should be great to do this job?
 *
 * The tables are created by ctl_init() from information in special section
 * .sysctl.objs, which should only contains struct ctl_object.
 * To add a new sysctl object, use the macro SYSCTL_DECLARE_OBJECT()
 * with the appropriate struct.
 */

#include <interface/fixos/sysctl.h>
#include <utils/types.h>

// any raw data
#define CTL_TYPE_OPAQUE		1
// raw data accessed by next level index (such as KERN_PROC_PID)
#define CTL_TYPE_OPAQUE_NDX	2
// node with sub-objects
#define CTL_TYPE_NODE		3


typedef int (*ctl_access) (void *oldbuf, size_t *oldlen,
		const void *newbuf, size_t newlen);
typedef int (*ctl_access_ndx) (void *oldbuf, size_t *oldlen,
		const void *newbuf, size_t newlen, int index);

struct ctl_object {
	// parent object, or NULL if it's a top-level object
	const struct ctl_object *parent;
	// name for MIB representation
	const char *name;

	// integer id for this object (need to be unique in parent scope)
	// (note we are cheating a bit using a short instead an integer)
	short id;
	// object type
	short type;

	// function to call to read/write value, depending object type (not used
	// for nodes)
	union {
		ctl_access data;
		ctl_access_ndx indexed;
	} access;
};

#define SYSCTL_OBJECT(obj) const struct ctl_object \
	__attribute(( section(".sysctl.objs") )) \
	__attribute(( used )) obj


// pre-defined nodes
#define ctl__root	(*(struct ctl_object*)NULL)
extern const struct ctl_object ctl__hw;
extern const struct ctl_object ctl__kern;

/**
 * Initialize the sysctl tables from link-time data in .sysctl.objs section.
 */
void ctl_init();


/**
 * Helper for sysctl access function on simple data types (follow sysctl behavior
 * if either oldbuf is NULL or *oldlen is less than datalen).
 */
int ctl_access_read_helper(void *oldbuf, size_t *oldlen, const void *data,
		size_t datalen);

/**
 * Read a system data specified by the name array (with name_len levels).
 * If the specified buffer is either NULL or its size is two short to fit
 * the whole result, the call fail with a negative return value and len is
 * set to the expected buffer size.
 * No partial copy is done if len is too small.
 * If data is copied successfully, len is set to the amount of bytes copied.
 */
int sys_sysctl_read(const int *name, size_t name_len, void *buf, size_t *len);


/**
 * TODO
 */
int sys_sysctl_write(const int *name, size_t name_len, const void *buf, size_t *len);


/**
 * Convert a given sysctl string name into a MIB integer representation.
 * Separator is expected to be '.', like "kern.osrelease".
 * If strname may be converted to MIB, it will be stored in name array,
 * which should be at least of name_len integers.
 * Negative value is returned if name cannot be converted, or if name_len
 * is not large enough.
 * In success case, name_len is set to the actual name length.
 */
int sys_sysctl_mibname(const char *strname, int *name, int *name_len);

#endif //_SYS_SYSCTL_H
