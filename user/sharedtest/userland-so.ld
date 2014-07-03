OUTPUT_ARCH(sh3)
ENTRY(default_entry)

/**
 * Linker script for user shared libraries.
 */

MEMORY
{
	/* virtual memory, relative to the dynamic relocation start address */
	relative (rwx) : o = 0x00000000, l = 256k
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
		default_entry = . ;
		*(.text)
		*(.text.*)
	} : text

	.rodata : {
		*(.rodata);
		*(.rodata.*);
	}

	GROUP ALIGN(1024) : {
		.data : ALIGN(1024){
			*(.plt);
			*(.data);
			*(.data.*);
		} : data

		.relocgot : {
			RELOC_GOT_B = . ;
			*(.got.plt)
			*(.got)
			RELOC_GOT_E = . ;
		}

		.bss : {
			*(.bss) *(COMMON);
		}
	}
}

