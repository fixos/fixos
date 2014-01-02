#ifndef _LOADER_ELFLOADER_ELF_H
#define _LOADER_ELFLOADER_ELF_H


/**
 * ELF file format definition and utilities.
 */

#include <utils/types.h>


/**
 * Header of ELF file.
 */
struct elf_header {
	uint8 magic[4];
	uint8 elf_class; // 32 or 64 bits
	uint8 endianness;
	uint8 ident_version;
	uint8 osabi;
	
	uint8 osabi_version;
	uint8 pad[7];

	uint16 type;
	uint16 machine;
	uint32 version;
	uint32 entry;

	uint32 phoff; // program header offset
	uint32 shoff; // section header offset

	uint32 flags; // machine specific flags

	uint16 ehsize; // ELF header size
	
	// program header size and number
	uint16 phentsize;
	uint16 phnum;

	// section header size and number
	uint16 shentsize;
	uint16 shnum;

	uint16 shstrndx; // index of section header with section names
};


#define ELF_MAG0		0x7F
#define ELF_MAG1		'E'
#define ELF_MAG2		'L'
#define ELF_MAG3		'F'

#define ELF_CLASS32		0x01

#define ELF_ENDIAN_BIG	0x02

#define ELF_IDENT_VERS	0x01

#define ELF_OSABI_NONE	0x00


#define ELF_TYPE_EXEC	2

#define ELF_MACHINE_SH	42

#define ELF_VERS		1



/**
 * Program header definition (only for ELF_CLASS32 class).
 */
struct elf_prog_header {
	uint32 type;
	uint32 offset;
	uint32 vaddr;
	uint32 paddr;
	uint32 filesz;
	uint32 memsz;
	uint32 flags;
	uint32 align;
};

#define ELFP_TYPE_NONE	0
#define ELFP_TYPE_LOAD	1

#define ELFP_FLAG_X		1
#define ELFP_FLAG_W		2
#define ELFP_FLAG_R		4



#endif //_LOADER_ELFLOADER_ELF_H
