OUTPUT_ARCH(sh3)
ENTRY(bootstrap)

MEMORY
{
	osram :   o = 0x88000000, l = 64k  /* for fx-9860G */
	rom   :   o = 0x00300200, l = 512k

	userram : o = 0x10000000, l = 64k /* for user data (virtual address) */
}

SECTIONS
{
	/*
	   For the OS, the memory mapping of the sections is a bit strange.
	   If FiXos is launched by the Casio OS, we need to copy it from the
	   ROM storage (mapped in 0x00300200 by the MMU) to the real start of
	   the RAM (0x88000000 using section P1, or 0xA8000000 using P2)
	   So, all OS data (.text, .rodata, .data values... sections of FiXos)
	   are dumped by the .bootstrap section code into the RAM, and then
	   the bootstrap function call initialize().
	   Due to objcopy operation from ELF to binary, we must rename some sections...
	 */

	.text : {
		_bbootstrap = . ;
		*(.bootstrap)
			_ebootstrap = . ;
		_romdata = . ;  /* relocation position for all data (.text, .data, .rodata... origin sections) */
	} > rom

	.rodata : AT( _romdata ) {
		_breloc = ABSOLUTE(.) ;
		_btext = . ;
		*(.pretext)     /* init stuff */
			*(.text)
			*(.text.*)
			_etext = . ;

		_bdata = . ;
		*(.rodata);
		*(.rodata.*);
		*(.data);
		*(.data.*);
		_edata = . ;

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
		*(.handler.tlb);  /* TLB Miss handler, VBR + 0x400 */
		_tlbhandler_size = ABSOLUTE(.) - _btlbhandler ;
		. = _fixos_vbr + 0x600 ;
		_binthandler = ABSOLUTE(.) ;
		*(.handler.interrupt);  /* Interrupt Handler, VBR + 0x600 */
		_inthandler_size = ABSOLUTE(.) - _binthandler ;
		*(.handler.stuff);
		
		. = ALIGN(1024);
		_osram_buser = . ;

		_ereloc = ABSOLUTE(.) + SIZEOF(.userdata);

		_usersize = SIZEOF(.userdata) ;
	} > osram

	.userdata : AT( _romdata + SIZEOF(.rodata)  ) {
		. = ALIGN(1024);
		_buser = . ;
		*(.user.pretext) ;
		*(.user.*) ;
		_euser = . ;
	} > userram

	.bss : {
		. = . + SIZEOF(.userdata);
		. = ALIGN(4);
		_bbss = . ;
		*(.bss) *(COMMON);
		_ebss = . ;

		/* symbol for the end of static data */
		_end_static_ram = . ;
	} > osram
}

