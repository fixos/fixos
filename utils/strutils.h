#ifndef _FIXOS_STRUTILS_H
#define _FIXOS_STRUTILS_H

#include "sys/types.h"

/**
  * strutils allow to use some functions like libc' string.h functions
  * For the SH, the routines are really optimized (currently I use
  * the Newlib ones, written in assembly).
  */

void * memcpy ( void * destination, const void * source, size_t num );

void * memset ( void * ptr, int value, size_t num );

int strcmp ( const char * str1, const char * str2 );

char * strcpy ( char * destination, const char * source );

size_t strlen ( const char * str );

#endif // _FIXOS_STRUTILS_H
