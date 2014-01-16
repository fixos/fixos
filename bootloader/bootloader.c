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
#include "config_parser.h"
#include <fs/casio_smemfs/smemfs_primitives_ng.h>
#include <utils/strutils.h>


// absolute path of configuration file in SMEM
#define CFG_FILE_PATH	"/bootldr.cfg"

#define _CASIO_FS ((void*)(0xA0270000))
#define _CASIO_STORAGE_MEM ((void*)(0xA0000000)) 


// bootloader configuration
static char _cfg_message[30];
static int _cfg_quiet = 0;
static int _cfg_default_entry = 1;


// first entry (temp)
struct boot_entry {
	int defined;
	char label[30];
	char args[50];
	char type[20];
	char kernel[50];
};

#define ENTRY_NUMBER	3
struct boot_entry _entries[ENTRY_NUMBER];


#define SCOPE_GLOBAL	1
#define SCOPE_ENTRY		2


static void parse_config_file(struct smem_file *file) {
	int curtag;
	char tagbuf[50];
	unsigned int key;
	int i;
	int line = 1;
	struct boot_entry *cur_entry = NULL;
	int cur_scope = SCOPE_GLOBAL;


	// set all entries undefined
	for(i=0; i<ENTRY_NUMBER; i++) {
		_entries[i].defined = 0;
	}

	do {
		casio_Bdisp_AllClr_VRAM();
		curtag = config_read_tag(file, tagbuf, 50);
		

		casio_PrintXY(0, 0,
				curtag == CONFIG_TAG_ASSIGN ? "Assign" :
				(curtag == CONFIG_TAG_EOF ? "EOF" :
				 (curtag == CONFIG_TAG_SCOPE ? "Scope" :
				  (curtag == CONFIG_TAG_EMPTY ? "Empty" :
				   "Unknown...") ) ) , 0);

		if(curtag == CONFIG_TAG_SCOPE) {
			casio_PrintXY(0, 8, tagbuf, 0);

			if(strcmp(tagbuf, "Global") == 0) {
				cur_scope = SCOPE_GLOBAL;
			}
			else if (tagbuf[0] == 'E' && tagbuf[1] == 'n' && tagbuf[2] == 't'
					&& tagbuf[3] == 'r' && tagbuf[4] == 'y' && tagbuf[5] != '\0'
					&& tagbuf[6] == '\0')
			{
				// entry number N scope
				if(tagbuf[5] >= '1' && tagbuf[5] <= ('0' + ENTRY_NUMBER)) {
					cur_scope = SCOPE_ENTRY;
					cur_entry = &(_entries[ tagbuf[5] - '1' ]);
					cur_entry->defined = 1;
				}
			}
		}

		else if(curtag == CONFIG_TAG_ASSIGN) {
			casio_PrintXY(0, 8, tagbuf, 0);

			// check attribute name
			if(cur_scope == SCOPE_GLOBAL) {
				if(strcmp(tagbuf, "message") == 0) {
					config_read_assign_string(file, _cfg_message, 30);
				}
				else if(strcmp(tagbuf, "quiet") == 0) {
					config_read_assign_bool(file, &_cfg_quiet);
				}
				else if(strcmp(tagbuf, "default") == 0) {
					config_read_assign_int(file, &_cfg_default_entry);
				}
				else {
					// error
				}
			}

			else if(cur_scope == SCOPE_ENTRY) {
				if(strcmp(tagbuf, "label") == 0) {
					config_read_assign_string(file, cur_entry->label, 30);
				}
				else if(strcmp(tagbuf, "args") == 0) {
					config_read_assign_string(file, cur_entry->args, 50);
				}
				else if(strcmp(tagbuf, "type") == 0) {
					config_read_assign_string(file, cur_entry->type, 20);
				}
				else if(strcmp(tagbuf, "kernel") == 0) {
					config_read_assign_string(file, cur_entry->kernel, 50);
				}
				else {
					// error
				}
			}

			else {
				// error
			}
		}

		//casio_Bdisp_PutDisp_DD();
		//casio_GetKey(&key);

		line++;
	} while(curtag != CONFIG_TAG_EOF);
}


void bootloader_init() {
	struct smem_file cfgfile;
	int i;
	char title[23]; // 22 character + '\0'

	casio_Bdisp_AllClr_VRAM();
	casio_PrintXY(0, 0, "This is a test.", 0);
	casio_Bdisp_PutDisp_DD();

	smemfs_prim_init(_CASIO_FS, _CASIO_STORAGE_MEM);
	if(smem_open(CFG_FILE_PATH, &cfgfile) == 0) {
		casio_PrintXY(0, 8, "bootldr.cfg opened!", 0);
		parse_config_file(&cfgfile);
	}
	else {
		casio_PrintXY(0, 40, "Configuration file", 0);
		casio_PrintXY(18, 48, "not found!", 0);
		casio_Bdisp_PutDisp_DD();
		while(1);
	}

	// to have a beautiful title, use _cfg_message size to center it and add spaces if needed
	// each line contains 21 character and an other partialy
	int title_size;
	int title_offset;
	for(title_size=0; _cfg_message[title_size] != '\0'; title_size++);
	title_size = title_size > 22 ? 22 : title_size;
	title_offset = (22-title_size)/2;

	for(i=0; i<22; i++)
		title[i] = '=';
	title[i] = '\0';

	// copy the content
	for(i=0; i<22 && i<title_size; i++)
		title[i+title_offset] = _cfg_message[i];


	int selected = _cfg_default_entry;
	while(1) {
		int i;
		unsigned int key;

		// display menu
		casio_Bdisp_AllClr_VRAM();
		casio_PrintXY(0, 0, title, 1);

		for(i=0; i<ENTRY_NUMBER; i++) {
			casio_PrintXY(6, 8*i + 8, _entries[i].label, 0);
		}


		casio_PrintXY(0, 8*selected, ">", 0);

		// display informations about current line

		casio_PrintXY(0, 44, _entries[selected-1].type, 0);
		casio_PrintXY(50, 52, _entries[selected-1].kernel, 0);
		
		casio_Bdisp_PutDisp_DD();


		// get pressed key
		casio_GetKey(&key);
		if(key == KEY_CTRL_UP) {
			int tempsel = selected - 1;
			while(tempsel >= 1 && _entries[tempsel-1].defined == 0)
				tempsel--;

			if(tempsel >= 1 && _entries[tempsel-1].defined != 0)
				selected = tempsel;
		}
		else if(key == KEY_CTRL_DOWN) {
			int tempsel = selected + 1;
			while(tempsel < ENTRY_NUMBER && _entries[tempsel-1].defined == 0)
				tempsel++;

			if(tempsel < ENTRY_NUMBER && _entries[tempsel-1].defined != 0)
				selected = tempsel;
		}

		else if(key == KEY_CTRL_EXE  || key == KEY_CTRL_SHIFT) {
			// boot on current entry
		}
	}
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



