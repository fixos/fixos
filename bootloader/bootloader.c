/**
 * The bootloader is an absolute requierement when booting directly from
 * G1A file, because Casio's OS TLB management is used, and copying
 * data into the RAM may overwrite Casio's OS data...
 *
 * The goal for now is only to implement a *minimalist* read-only API
 * for accessing to SMEM fs, find the good file, and doing same work than the
 * old 'bootstrap.s' (copying each sectio in the good place, etc...).
 *
 * In the medium-term, some evolution may be considered :
 *   - Make an independant G1A bootloader, wich allow to select manually the
 *     kernel file in the SMEM FS.
 *     In this case, relocation symbols need to be added as a header for the
 *     kernel binary (instead of G1A).
 *   - Improve this one to bootstrap itself, and then running without any
 *     virtual memory usage.
 *   - Allow to running kernel directly from EEPROM (this can be done using a
 *     special copying in the SMEM fs, with a hand-made optimisation of storage
 *     memory, to put the kernel binary in a continuous area and to hide this
 *     area to the Casio's OS).
 *
 * Technical notes :
 * the stack is set at the end of ram by bootloader_pre.s, but no area is set
 * for .bss, .rodata and .data, so any usage of these area here can cause an
 * issue at runtime...
 */

#include "minimalist_smemfs.h"

// need to be the name of the genereated G1A itself
// so, for now it's not possible to rename or to place the G1A file at
// any other place than the root on storage memory
#define G1A_FILE_PATH	"/fls0/FIXOS.g1a"


// symbols for relocation & co
extern char breloc;
extern char ereloc;


// compute offsets in G1A file
#define G1A_OFFSET_HEADER	512
extern char bbootstrap;
extern char romdata;


extern void initialize();


void bootloader_init() {
	const char *g1a_filename = G1A_FILE_PATH;
	struct _fscasio_file file;

	// look for the OS G1A file
	if(fscasio_fopen_ro(g1a_filename, &file) == 0) {
		// do the bootstrap itself
		unsigned int relocl = &ereloc - &breloc;

		// seek the begin of romdata in file
		unsigned int rompos = G1A_OFFSET_HEADER + (&romdata - &bbootstrap);
		fscasio_fseek(&file, rompos, SEEK_SET);

		// copy from EEPROM to RAM
		fscasio_fread(&breloc, 1, relocl, &file);

		// TODO bss initialization

		initialize();
	}
	else {
		// file probably doesn't exists
	}

}



