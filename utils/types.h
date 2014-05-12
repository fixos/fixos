#ifndef FIXOS_SYS_TYPES_H
#define FIXOS_SYS_TYPES_H

/**
  * This header contain the FiXos generic types definitions.
  * Even if FiXos doesn't aim to be ported on other architectures
  * that SuperH 32bit, it's really cleaner.
  */

#include <interface/types.h>

// Address Space IDentifier
typedef uint8 asid_t;


// temporary location
// TODO move them to a more consistant place
//lseek constants
#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

#endif // FIXOS_SYS_TYPES_H
