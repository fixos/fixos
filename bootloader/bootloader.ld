OUTPUT_ARCH(sh3)
ENTRY(bootstrap)

MEMORY
{
	/* bootram is the RAM physical location of global variables and stack
	   during bootloader step (this location is realy important because
	   no Casio's OS data should be overwrite during this step, and the
	   bootloader will copy directly kernel in RAM, so it must not
	   overwrite itself too...)
	   For now, considering RAM is 128kB, we use the 4 last kB for .bss and
	   .data (global variables), and some kBytes before for the stack.
	 */
	bootram :   o = 0x8801f000, l = 4k  /* for fx-9860G */
	rom   :   o = 0x00300200, l = 512k
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
		*(.text)
		*(.rodata)
		*(.rodata.*)
		_ebootstrap = . ;
		_romdata = . ;  /* relocation position for all RAM data (.data, .bss) */
	} > rom

	.rodata : AT( _romdata ) {
		_breloc = ABSOLUTE(.) ;
		_boot_stack = . ; /* last (exclusive) byte of bootloader stack */
		_bdata = . ;
		*(.data);
		*(.data.*);
		_edata = . ;

		_ereloc = ABSOLUTE(.);
	} > bootram

	.bss : {
		. = ALIGN(4);
		_bbss = . ;
		*(.bss) *(COMMON);
		_ebss = . ;
	} > bootram
}

