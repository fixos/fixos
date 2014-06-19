/**
 * The bootloader is an absolute requierement when booting directly from
 * G1A file, because Casio's OS TLB management is used, and copying
 * data into the RAM may overwrite Casio's OS data...
 *
 * The goal for now is only to implement a *minimalist* read-only API
 * for accessing to SMEM fs, find the good file, and doing same work than the
 * old 'bootstrap.s' (copying each sectio in the good place, etc...).
 *
 * A user interface and configuration file were added to provide some
 * flexibility to the bootloader, and it's now possible to load kernel directly
 * from an ELF file.
 *
 * In the medium-term, some evolution may be considered :
 *   - Improve this one to bootstrap itself, and then running without any
 *     virtual memory usage.
 *   - Allow to running kernel directly from EEPROM (this can be done using a
 *     special copying in the SMEM fs, with a hand-made optimisation of storage
 *     memory, to put the kernel binary in a continuous area and to hide this
 *     area to the Casio's OS).
 *   - Add some informative text if any error occurs during config file parsing
 *     or kernel loading...
 *
 * Technical notes :
 * The .data and .bss sections are limited to few KiB (see bootloader.ld for
 * details). In addition, stack must be as small as possible.
 * Later, additionnal tests on kernel destination address and size should be
 * added to ensure it will not overwrite bootloader data before to be fully
 * loaded.
 */

#include "casio_syscalls.h"
#include "smem_file.h"
#include "config_parser.h"
#include "elf_loader.h"
#include <fs/casio_smemfs/smemfs_primitives_ng.h>
#include <utils/strutils.h>

// absolute path of configuration file in SMEM
#define CFG_FILE_PATH	"/bootldr.cfg"

#define _CASIO_FS ((void*)(0xA0270000))
#define _CASIO_STORAGE_MEM ((void*)(0xA0000000)) 

// for now, command line arguments are designed to work for FiXos kernel design
#define CMDLINE_ADDR	(void*)(0x88000000)
#define CMDLINE_MAX		1024


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
		

		/*casio_PrintXY(0, 0,
				curtag == CONFIG_TAG_ASSIGN ? "Assign" :
				(curtag == CONFIG_TAG_EOF ? "EOF" :
				 (curtag == CONFIG_TAG_SCOPE ? "Scope" :
				  (curtag == CONFIG_TAG_EMPTY ? "Empty" :
				   "Unknown...") ) ) , 0);
		*/

		if(curtag == CONFIG_TAG_SCOPE) {
			//casio_PrintXY(0, 8, tagbuf, 0);

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
			//casio_PrintXY(0, 8, tagbuf, 0);

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


static void bootloader_boot_entry(struct boot_entry *entry) {
	char *cmdline;
	int i;

	// boot on current entry
	// TODO test entry type
	cmdline = entry->args;
	if(cmdline[0] == '\0')
		cmdline = NULL;

	elf_load_kernel(entry->kernel, cmdline, CMDLINE_ADDR,
			CMDLINE_MAX);
}


void bootloader_init() {
	struct smem_file cfgfile;
	int i;
	unsigned int key;

	casio_Bdisp_AllClr_VRAM();
	
	smemfs_prim_init(_CASIO_FS, _CASIO_STORAGE_MEM);
	if(smem_open(CFG_FILE_PATH, &cfgfile) == 0) {
		parse_config_file(&cfgfile);
	}
	else {
		casio_PrintMini(0, 40, "Configuration file not found!", 0);
		casio_PrintXY(0, 46, "Check '" CFG_FILE_PATH "' in SMEM.", 0);
		casio_Bdisp_PutDisp_DD();
		while(1)
			casio_GetKey(&key);
	}



	int selected = _cfg_default_entry;
	if(_cfg_quiet == 0) {
		// interactive mode

		char title[33]; // 32 character + '\0'
		const char *separator = "--------------------------------";

		// to have a beautiful title, use _cfg_message size to center it and add spaces if needed
		// each line contains 32 character and an other partialy
		int title_size;
		int title_offset;
		for(title_size=0; _cfg_message[title_size] != '\0'; title_size++);
		title_size = title_size > 32 ? 32 : title_size;
		title_offset = (32-title_size)/2;

		for(i=0; i<32; i++)
			title[i] = '=';
		title[i] = '\0';

		// copy the content
		for(i=0; i<title_size; i++)
			title[i+title_offset] = _cfg_message[i];

		while(1) {
			int i;

			// display menu
			casio_Bdisp_AllClr_VRAM();
			casio_PrintMini(0, 0, title, 0x12);

			for(i=0; i<ENTRY_NUMBER; i++) {
				casio_PrintMini(6, 6*i + 10, _entries[i].label, 0);
				casio_PrintMini(0, 6*i + 10, "-" , 0);
			}


			casio_PrintMini(0, 6*selected + 4, ">", 0);

			// display informations about current line (y base position : 10 + 6*3 + 4 = 32
			casio_PrintMini(0, 32, separator, 0x12);
			casio_PrintMini(20, 32, "Entry info", 0x12);

			casio_PrintMini(0, 38, "type : ", 0);
			casio_PrintMini(4*7, 38, _entries[selected-1].type, 0);

			casio_PrintMini(0, 44, "file : ", 0);
			casio_PrintMini(4*7, 44, _entries[selected-1].kernel, 0);

			casio_PrintMini(0, 50, "args : ", 0);
			casio_PrintMini(4*7, 50, _entries[selected-1].args, 0);
			
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
				bootloader_boot_entry(& _entries[selected-1]);

				// if function return, we have a problem...
				casio_Bdisp_AllClr_VRAM();

				casio_PrintMini(0, 24, "Problem when loading kernel.", 0);
				casio_PrintMini(0, 30, "(does the file exist?)", 0);

				casio_Bdisp_PutDisp_DD();
				casio_GetKey(&key);
			}
		}
	}
	else {
		// quiet mode, do not echo any thing and start immediatly default entry
		bootloader_boot_entry(& _entries[selected-1]);

		// if function return, we have a problem...
		casio_Bdisp_AllClr_VRAM();

		casio_PrintMini(0, 17, "Bootloader is set in quiet mode!" , 0x12);

		casio_PrintMini(0, 24, "Problem when loading kernel.", 0);
		casio_PrintMini(0, 30, "(does the file exist?)", 0);

		casio_Bdisp_PutDisp_DD();
		while(1)
			casio_GetKey(&key);	
	}
}



