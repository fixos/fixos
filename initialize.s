
	.section ".pretext"
	.global _initialize
	.type _initialize, @function
	.align 2

! initialize the stack and call _init()
_initialize:
  ! stop interrupt to be safe
  mov	#1, r1
  mov	#28, r2
  shld	r2, r1	! select bit 28 (BL)
  stc	sr, r0
  or	r1, r0	! BL bit of SR to 1
  ldc	r0, sr
  ! stack and _init()
  mov.l	stack_start, r15
  mov.l	init, r0
  jmp	@r0
  nop
  rts
  nop

  .align 4
init:
  .long _init
stack_start:
  .long _end_stack		! defined in fixos.ld


!TODO change stack address...
