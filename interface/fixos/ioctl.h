#ifndef _FIXOS_INTERFACE_IOCTL_H
#define _FIXOS_INTERFACE_IOCTL_H

/**
 * Linux-like way to define ioctl : 2 chars used as unique identifier for
 * a subsystem/module, and a 16 bits value.
 * The R/W/RW suffix and the given type are designed to help to understand
 * how to use the ioctl, but are not currently used by the kernel.
 */


#define IOCTL_NAMESPACE(c1,c2)		((unsigned char)(c1) << 8 | (unsigned char)(c2))

#define IOCTL(space, val)			(space << 16 | val)
#define IOCTL_R(space, val, type)	(space << 16 | val)
#define IOCTL_W(space, val, type)	(space << 16 | val)
#define IOCTL_RW(space, val, type)	(space << 16 | val)


#endif //_FIXOS_INTERFACE_IOCTL_H
