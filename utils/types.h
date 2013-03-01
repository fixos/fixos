#ifndef FIXOS_SYS_TYPES_H
#define FIXOS_SYS_TYPES_H

/**
  * This header contain the FiXos generic types definitions.
  * Even if FiXos doesn't aim to be ported on other architectures
  * that SuperH 32bit, it's really cleaner.
  */


typedef unsigned int size_t;

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

#endif // FIXOS_SYS_TYPES_H
