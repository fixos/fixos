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

#ifndef ARCH_SH_MMU_H
#define ARCH_SH_MMU_H

#include "7705.h"

/**
 * Low-level MMU/TLB manipulation routines and definitions
 */

// flags for the TLB (PTEL)
#define TLB_VALID		(1<<8)
#define TLB_INVALID		(0<<8)

#define TLB_NOTSHARED	(0<<1)

#define TLB_SIZE_1K		(0<<4)
#define TLB_SIZE_4K		(1<<4)

// TLB protection flags, P for "only in protected mode", U for
// "user/protected modes", R > Read, RW > Read/Write
#define TLB_PROT_P_R	(0x0<<5)
#define TLB_PROT_P_RW	(0x1<<5)
#define TLB_PROT_U_R	(0x2<<5)
#define TLB_PROT_U_RW	(0x3<<5)

#define TLB_DIRTY		(1<<2)

#define TLB_CACHEABLE	(1<<3)


// value used to represent an invalid ASID in SuperH arch
#define ASID_INVALID	0xFE


// flush the TLB (set V bit of each entry to 0)
static inline void mmu_tlbflush() {
	MMU.MMUCR.BIT.TF = 1;
}


// initialize (and start) the MMU, flush the TLB, and set the current ASID to 0xFF
void mmu_init();


// set the current ASID (dangerous if virtual memory is used consecutivly!)
static inline void mmu_setasid(unsigned char asid) {
	MMU.PTEH.BIT.ASID = asid;
}


// get the current ASID
static inline unsigned char mmu_getasid() {
	return MMU.PTEH.BIT.ASID;
}

// fill and load a TLB entry in PTEL without change informations in PTEH
// (after a TLB miss, PTEH should be valid if the page is allowed)
// PPN must be given like a 1K page number (even for 4K page!)
static inline void mmu_tlb_fillload(unsigned int ppn, unsigned short flags) {
	MMU.PTEL.LONG = (ppn << 10) | flags;
	__asm__ volatile ("ldtlb":::"memory" );
}


// load

#endif // ARCH_SH_MMU_H
