
	.section ".text"
	.global _interrupt_inhibit_all
	.type _interrupt_inhibit_all, @function
	.align 2

	.equ BL_OFFSET, 28
! set or unset the BL bit in SR register
! if r4 == 0, 0->BL, else 1->BL
_interrupt_inhibit_all:
	stc		sr, r0
	mov		#1, r1
	mov		#BL_OFFSET, r2
	shld	r2, r1

	mov		#0, r2
	cmp/eq	r2, r4
	bt		unset_flag

	! set flag
	or		r1, r0
	bra		set_newsr
	nop

unset_flag:
	not		r1, r1
	and		r1, r0

set_newsr:
	ldc		r0, sr

	rts
	nop

