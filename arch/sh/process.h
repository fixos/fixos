#ifndef _ARCH_SH_PROCESS_H
#define _ARCH_SH_PROCESS_H

/**
 * Low level primitives for process manipulation.
 */

#include <utils/types.h>
// debug
#include <utils/log.h>
#include <device/keyboard/keyboard.h>

// Default values for user mode process :
// MD, BL, RB = 0, Interrupt Mask = 0x0...
#define ARCH_UNEWPROC_DEFAULT_SR (0)
// begin of stack area (greatest address, never reached) :
#define ARCH_UNEWPROC_DEFAULT_STACK	0x40000000
// begin of process instructions
#define ARCH_UNEWPROC_DEFAULT_TEXT	0x10000000


extern void * g_process_current_kstack;


// information needed for process switch(store/load context)
struct _context_info {
	// registers r0~r15 (only the current bank for r0~r15) 
	uint32 reg[16];

	uint32 gbr;

	uint32 macl;
	uint32 mach;

	uint32 pc;
	uint32 sr;

	void *kernel_stack;
};

// extern inline void arch_kernel_contextjmp(struct _context_info *cnt)
//	__attribute__ ((noreturn)) ;

// 
extern inline void arch_kernel_contextjmp(struct _context_info *cnt) {
//	printk("[I] new context pc = %p\n  new sr = %p\n", (void*)(cnt->pc), (void*)(cnt->sr));  

	//printk("kstack = %p\n", cnt->kernel_stack);
	
	// set current kernel stack for next mode switching (interrupt)
	g_process_current_kstack = cnt->kernel_stack;
	
	// TODO kernel / user stack setting

	asm volatile (
			"mov %0, r0 ;"
			"mov r0, r15;"
			"add #64, r0;"
			"ldc.l @r0+, gbr;"
			"lds.l @r0+, macl;"
			"lds.l @r0+, mach;"
			"ldc.l @r0+, spc;"
			"ldc.l @r0+, ssr;"

			"mov.l @r15, r0;"
			"mov.l @(4, r15), r1;"
			"mov.l @(8, r15), r2;"
			"mov.l @(12, r15), r3;"
			"mov.l @(16, r15), r4;"
			"mov.l @(20, r15), r5;"
			"mov.l @(24, r15), r6;"
			"mov.l @(28, r15), r7;"
			"mov.l @(32, r15), r8;"
			"mov.l @(36, r15), r9;"
			"mov.l @(40, r15), r10;"
			"mov.l @(44, r15), r11;"
			"mov.l @(48, r15), r12;"
			"mov.l @(52, r15), r13;"
			"mov.l @(56, r15), r14;"
			"mov.l @(60, r15), r15;"
			"nop;"

			"rte;"
			"nop" : : "r"(cnt) :  
	);
}

#endif //_ARCH_SH_PROCESS_H
