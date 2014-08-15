#include "mmu.h"
#include "7705.h"
#include <utils/log.h>
#include <arch/generic/memory.h>


#define MMU_MULTIPLE_VM (0<<8)
#define MMU_TLB_FLUSH	(1<<2)
#define MMU_TLB_INDEX_0	(0<<1)
#define MMU_ENABLE		(1<<0)


// software ASID management :

// Reserved ASID are 255 (for idle task) and 254 (invalid value)
#define ASID_MAX		100

// array of usage for each possible ASID
#define ASID_FREE		0
#define ASID_USED		1
#define ASID_DIRTY		2	// used when an ASID is released, but TLB not flushed
static char _asid_usage[ASID_MAX];



void mmu_init()
{
	unsigned int mmu;
	int i;

	mmu = MMU_MULTIPLE_VM | MMU_TLB_FLUSH | MMU_TLB_INDEX_0 | MMU_ENABLE;
	MMU.MMUCR.LONG = mmu;
	mmu_setasid(0xFF);

	printk("[I] MMU init : 0x%x\n     (real=0x%x)\nPTEH=%p\n", mmu, MMU.MMUCR.LONG, (void*)(MMU.PTEH.LONG));

	MMU.TTB = 0;

	for(i=0; i<ASID_MAX; i++)
		_asid_usage[i] = ASID_FREE;
}


// address space related functions (from arch/generic/memory.h)

int arch_adrsp_init(struct addr_space *adrsp) {
	adrsp->asid = ASID_INVALID;
	return 0;
}


/**
 * Switch from the current address space to the given one.
 * If needed, adrsp may change to have a "clean" address space identifier.
 */
int arch_adrsp_switch_to(struct addr_space *adrsp) {
	// if ASID is not ASID_INVALID, only switch the current used ASID
	if(adrsp->asid == ASID_INVALID) {
		int i;

		// find a not used ASID
		for(i=0; i<ASID_MAX && _asid_usage[i] != ASID_FREE; i++);
		if(i >= ASID_MAX) {
			int j;
			// try to free some dirty ASIDs, with a TLB flush
			for(j=0; j<ASID_MAX; j++) {
				if(_asid_usage[j] == ASID_DIRTY) {
					_asid_usage[j] = ASID_FREE;
					i = j;
				}
			}

			// if no more ASID are available even after quick cleanning :
			// FIXME force old ASID to be removed?
			if(i >= ASID_MAX) {
				printk("adrsp: no more ASID available!\n");
				return -1;
			}
			else {
				mmu_tlbflush();
			}
		}

		// now, i is in all case the current ASID
		_asid_usage[i] = ASID_USED;
		adrsp->asid = i;

	}

	mmu_setasid(adrsp->asid);
	return 0;
}


/**
 * Release an address space (no more usage of the represented address space).
 */
int arch_adrsp_release(struct addr_space *adrsp) {
	if(adrsp->asid >= ASID_MAX) {
		adrsp->asid = ASID_INVALID;
		return -1;
	}

	_asid_usage[adrsp->asid] = ASID_DIRTY;
	adrsp->asid = ASID_INVALID;
	return 0;
}
