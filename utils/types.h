#ifndef FIXOS_SYS_TYPES_H
#define FIXOS_SYS_TYPES_H

/**
  * This header contain the FiXos generic types definitions.
  * Even if FiXos doesn't aim to be ported on other architectures
  * that SuperH 32bit, it's really cleaner.
  */


typedef unsigned int size_t;
typedef int ssize_t;
typedef int off_t;

#define NULL ((void*)0)

// classic plateform-independant types
typedef unsigned int	uint32;
typedef int				int32;
typedef unsigned short	uint16;
typedef short			int16;
typedef unsigned char	uint8;
typedef char			int8;

// Address Space IDentifier
typedef uint8 asid_t;

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


// temporary location
// TODO move them to a more consistant place
//lseek constants
#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

#endif // FIXOS_SYS_TYPES_H
