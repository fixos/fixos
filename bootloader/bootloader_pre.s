

	.section ".bootstrap"
	.global bootstrap
	.type bootstrap, @function

	.extern _bootloader_init
	.extern _boot_stack

	.extern _breloc
	.extern _ereloc

	.extern _bdata
	.extern _ebss

	.align 2

	! Initialize temporary stack and call the bootloader

bootstrap:
	mov.l stack_bootload, r15

	! copy .data section into RAM
	mov.l breloc, r1	! begin of relocation in virtual memory (ROM)
	mov.l ereloc, r2
	mov.l bdata, r3		! begin of RAM relocation of .data and .bss

	! quick (but not fast) implementation of memcpy
bdata_copy:
	cmp/hi r1, r2
	bf edata_copy
	mov.b @r1+, r4
	mov.b r4, @r3
	add #1, r3
	bra bdata_copy
	nop


edata_copy:
	! now initialize .bss section to 0x00
	mov.l ebss, r2
	mov #0, r4

bbss_init:
	cmp/hi r3, r2
	bf ebss_init
	mov.b r4, @r3
	add #1, r3
	bra bbss_init
	nop

ebss_init:

	! now jump to C function
	mov.l bootloader_init, r0
	jmp @r0
	nop



	.align 4
bootloader_init:
	.long _bootloader_init
stack_bootload:
	.long _boot_stack

breloc:
	.long _breloc
ereloc:
	.long _ereloc

bdata:
	.long _bdata
ebss:
	.long _ebss
