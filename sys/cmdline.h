/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SYS_CMDLINE_H
#define _SYS_CMDLINE_H

/**
 * Command Line Arguments are boot time parameters for the kernel.
 * They are copied by the bootloader at a fixed position in RAM (arch-dependant),
 * as a sequence of 'key=value' (or 'key' for booleans) separated by spaces.
 *
 * To handle these parameters, each possible argument is added to a dedicated
 * section in the binary file. The name of the argument is associated to the
 * callback function, called when the command line is parsed.
 * Be careful : these callback must be designed to run in first steps during
 * boot process, not everything is initialized and clean!
 */

#include <utils/types.h>

/**
 * The value parameter is a zero-terminated string, or NULL if no '=' character
 * follow directly the argument name.
 */
struct kernel_arg {
	const char * argname;
	int (*callback)(const char *value);
};


#define KERNEL_BOOT_ARG(name, callback) \
	const struct kernel_arg bootarg_##name \
		__attribute__ ((section (".vector.kernelarg") )) = { #name, callback}


/**
 * Parse the command line arguments string, and call appropriate functions
 * for each declared argument.
 */
void cmdline_parse(char *bootargs, size_t max_size);


#endif //_SYS_CMDLINE_H
