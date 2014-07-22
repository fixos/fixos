#ifndef _FIXOS_INTERFACE_SYSCTL_H
#define _FIXOS_INTERFACE_SYSCTL_H

/**
 * BSD-like sysctl() interface, to get/set values from the kernel, managed
 * by a Management Interface Base (MIB) partialy described here.
 */


// Top level names
#define CTL_HW				0
#define CTL_KERN			1


// Second level for CTL_HW
// string 	: machine name (processor architecture)
#define HW_MACHINE			0
// string 	: model name (processor model and misc)
#define HW_MODEL			1
// int		: physical page size
#define HW_PAGESIZE			2
// int		: bytes of physical memory
#define HW_PHYSMEM			3
// int		: bytes of non-kernel memory
#define HW_USERMEM			4


// Second level for CTL_KERN
// string	: kernel name
#define KERN_OSTYPE			0
// node		: processes info
#define KERN_PROC			1
// string	: kernel release number, in the format "M.mm" (M is the major
// 			  version number, mm is the minor version)
#define KERN_OSRELEASE		2
// string	: kernel build date and time, in a human readable form
#define KERN_OSBUILDDATE	3

// Third level for KERN_PROC
// proc_uinfo[]	: array of all running processes in the system
#define KERN_PROC_ALL		0
// pid_t[]		: array of pids of all running processes in the system
#define KERN_PROC_PIDALL	1
// proc_uinfo	: process for the given PID (specified in fourth level) if any
#define KERN_PROC_PID		2


#endif //_FIXOS_INTERFACE_SYSCTL_H
