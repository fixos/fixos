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

#ifndef _ARCH_SH_INCLUDE_PROCESS_H
#define _ARCH_SH_INCLUDE_PROCESS_H

/**
 * sh3 arch-specific process definitions
 */

/**
 * Low level primitives for process manipulation.
 */

#include <utils/types.h>

// Default values for user mode process :
// MD, BL, RB = 0, Interrupt Mask = 0x0...
#define ARCH_UNEWPROC_DEFAULT_SR (0)
// begin of stack area (greatest address, never reached) :
#define ARCH_UNEWPROC_DEFAULT_STACK	0x40000000
// begin of process instructions
#define ARCH_UNEWPROC_DEFAULT_TEXT	0x10000000
// arguments and environnement location (for parameterized main())
#define ARCH_UNEWPROC_DEFAULT_ARGS	0x50000000


// information needed for process switch(store/load context)
struct _context_info {
	// registers r0~r15 (only the current bank for r0~r15) 
	uint32 reg[16];

	uint32 gbr;

	uint32 macl;
	uint32 mach;

	uint32 pc;
	uint32 sr;

	uint32 pr;

	// previous context structure (may be NULL if this interrupted from user space)
	struct _context_info *previous;
};



#endif //_ARCH_SH_INCLUDE_PROCESS_H
