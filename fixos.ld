OUTPUT_ARCH(sh3)
ENTRY(_initialize)

/*
   Now, with an external bootloader, and targetting ELF output instead of
   raw binary, the linker script is realy simpler.
   For the kernel, most important things to know is were the RAM begins,
   because it will be fully copied in RAM by the bootloader.
   Sections can follow freely (.text, .rodata, .data and .bss)

   So, now, the main goal of this file is to reduce the number of sections
   and to order them if needed (VBR purpose for example).
 */


MEMORY
{
	osram :   o = 0x88000000, l = 128k  /* for fx-9860G */
}

SECTIONS
{
	/* place for command-line arguments */
	.cmdargs : {
		_cmdargs_begin = . ;
		. = . + 1024 ;
		_cmdargs_end = . ;
	} > osram

	.text : {
		*(.pretext);     /* init stuff */
		*(.text);
		*(.text.*);

		/* Exceptions and Interrupt handlers : due tu the SH3 VBR system, we have some restrictions */
		/* The handlers should reside at VBR relative position, in P1 or P2 protected space (0x8* or 0xA*) */
		/* There are 3 vectors offset called by the processor : VBR + 0x100, 0x400 and 0x600 */
		/* The bootstrap routine must copy them at these places, and bytes between handlers are undefined */
		/* Aligned in 0x100 because of ld issue*/
		. = ALIGN(0x100);
		_fixos_vbr = . ; /* VBR value. WARNING : VBR must point to P1 or P2 space! */
		. = _fixos_vbr + 0x100 ;  /* cleaner than "vbr = . - 0x100" */
		_bexhandler = ABSOLUTE(.) ;
		*(.handler.exception.pretext);
		*(.handler.exception);  /* general exception handler, VBR + 0x100 */
		_exhandler_size = ABSOLUTE(.) - _bexhandler ;
		. = _fixos_vbr + 0x400 ;
		_btlbhandler = ABSOLUTE(.) ;
		*(.handler.tlb.pretext);
		*(.handler.tlb);  /* TLB Miss handler, VBR + 0x400 */
		_tlbhandler_size = ABSOLUTE(.) - _btlbhandler ;
		. = _fixos_vbr + 0x600 ;
		_binthandler = ABSOLUTE(.) ;
		*(.handler.interrupt.pretext);
		*(.handler.interrupt);  /* Interrupt Handler, VBR + 0x600 */
		_inthandler_size = ABSOLUTE(.) - _binthandler ;
		*(.handler.stuff);
	} > osram

	.rodata : {
		*(.rodata);
		*(.rodata.*);
	} > osram

	.data : {
		*(.data);
		*(.data.*);
	} > osram

	.bss : {
		. = ALIGN(4);
		_bbss = . ;
		*(.bss) *(COMMON);
		_ebss = . ;

		/* symbol for the end of static data */
		_end_static_ram = . ;
		/* initial kernel stack is set here, inside the .bss section, but can be
		 * free'd once control is transfered to user-space init */
		. = ALIGN(1024);
		_begin_stack = . ;
		. = . + 4096 ;
		_end_stack = . ;
	} > osram
}

