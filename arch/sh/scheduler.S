
	.section ".text"
	.align 2
	.global _arch_sched_preempt_task
	.global _arch_sched_preempt_fork

	.extern _context_saved_next


! void arch_sched_preempt_task(process_t *cur_proc);
! this function save the caller context, and let the scheduler running an other
! task.
! when the scheduler run the caller task again, this function just seems to
! return normaly (so we don't save r0~r7 because SH3 calling convention
! consider them as callee safe)
_arch_sched_preempt_task:
	! cur_proc is a process_t, but it begins by the struct _context_info pointer
	mov.l @r4, r0
	
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
	sts pr, r2
	mov.l r2, @(12, r0)
	sts mach, r2
	mov.l r2, @(8, r0)
	sts macl, r2
	mov.l r2, @(4, r0)

	mov.l context_saved_next, r0
	jmp @r0

	.align 4
context_saved_next:
		.long _context_saved_next


