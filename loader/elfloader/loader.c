#include "loader.h"
#include "elf.h"
#include "elf_utils.h"

#include <fs/vfs_file.h>
#include <fs/vfs.h>
#include <sys/memory.h>
#include <utils/log.h>
#include <utils/strutils.h>
#include <sys/user.h>
#include <sys/mem_area.h>


// check static expected informations of the given header, returns non-zero
// if anything seems to be wrong
int check_elf_header(struct elf_header *header);


/**
 * Read the given segment from elf file, copy it in RAM using physical page
 * allocation, and add corresponding virtual memory mapping.
 * If offset is not NULL, the given address is used as base offset of
 * virtual addresses (useful for shared objects).
 */
static int elfloader_load_segment(struct file *filep, void *offset,
		const struct elf_prog_header *pheader, struct process *dest);

/**
 * Load all segment described in the given file, using offset as base virtual
 * address if not NULL, into the process dest.
 * If file is succefully loaded, header is filled with the ELF file header.
 * If flag contain ELF_LOAD_SET_BRK, the last program header in memory will
 * be used as base address for the heap of dest (dest->initial_brk will be
 * set to the first byte beyond the last data section allocated).
 */
#define ELF_LOAD_SET_BRK	(1<<0)
int elfloader_load_all(struct file *filep, void *offset, struct process *dest,
		struct elf_header *header, int flags);


int elfloader_load(struct file *filep, struct process *dest) {
	struct elf_header header;

	if(elfloader_load_all(filep, NULL, dest, &header, ELF_LOAD_SET_BRK) == 0) {
		void *pageaddr;

		// set user stack (the size used *is* a maximum, not the allocated one)
		struct mem_area *user_stack;
		user_stack = mem_area_make_anon((void*)(ARCH_UNEWPROC_DEFAULT_STACK 
				- PROCESS_DEFAULT_STACK_SIZE), PROCESS_DEFAULT_STACK_SIZE);
		mem_area_insert(dest, user_stack);


		// set kernel stack address, for now any physical memory
		pageaddr = arch_pm_get_free_page(MEM_PM_CACHED);
		if(pageaddr == NULL) {
			printk(LOG_ERR, "elfloader: no physical page\n");
			return -1;
		}

		// kernel stack begins at the end of pageaddr (contains the first
		// context struct)
		dest->kernel_stack = pageaddr + PM_PAGE_BYTES; 
		dest->acnt = dest->kernel_stack - sizeof(struct _context_info);
		dest->acnt->reg[15] = ARCH_UNEWPROC_DEFAULT_STACK;
		dest->acnt->reg[4] = 0;
		dest->acnt->reg[5] = 0;
		dest->acnt->reg[6] = 0;
		dest->acnt->pc = header.entry;
		dest->acnt->sr = ARCH_UNEWPROC_DEFAULT_SR;
		dest->acnt->previous = NULL;
	}

	return 0;
}


int elfloader_load_all(struct file *filep, void *offset, struct process *dest,
		struct elf_header *header, int flags)
{
	// first step : read ELF header and check if it seems correct
	if(elf_get_header(filep, header) == 0) {
		if(check_elf_header(header) != 0) {
			printk(LOG_ERR, "elf: unexpected value in header\n");
			return -1;
		}
		else {
			struct elf_prog_header pheader;
			int curph;

			// to set brk if needed
			void *cur_brk = NULL;

			// now, do appropriate job for each program header, depending their type
			for(curph=0; curph < header->phnum; curph++) {
				vfs_lseek(filep, header->phoff + curph*sizeof(pheader), SEEK_SET);
				if(vfs_read(filep, &pheader, sizeof(pheader)) != sizeof(pheader)) {
					printk(LOG_ERR, "elfloader: unable to read prog header %d\n", curph);
					return -1;
				}

				if(pheader.type == ELFP_TYPE_LOAD) {
					if(elfloader_load_segment(filep, offset, &pheader, dest) == 0) {
						//loadable = 1;
						if(pheader.vaddr + pheader.memsz > (uint32)cur_brk) {
							cur_brk = (void*)(pheader.vaddr + pheader.memsz);
						}
					}
					else {
						printk(LOG_ERR, "elfloader: unable to load segment\n");
						// TODO free loaded segments
						return -1;
					}
				}
				else if(pheader.type == ELFP_TYPE_SHLIB) {
#ifdef CONFIG_ELF_SHARED
					char buf[30];
					int index = 0;

					while(index < pheader.filesz) {
						vfs_lseek(filep, pheader.offset + index, SEEK_SET);
						if(vfs_read(filep, buf, 29) > 0) {
							int strsize;
							buf[29] = '\0';
							
							// no strlen
							for(strsize=0; buf[strsize] != '\0'; strsize++);
							index += strsize;

							if(strsize > 0) {
								printk(LOG_DEBUG, "elfloader: need library '%s'\n", buf);
								elfloader_load_dynlib(buf, dest);
							}
							else
								break;
						}
					}
#else
					printk(LOG_ERR, "elfloader: no shared library support!\n");
#endif //CONFIG_ELF_SHARED
				}

			}

			if(flags & ELF_LOAD_SET_BRK) {
				dest->initial_brk = cur_brk;
			}
		}
	}
	else {
		return -1;
	}



	return 0;
}



int check_elf_header(struct elf_header *h) {
	int ret=-1;

	if(h->magic[0] == ELF_MAG0 && h->magic[1] == ELF_MAG1 && h->magic[2] == ELF_MAG2
			&& h->magic[3] == ELF_MAG3)
	{
		// it's an ELF file, now check if it's OK for our architecture
		if(h->elf_class == ELF_CLASS32 && h->endianness == ELF_ENDIAN_BIG
				&& h->ident_version == ELF_IDENT_VERS
				&& h->osabi == ELF_OSABI_NONE)
		{
			// ELF identification is as expected, now check other fields
			if(h->type == ELF_TYPE_EXEC && h->machine == ELF_MACHINE_SH
					&& h->version == ELF_VERS
					&& h->ehsize == sizeof(struct elf_header)
					&& h->phentsize == sizeof(struct elf_prog_header))
			{
				// okay, this ELF header is fine!
				ret = 0;
			}
		}
	}

	return ret;
}


#include <fs/casio_smemfs/file.h>

int elfloader_load_segment(struct file *filep, void *offset,
		const struct elf_prog_header *ph, struct process *dest)
{
	if(ph->vaddr % PM_PAGE_BYTES == 0) {
		int prot;

		// TODO use real permissions from ELF
		prot = MEM_AREA_PROT_R | MEM_AREA_PROT_W | MEM_AREA_PROT_X;
		return vfs_map_area(filep, ph->memsz, ph->offset, offset + ph->vaddr,
				MEM_AREA_PARTIAL | prot, ph->filesz, dest);
	}
	else {
		printk(LOG_ERR, "elfloader: segment begin not page-aligned.\n");
		return -1;
	}
}


#ifdef CONFIG_ELF_SHARED
int elfloader_load_dynlib(const char *soname, struct process *dest) {
	char absname[40] = "/mnt/smem/lib/";
	struct inode *ilib;
	struct file *lib;

	// TODO replace by strncpy
	strcpy(absname + sizeof("/mnt/smem/lib/")-1, soname);
	absname[39] = '\0';

	ilib = vfs_resolve(absname);
	if(ilib != NULL && (lib = vfs_open(ilib, O_RDONLY)) != NULL) {
		// TODO runtime offset calculation
		void *offset = (void*)0x15000000;

		struct elf_header header;
		if(elfloader_load_all(lib, offset, dest, &header, 0) == 0) {
			struct elf_section_header symtab;

			printk(LOG_DEBUG, "elfloader: library '%s' loaded!\n", absname);
			// FIXME don't work anymore with the new memory area system, should
			// be re-written!
			if(elf_get_symtab(lib, &header, &symtab) == 0) {
				struct elf_symbol sym;
				uint32 *reloc_got_b = NULL;
				uint32 *reloc_got_e = NULL;

				if(elf_symbol_lookup(lib, &symtab, &header, "RELOC_GOT_B", &sym)
						== 0)
				{
					reloc_got_b = offset + sym.value;
				}

				if(elf_symbol_lookup(lib, &symtab, &header, "RELOC_GOT_E", &sym)
						== 0)
				{
					reloc_got_e = offset + sym.value;
				}

				printk(LOG_DEBUG, "elfloader: RELOC_GOT_B=%p nb=%d\n", reloc_got_b,
						reloc_got_e - reloc_got_b);

				for( ; reloc_got_b < reloc_got_e; reloc_got_b++) {
					uint32 old;
					old = user_read_32(reloc_got_b, dest);
					user_write_32(reloc_got_b,  old + (uint32)offset, dest);
					// TODO dyn reloc
				}

				// done, add the library to process
				dest->shared.file = lib;
				dest->shared.offset = offset;

			}
		}
		return 0;
	}
	else {
		printk(LOG_ERR, "elfloader: unable to find dynamic lib '%s'\n", absname);
		return -1;
	}
}



void *elfloader_resolve_dynsymbol(const char *name, struct process *target) {
	int i;
	struct elf_shared_lib *cur;
	void *ret = NULL;

	for(i=0, cur = & target->shared; i<1; i++, cur++) {
		struct elf_header header;

		if(elf_get_header(cur->file, &header) == 0) {
			if(check_elf_header(&header) != 0) {
				printk(LOG_ERR, "elf: unexpected value in header\n");
				return NULL;
			}

			struct elf_section_header symtab;

			if(elf_get_symtab(cur->file, &header, &symtab) == 0) {
				struct elf_symbol sym;

				if(elf_symbol_lookup(cur->file, &symtab, &header, name, &sym) == 0) {
					ret = cur->offset + sym.value;
					printk(LOG_DEBUG, "elfloader: '%s' solved @%p\n", name, ret);
				}
			}
		}
	}

	return ret;
}



int sys_dynbind(const char *symbol, void **dest) {
	void *ret;
	struct process *cur;

	cur = process_get_current();
	ret = elfloader_resolve_dynsymbol(symbol, cur);
	if(ret != NULL) {
		if(dest != NULL)
			*dest = ret;
		return 0;
	}
	return -1;
}
#endif //CONFIG_ELF_SHARED
