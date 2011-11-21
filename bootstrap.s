
	.section ".bootstrap"
	.global bootstrap
	.type bootstrap, @function
	.align 2

! bootstrap() function, dump the OS into the RAM and call initialize()
! WARNING :: this function can't use any variable safely except registers (no stack!)
! the linker script (fixos.ld) have change 
!

bootstrap:

  ! clear the BSS
  mov.l   bbss, r4   ! start
  mov.l   ebss, r5   ! end
  bra     L_check_bss
  mov     #0, r6
L_zero_bss:
  mov.l   r6, @r4        ! zero and advance
  add     #4, r4
L_check_bss:
  cmp/hs  r5, r4
  bf      L_zero_bss
 
  ! Copy the .text, .data, .rodata and other sections into the RAM
  mov.l   breloc, r4  ! dest
  mov.l   ereloc, r5  ! dest limit
  mov.l   romdata, r6        ! source
  bra     L_check_reloc
  nop
L_copy_reloc:
  mov.l   @r6+, r3
  mov.l   r3, @r4
  add     #4, r4
L_check_reloc:
  cmp/hs  r5, r4
  bf      L_copy_reloc
 

	mov.l   initialize, r3
  jmp     @r3
  nop

	rts     ! rts must never be reached
	nop


	.align 4
breloc:      .long _breloc
ereloc:      .long _ereloc

bbss:		.long _bbss
ebss:		.long _ebss

romdata:        .long _romdata

initialize:     .long _initialize

