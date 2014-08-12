#ifndef _FIXOS_INTERFACE_ERRNO_H
#define _FIXOS_INTERFACE_ERRNO_H

/**
 * Values used to indicate an error.
 * Used internally in the kernel to indicate the reason of an error, and
 * in system call return value.
 */

#define EAGAIN			2
#define EBADF			3
#define EFAULT			4
#define EINTR			5
#define EINVAL			6
#define EIO				7
#define EISDIR			8
#define ENFILE			9
#define EMFILE			10
#define ENOTDIR			11
#define ENXIO			12
#define ENAMETOOLONG	13
#define EROFS			14
#define ENOENT			15
#define ESRCH			16
#define EACCES			17
#define EPERM			18


#endif //_FIXOS_INTERFACE_ERRNO_H
