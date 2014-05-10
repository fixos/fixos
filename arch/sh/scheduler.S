
	.section ".text"
	.align 2
	.global _arch_sched_preempt_task


! void arch_sched_preempt_task(process_t *cur_proc, void (*next)());
! this function save the caller context on the kernel stak, then call the
! given function (likely a scheduler internal function).
! when the scheduler run the caller task again, this function just seems to
! return normaly (so we don't save r0~r7 because SH3 calling convention
! consider them as callee safe)
_arch_sched_preempt_task:
	! cur_proc is a process_t, but it begins by the struct _context_info pointer
	mov.l @r4, r0

	! use the current kernel stack to store the new context_info
	mov r15, r1
	mov.l r0, @-r15
	! PR register is saved by the caller
	add #-4, r15
	stc.l sr, @-r15
	! current PR is the context PC
	sts.l pr, @-r15
	sts.l mach, @-r15
	sts.l macl, @-r15
	stc.l gbr, @-r15

	! general registers, r15 was saved in r1
	mov.l r1, @-r15		! old stack
	mov.l r14, @-r15
	mov.l r13, @-r15
	mov.l r12, @-r15
	mov.l r11, @-r15
	mov.l r10, @-r15
	mov.l r9, @-r15
	mov.l r8, @-r15

	! do not save r0~r7
	add #-(8*4), r15

	! update proc->acnt
	mov.l r15, @r4

	jmp @r5


