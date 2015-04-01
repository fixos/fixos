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

#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

/**
 * List of syscalls number and prototypes.
 * Depending the architecture, syscall are called by different way.
 * On SH3, FiXos uses assembly instruction "trapa #<syscall ID>".
 */

#include <interface/fixos/syscalls.h>



#define SYSCALL_NUMBER		35

// for kernel-part syscall handling
extern void* const _syscall_funcs[];

/**
 * Returns the syscall function associated to sysnum, or NULL if the given
 * syscall is not implemented.
 */
void* syscall_get_function(int sysnum);


#endif //_SYS_SYSCALL_H
