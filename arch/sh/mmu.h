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

/**
 * IMPORTANT : ASID problems with fx9860 (and other?) processor
 * On these CPUs, ASID can't be write (probably because doesn't exist
 * in the hardware inself :/)
 * Because of that, ASID is still used in the kernel code as if it existed,
 * but it's value is stored in a RAM variable.
 * 
 * The big issue is the MMU which can't diferentiate running processes.
 * The only way to allow multiple process (with multiple adress space) without
 * hardware ASID is to flush the TLB between each context switch (in reality only
 * between two user process, if the kernel itself never use same VM space)...
 *
 * TODO confirm that no hardware ASID exists :( 
 */
extern unsigned char g_soft_asid;

// flush the TLB (set V bit of each entry to 0)
extern inline void mmu_tlbflush() {
	MMU.MMUCR.BIT.TF = 1;
}


// initialize (and start) the MMU, flush the TLB, and set the current ASID to 0xFF
void mmu_init();


// set the current ASID (dangerous if virtual memory is used consecutivly!)
extern inline void mmu_setasid(unsigned char asid) {
	//MMU.PTEH.BIT.ASID = asid;
	g_soft_asid = asid;
}

// get the current ASID
extern inline unsigned char mmu_getasid() {
	//return MMU.PTEH.BIT.ASID;
	return g_soft_asid;
}

// fill and load a TLB entry in PTEL without change informations in PTEH
// (after a TLB miss, PTEH should be valid if the page is allowed)
// PPN must be given like a 1K page number (even for 4K page!)
extern inline void mmu_tlb_fillload(unsigned int ppn, unsigned short flags) {
	MMU.PTEL.LONG = (ppn << 10) | flags;
	asm volatile ("ldtlb":::"memory" );
}


// load

#endif // ARCH_SH_MMU_H