
! See exception_pre.s


	.section ".handler.interrupt.pretext"

	.global _pre_interrupt_handler
	.type _pre_interrupt_handler, @function

	.extern _interrupt_handler
	! defined in arch/sh/exception_pre.s
	.extern _common_pre_handler

	.align 2
_pre_interrupt_handler:
	! we just set the 'high level' function to call after context is saved,
	! and jump to the common context saving routine
	mov.l common_pre_handler, r0
	mov.l interrupt_handler, r6
	jmp @r0
	nop


	.align 4
interrupt_handler:
	.long _interrupt_handler
common_pre_handler:
	.long _common_pre_handler
