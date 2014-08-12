OUTPUT_ARCH(sh3)
ENTRY(_main)

/**
 * Linker script for user executables.
 */

MEMORY
{
	/* virtual memory, read-write segment */
	userram (rwx) : o = 0x10000000, l = 256k

	/* virtual memory, read-exec segment */
	/* Unused for now (everything in the same segment is lighter... */
	/*userrom : o = 0x30000000, l = 64k*/
}

PHDRS
{
	sharedlibs PT_SHLIB;
	text PT_LOAD ;
	data PT_LOAD ;
}

SECTIONS
{
	.libraries : {
		*(.libraries)
	} : sharedlibs

	.text : {
		*(.text)
		*(.text.*)
	} : text

	.rodata : {
		*(.rodata);
		*(.rodata.*);
	}

	.data ALIGN(1024) : {
		*(.plt);
		*(.data);
		*(.data.*);

		/* bss section included to avoid missaligned segment */
		*(.bss);
		*(.bss.*)
		*(COMMON);
	} : data
}

