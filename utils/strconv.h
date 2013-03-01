#ifndef _UTILS_STRCONV_H
#define _UTILS_STRCONV_H

/**
 * Some functions for string representation of common types.
 */

char * strconv_int_dec(int val, char *buf);

char * strconv_int_hex(unsigned int val, char *buf);

char * strconv_ptr(void* val, char *buf);

#endif //_UTILS_STRCONV_H
