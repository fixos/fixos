#ifndef _FIXOS_INTERFACE_TYPES_H
#define _FIXOS_INTERFACE_TYPES_H

/**
 * Common types used as well by the kernel and by the user space.
 */

#ifndef NULL
#define NULL ((void*)0)
#endif //defined(NULL)


typedef unsigned int size_t;
typedef int ssize_t;
typedef int off_t;


// classic plateform-independant types
typedef unsigned int	uint32;
typedef int				int32;
typedef unsigned short	uint16;
typedef short			int16;
typedef unsigned char	uint8;
typedef char			int8;

// Process IDendifier
typedef uint32 pid_t;


// time representation
typedef uint32 clock_t;
typedef uint32 time_t;

// high precision time struct
struct hr_time {
	time_t sec;
	uint32 nano;
};

// temp location (easier for user/kernel sharing)
struct tms {
	clock_t tms_utime;
	clock_t tms_stime;
	clock_t tms_cutime;
	clock_t tms_cstime;
};

#endif //_FIXOS_INTERFACE_TYPES_H
