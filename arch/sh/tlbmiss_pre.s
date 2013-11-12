
! See exception_pre.s
! In TLB Miss case, we use only BANK1 (no reentrancy)


	.section ".handler.tlb.pretext"

	.global _pre_tlbmiss_handler
	.type _pre_tlbmiss_handler, @function

	.extern _tlbmiss_handler
	.extern _g_process_current_kstack

	.equ MD_MASK, (1<<30)

	.align 2
_pre_tlbmiss_handler:
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

	! now call the high level exception handler
	mov.l tlbmiss_handler, r0
	jsr @r0
	nop

	lds.l @r15+, pr
	mov.l @r15, r15

	rte
	nop


	.align 4
tlbmiss_handler:
	.long _tlbmiss_handler
g_process_current_kstack:
	.long _g_process_current_kstack

md_mask:
	.long MD_MASK
