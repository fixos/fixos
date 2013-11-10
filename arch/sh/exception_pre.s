!
! Pre-handlers for exceptions (allowing to act before any GCC generated code)
! The goal is mainly to get the correct stack address before any stack usage,
! and to store/restore from handling state.
! These handlers also switch from Register Bank 1 to 0, after stack is set and
! r0~r7 in bank 0 are saved.
!
! The C function is then called as a common function.
!


	.section ".handler.exception.pretext"

	.global _pre_exception_handler
	.type _pre_exception_handler, @function


	.extern _exception_handler
	.extern _g_process_current_kstack

	.equ MD_MASK, (1<<30)
	.equ RB_MASK, (1<<29)

	.align 2
_pre_exception_handler:
	! save old stack in r4 BANK1 in all case
	mov r15, r4

	! at the beginning, r0~r7 are mapped into BANK1, use it to retrieve the stack
	! if SSR.MD is 1, process was already in kernel mode, keep r15 as stack
	stc ssr, r1
	mov.l md_mask, r2
	tst r1, r2 		! T bit = 0 if MD = 1
	bf stack_set


	! in this cas, get the good kernel stack
	mov.l g_process_current_kstack, r0
	mov.l @r0, r15
	
stack_set:
	! save BANK1 on stack and switch to BANK0
	mov.l r4, @-r15		! old stack
	sts.l pr, @-r15
	stc.l R0_BANK, @-r15
	stc.l R1_BANK, @-r15
	stc.l R2_BANK, @-r15
	stc.l R3_BANK, @-r15
	stc.l R4_BANK, @-r15
	stc.l R5_BANK, @-r15
	stc.l R6_BANK, @-r15
	stc.l R7_BANK, @-r15

	mov.l rb_mask, r1
	not r1, r1
	stc sr, r2
	and r1, r2
	ldc r2, sr
	nop
	nop


	! now call the high level exception handler
	mov.l exception_handler, r0
	jsr @r0
	nop

	! restore full context (we still are in BANK0)
	mov.l @r15+, r0
	mov.l @r15+, r1
	mov.l @r15+, r2
	mov.l @r15+, r3
	mov.l @r15+, r4
	mov.l @r15+, r5
	mov.l @r15+, r6
	mov.l @r15+, r7
	lds.l @r15+, pr

	mov.l @r15, r15

	rte
	nop


	.align 4
exception_handler:
	.long _exception_handler
g_process_current_kstack:
	.long _g_process_current_kstack

md_mask:
	.long MD_MASK
rb_mask:
	.long RB_MASK
