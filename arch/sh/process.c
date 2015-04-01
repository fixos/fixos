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

#include <arch/process.h>
#include <arch/generic/process.h>
#include <sys/process.h>

// debug
#include <utils/log.h>


#define SR_MD_MASK		(1<<30)

// defined in arch/generic/process.h :

void arch_kernel_contextjmp(struct _context_info *cnt, struct _context_info **old_cnt) {
//	printk(LOG_DEBUG, "[I] new context pc = %p\n  new sr = %p\n", (void*)(cnt->pc), (void*)(cnt->sr));  

	//printk(LOG_DEBUG, "kstack = %p\n", cnt->kernel_stack);
	
	if(old_cnt != NULL)
		*old_cnt = cnt->previous;

	__asm__ volatile (
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


int arch_process_mode(struct process *proc) {
	if(proc->acnt != NULL) {
		// if MD is 1, process is in kernel mode
		return (proc->acnt->sr & SR_MD_MASK) ? 0 : 1;
	}
	return -1;
}



void arch_idle_func();

__asm__ (
		"	.section \".text\" ;"
		"	.align 1 ;"
		"_arch_idle_func:"
		"	sleep;"
		"	bra _arch_idle_func;"
		"	nop;");


extern char end_stack; // defined in linker script

void arch_init_idle(struct process *proc) {
	static struct _context_info acnt_idle;
	acnt_idle.previous = NULL;
	acnt_idle.pr = (uint32)&arch_idle_func;
	acnt_idle.pc = (uint32)&arch_idle_func;
	acnt_idle.sr = 0x40000000; // MD=1, RB=0, BL=0, enable interrupts

	// we should reuse the kernel init stack safely
	acnt_idle.reg[15] = (uint32)&end_stack;
	proc->acnt = &acnt_idle;
	proc->dir_list = NULL;
	proc->kernel_stack = &end_stack;
	sigemptyset(& proc->sig_pending);
	sigfillset(& proc->sig_blocked);
	proc->addr_space.asid = 0xFF;
}
