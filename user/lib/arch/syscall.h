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

#ifndef _SYSCALL_ARCH_SYSCALL_H
#define _SYSCALL_ARCH_SYSCALL_H

// fast userland syscall definition
// dirty solution (temp) because these functions need to be defined in USER code
#define _SYSCALL_(name,id) \
	void name(int a, int b, int c, int d) \
	{ \
		__asm__ volatile ("trapa %0;" : : "n"(id) ); \
	}

#endif //_SYSCALL_ARCH_SYSCALL_H
