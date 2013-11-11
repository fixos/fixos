

	.section ".bootstrap"
	.global bootstrap
	.type bootstrap, @function

	.extern _bootloader_init

	.align 2

	! Initialize temporary stack and call the bootloader

bootstrap:
	mov.l stack_bootload, r15
	mov.l bootloader_init, r0
	jmp @r0
	nop



  .align 4
bootloader_init:
  .long _bootloader_init
stack_bootload:
  .long 0x88010000  ! temp stack address, at RAM + 64kio
