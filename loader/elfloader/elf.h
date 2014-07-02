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
#define ELF_TYPE_DYN	3

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
#define ELFP_TYPE_SHLIB	5

#define ELFP_FLAG_X		1
#define ELFP_FLAG_W		2
#define ELFP_FLAG_R		4


/**
 * Section header definition.
 */
struct elf_section_header {
	uint32 name;
	uint32 type;
	uint32 flags;
	uint32 addr;
	uint32 offset;
	uint32 size;
	uint32 link;
	uint32 info;
	uint32 addralign;
	uint32 entsize;
};

#define ELFS_NAME_UNDEF		0

#define ELFS_TYPE_NULL		0
#define ELFS_TYPE_PROGBITS	1
#define ELFS_TYPE_SYMTAB	2
#define ELFS_TYPE_STRTAB	3

#define ELFS_FLAG_WRITE		0x1
#define ELFS_FLAG_ALLOC		0x2
#define ELFS_FLAG_EXECINSTR	0x4
#define ELFS_FLAG_MERGE		0x10
#define ELFS_FLAG_STRINGS	0x20


/**
 * Symbol table definition.
 */
struct elf_symbol {
	uint32 name;
	uint32 value;
	uint32 size;
	uint8 info;
	uint8 other;
	uint16 shndx;
};

#define ELFSYM_BIND(info)		((info) >> 4)
#define ELFSYM_TYPE(info)		((info) & 0x0F)
#define ELFSYM_INFO(bind,type)	( ((bind)<<4) | ((info)&0x0F) )

#define ELFSYM_BIND_LOCAL	0
#define ELFSYM_BIND_GLOBAL	1
#define ELFSYM_BIND_WEAK	2

#define ELFSYM_TYPE_NOTYPE	0
#define ELFSYM_TYPE_OBJECT	1
#define ELFSYM_TYPE_FUNC	2
#define ELFSYM_TYPE_SECTION	3
#define ELFSYM_TYPE_FILE	4
#define ELFSYM_TYPE_COMMON	5


#define ELFSYM_VISIBILITY(other)	((other) & 0x3)

#define ELFSYM_VISIBILITY_DEFAULT	0
#define ELFSYM_VISIBILITY_INTERNAL	1
#define ELFSYM_VISIBILITY_HIDDEN	2
#define ELFSYM_VISIBILITY_PROTECTED	3


#endif //_LOADER_ELFLOADER_ELF_H
