OUTPUT_ARCH(sh3)
ENTRY(_main)

/**
 * Linker script for user executables.
 */

MEMORY
{
	/* virtual memory, read-write segment */
	userram : o = 0x10000000, l = 64k

	/* virtual memory, read-exec segment */
	/* Unused for now (everything in the same segment is lighter... */
	/*userrom : o = 0x30000000, l = 64k*/
}

SECTIONS
{
	.text : {
		*(.text)
		*(.text.*)
	} > userram /*userrom*/

	.rodata : {
		*(.rodata);
		*(.rodata.*);
	} > userram /*userrom*/

	.data : {
		*(.data);
		*(.data.*);
	} > userram

	.bss : {
		*(.bss) *(COMMON);
	} > userram
}

