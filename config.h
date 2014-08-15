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
// ARCH_FAMILY used to find the machine-specific directory into arch/
#define CONFIG_ARCH_FAMILY		sh
#define CONFIG_ARCH				sh3
#define CONFIG_PROC_MODEL		7705_Casio
#define CONFIG_ENDIAN_BIG		y	
//#define CONFIG_ENDIAN_LITTLE	y	
#define CONFIG_PLATFORM			fx9860

// maximum number of processes allowed
#define CONFIG_PROC_MAX			100

// number of PIDs (one more than the greatest PID allowed)
// should be a power of 2 for optimization purpose
#define CONFIG_PID_MAX			1024

// If defined, try to force optimizer to remove printk() calls (and constants
// passed to it), usefull to check how much size is used by printk() messages
//#define CONFIG_PRINTK_DUMMY		y

// Used to enable stack-debuging (allow more powerful post-mortem stack analysis)
//#define CONFIG_DEBUG_STACK		y

// Add debug information in binary, with ability to find corresponding text
// symbol name from any text address.
// Allow to see function names in call trace
//#define CONFIG_DEBUG_SYMBOL_NAMES	y


// Add kernel-part support for ELF shared libraries
#define CONFIG_ELF_SHARED			y


#endif //_FIXOS_CONFIG_H

