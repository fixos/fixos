#ifndef _FIXOS_INTERFACE_FCNTL_H
#define _FIXOS_INTERFACE_FCNTL_H

/**
 * Constants used by open()/fcntl() functions
 */

#define O_RDONLY	(1<<0)
#define O_WRONLY	(1<<1)
#define O_RDWR		(O_RDONLY | O_WRONLY)

#define O_NONBLOCK	(1<<2)

#define O_CLOEXEC	(1<<10)


#define FD_CLOEXEC	(1<<0)

#endif //_FIXOS_INTERFACE_FCNTL_H
