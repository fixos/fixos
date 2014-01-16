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

#include "casio_syscalls.h"
#include "smem_file.h"
#include <fs/casio_smemfs/smemfs_primitives_ng.h>

// absolute path of configuration file in SMEM
#define CFG_FILE_PATH	"/bootldr.cfg"

#define _CASIO_FS ((void*)(0xA0270000))
#define _CASIO_STORAGE_MEM ((void*)(0xA0000000)) 

void bootloader_init() {
	struct smem_file cfgfile;

	casio_Bdisp_AllClr_VRAM();
	casio_PrintXY(0, 0, "This is a test.", 0);
	casio_Bdisp_PutDisp_DD();

	smemfs_prim_init(_CASIO_FS, _CASIO_STORAGE_MEM);
	if(smem_open(CFG_FILE_PATH, &cfgfile) == 0) {
		char test[10];
		int i;

		casio_PrintXY(0, 9, "bootloader.cfg opened!", 0);
		casio_Bdisp_PutDisp_DD();

		for(i=0; i<9; i++)
			test[i] = smem_readchar(&cfgfile);
		test[i] = '\0';

		casio_PrintXY(6, 18, test, 0);
		casio_Bdisp_PutDisp_DD();
	}

	while(1);
/*
	// look for the OS G1A file
	if(fscasio_fopen_ro(g1a_filename, &file) == 0) {
		// do the bootstrap itself
		unsigned int relocl = &ereloc - &breloc;
		char *bss;

		// seek the begin of romdata in file
		unsigned int rompos = G1A_OFFSET_HEADER + (&romdata - &bbootstrap);
		fscasio_fseek(&file, rompos, SEEK_SET);

		// copy from EEPROM to RAM
		fscasio_fread(&breloc, 1, relocl, &file);

		// TODO better bss initialization
		for(bss=&bbss; bss!=&ebss; bss++)
			*bss = 0x00;

		//initialize();
		while(1);
	}
	else {
		// file probably doesn't exists
	}
*/
}



