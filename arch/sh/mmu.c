#include "mmu.h"
#include "7705.h"
#include "utils/log.h"

#define MMU_MULTIPLE_VM (0<<8)
#define MMU_TLB_FLUSH	(1<<2)
#define MMU_TLB_INDEX_0	(0<<1)
#define MMU_ENABLE		(1<<0)

unsigned char g_soft_asid = 0;


void mmu_init()
{
	unsigned int mmu;

	mmu = MMU_MULTIPLE_VM | MMU_TLB_FLUSH | MMU_TLB_INDEX_0 | MMU_ENABLE;
	MMU.MMUCR.LONG = mmu;
	printk("[I] MMU init : 0x%x\n     (real=0x%x)\nPTEH=%p\n", mmu, MMU.MMUCR.LONG, &(MMU.PTEH.LONG));

	MMU.TTB = 0;

	printk("[I] set adress : %p\n", &mmu_init);
	mmu_setasid(0xFF);
}



