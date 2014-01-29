#include <sys/process.h>
#include "process.h"

// debug
#include <utils/log.h>
#include <device/keyboard/keyboard.h>



void arch_kernel_contextjmp(struct _context_info *cnt, struct _context_info **old_cnt) {
//	printk("[I] new context pc = %p\n  new sr = %p\n", (void*)(cnt->pc), (void*)(cnt->sr));  

	//printk("kstack = %p\n", cnt->kernel_stack);
	
/*	while(!is_key_down(K_EXE));
	while(is_key_down(K_EXE));
*/
	
	if(old_cnt != NULL)
		*old_cnt = cnt->previous;

	asm volatile (
			"mov %0, r0 ;"
			"mov r0, r15;"
			"add #64, r0;"
			"ldc.l @r0+, gbr;"
			"lds.l @r0+, macl;"
			"lds.l @r0+, mach;"
			"ldc.l @r0+, spc;"
			"ldc.l @r0+, ssr;"
			"lds.l @r0+, pr;"

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
