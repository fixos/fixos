#include "mmu.h"
#include "7705.h"

#define MMU_MULTIPLE_VM (1<<8)
#define MMU_TLB_FLUSH	(1<<2)
#define MMU_TLB_INDEX_0	(0<<1)
#define MMU_ENABLE		(1<<0)

void mmu_init()
{
	unsigned int mmu;

	mmu = MMU_MULTIPLE_VM | MMU_TLB_FLUSH | MMU_TLB_INDEX_0 | MMU_ENABLE;
	MMU.MMUCR.LONG = mmu;

	MMU.TTB = 0;

	mmu_setasid(0xFF);
}



