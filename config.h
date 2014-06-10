#ifndef _FIXOS_CONFIG_H
#define _FIXOS_CONFIG_H

/**
 * This file is the Configuration definition.
 * It's used as a C header included in each C file to build, and as a base to
 * generate the Makefile configuration file.
 *
 * For boolean config variables, set the value to y for "yes", and do not define
 * them for "no".
 */

#define __CONFIG__				y

// Architecture configuration
#define CONFIG_ARCH				sh3
#define CONFIG_ENDIAN_BIG		y	
//#define CONFIG_ENDIAN_LITTLE	y	
#define CONFIG_PLATFORM			fx9860

// TODO

// If defined, try to force optimizer to remove printk() calls (and constants
// passed to it), usefull to check how much size is used by printk() messages
//#define CONFIG_PRINTK_DUMMY		y


#endif //_FIXOS_CONFIG_H

