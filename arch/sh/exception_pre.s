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

	.global _common_pre_handler
	.type _common_pre_handler, @function


	.extern _exception_handler
	.extern _pre_tlbmiss_handler

	.extern __proc_current

	.equ MD_MASK, (1<<30)
	.equ RB_MASK, (1<<29)

	.equ EXPEVT, 0xFFFFFFD4 

	.align 2
_pre_exception_handler:
	! before to do usual job, check exception code 0x040 and 0x060
	! (for TLB Invalid Exception)
	! if one of these, redirect to _tlbmiss_pre_handler
	mov.l expevt, r0
	mov.l @r0, r0
	cmp/eq #0x40, r0
	bt redirect_tlbmiss
	cmp/eq #0x60, r0
	bf normal_exception

redirect_tlbmiss:
	mov.l tlbmiss_pre_handler, r0
	jmp @r0
	nop


normal_exception:
	! set 'high level' function to call in r6
	mov.l exception_handler, r6


	! the following part is common to interrupt and exceptions
_common_pre_handler:
!	mov.l printk, r0
!	mov.l msg, r4
!	mov.l _proc_current, r7
!	mov.l @r7, r7
!	mov r15, r5
!	jsr @r0
!	nop
!	pouet2: bra pouet2; nop
!	.align 4
!	.extern _printk
!	printk: .long _printk
!	msg: .long _msg
!	_msg: .asciz "[OmG] r15=%p, r6=%p, _cur=%p\n"

	! save old stack in r4 BANK1 in all case
	mov r15, r4
	! r5 is the address of current process struct
	mov.l _proc_current, r5
	mov.l @r5, r5

	! at the beginning, r0~r7 are mapped into BANK1, use it to retrieve the stack
	! if SSR.MD is 1, process was already in kernel mode, keep r15 as stack
	stc ssr, r1
	mov.l md_mask, r2
	tst r1, r2 		! T bit = 0 if MD = 1
	bf stack_set


	! in this cas, get the good kernel stack (second field in process_t struct)
	mov.l @(4,r5), r15
	
stack_set:
	! save BANK1 on stack and switch to BANK0
	! all the context is saved as described in _context_info structure
	! (in the reversed order of the structure)

	! first of all, save current process previous _context_info if any
	! (this allow exception/interrupt inside a syscall)
	mov.l @r5, r0
	mov.l r0, @-r15

	! then, special registers (spc and ssr replace pc and sr)
	sts.l pr, @-r15
	stc.l ssr, @-r15
	stc.l spc, @-r15
	sts.l mach, @-r15
	sts.l macl, @-r15
	stc.l gbr, @-r15

	! general registers (r15 was saved in r4, and r0~r7 come from BANK0)
	mov.l r4, @-r15		! old stack
	mov.l r14, @-r15
	mov.l r13, @-r15
	mov.l r12, @-r15
	mov.l r11, @-r15
	mov.l r10, @-r15
	mov.l r9, @-r15
	mov.l r8, @-r15
	stc.l R7_BANK, @-r15
	stc.l R6_BANK, @-r15
	stc.l R5_BANK, @-r15
	stc.l R4_BANK, @-r15
	stc.l R3_BANK, @-r15
	stc.l R2_BANK, @-r15
	stc.l R1_BANK, @-r15
	stc.l R0_BANK, @-r15


	! update current process context pointer
	mov.l r15, @r5

	! before to use BANK0 again, saving r6 into an un-bankable register
	mov r6, r8

	mov.l rb_mask, r1
	not r1, r1
	stc sr, r2
	and r1, r2
	ldc r2, sr
	nop
	nop


	! now call the high level exception handler (set in r6 before common_pre_handler)
	jsr @r8
	nop


	! should never return directly
infloop:
	bra infloop
	nop

	! TODO
	!mov.l bank0_context, r1
	mov.l @r15+, r2
	mov.l r2, @r1

	! restore full context (we still are in BANK0)
	mov.l @r15+, r0
	mov.l @r15+, r1
	mov.l @r15+, r2
	mov.l @r15+, r3
	mov.l @r15+, r4
	mov.l @r15+, r5
	mov.l @r15+, r6
	mov.l @r15+, r7
	ldc.l @r15+, ssr
	ldc.l @r15+, spc
	lds.l @r15+, pr

	mov.l @r15, r15

	rte
	nop


	.align 4
exception_handler:
	.long _exception_handler
_proc_current:
	.long __proc_current
tlbmiss_pre_handler:
	.long _pre_tlbmiss_handler

md_mask:
	.long MD_MASK
rb_mask:
	.long RB_MASK

expevt:
	.long EXPEVT
