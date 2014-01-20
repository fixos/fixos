
	.section ".text"
	.align 2
	.global _arch_sched_preempt_task
	.global _arch_sched_preempt_fork

	.extern _process_get_current
	.extern _context_saved_next


_arch_sched_preempt_task:
	sts.l pr, @-r15
	mov.l proc_get_cur, r0
	jsr @r0
	nop
	
	! the returned value is a process_t, but it begins by the struct _context_info
	mov.l @r15+, r1
	mov.l r15, @(60, r0)
	mov.l r14, @(56, r0)
	mov.l r13, @(52, r0)
	mov.l r12, @(48, r0)
	mov.l r11, @(44, r0)
	mov.l r10, @(40, r0)
	mov.l r9, @(36, r0)
	mov.l r8, @(32, r0)
	add #64, r0
	stc sr, r2
	mov.l r2, @(16, r0)
	stc gbr, r2
	mov.l r2, @(0, r0)
	mov.l r1, @(12, r0)
	sts mach, r2
	mov.l r2, @(8, r0)
	sts macl, r2
	mov.l r2, @(4, r0)

	mov.l context_saved_next, r0
	jmp @r0

	.align 4
proc_get_cur:
		.long _process_get_current
context_saved_next:
		.long _context_saved_next



! int arch_sched_preempt_fork(process_t *dest, void *kstack);
_arch_sched_preempt_fork:
	! dest is a process_t, but it begins by the struct _context_info
	sts pr, r1
	mov r4, r0
	! saved stack is faked by the given kstack (r5)
	mov.l r5, @(60, r0)
	! return value is faked by giving 1 to r0
	mov #1, r2
	mov.l r2, @(0, r0)
	mov.l r14, @(56, r0)
	mov.l r13, @(52, r0)
	mov.l r12, @(48, r0)
	mov.l r11, @(44, r0)
	mov.l r10, @(40, r0)
	mov.l r9, @(36, r0)
	mov.l r8, @(32, r0)
	add #64, r0

	stc sr, r2
	! force SR.BL to 1, exception must be avoided in the direct return of child
	mov.l sr_bl_bit, r3
	or r3, r2
	mov.l r2, @(16, r0)

	stc gbr, r2
	mov.l r2, @(0, r0)
	mov.l r1, @(12, r0)
	sts mach, r2
	mov.l r2, @(8, r0)
	sts macl, r2
	mov.l r2, @(4, r0)

	// return the parent process value (0)
	// this line is never executed by dest process, which will continue from
	// return address (pr) directly
	mov #0, r0
	rts
	nop

	.align 4
sr_bl_bit:
	.long (1 << 28)
