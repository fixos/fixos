
	.section ".bootstrap"
	.global bootstrap
	.type bootstrap, @function
	.align 2

! bootstrap() function, dump the OS into the RAM and call initialize()
! WARNING :: this function can't use any variable safely except registers (no stack!)
! the linker script (fixos.ld) have change 
!

bootstrap:
	! set up TLB
	mov.l   Hmem_SetMMU, r3
	mov.l   address_one, r4 ! 0x8102000
	mov.l   address_two, r5 ! 0x8801E000
	jsr     @r3    ! _Hmem_SetMMU
	mov     #108, r6

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


! obscure MMU related function of the Casio's OS
_Hmem_SetMMU:
	mov.l   sc_addr, r2
	mov.l   1f, r0
	jmp     @r2
	nop
	1:      .long 0x3FA


	.align 4

sc_addr:        .long 0x80010070

breloc:      .long _breloc
ereloc:      .long _ereloc

bbss:		.long _bbss
ebss:		.long _ebss

romdata:        .long _romdata

initialize:     .long _initialize

address_two:    .long 0x8801E000
address_one:    .long 0x8102000
Hmem_SetMMU:    .long _Hmem_SetMMU

