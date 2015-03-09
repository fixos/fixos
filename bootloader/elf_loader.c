#include "elf_loader.h"
#include "smem_file.h"
#include <utils/strutils.h>


int elf_check_header(struct elf_header *h) {
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



int elf_load_segment(struct smem_file *filep, const struct elf_prog_header *ph) {
	smem_seek(filep, ph->offset, SEEK_SET);

	// copy segment directy in physical RAM (trust ELF informations)
	smem_read(filep, (void*)(ph->paddr), ph->filesz);
	
	// memset to 0 for the diff between memsz and filesz
	memset((void*)(ph->paddr + ph->filesz), 0, ph->memsz - ph->filesz);

	return 0;
}



int elf_load_kernel(const char *path, const char *cmdline, void *cmd_addr,
		int cmd_max)
{
	struct smem_file kernel;
	struct elf_header header;
	int ret = -1;

	unsigned int old_sr;
	unsigned int new_sr;

	// TODO avoid any exception during kernel copy!!!
	__asm__ volatile ("stc sr, %0" : "=r"(old_sr) );
	new_sr = old_sr | (1 << 28); // BL bit set to 1
	__asm__ volatile ("ldc %0, sr" : : "r"(new_sr) );

	if(smem_open(path, &kernel) == 0) {
		// check for ELF informations

		if(smem_read(&kernel, (void*)&header, sizeof(header)) != sizeof(header)) {
		}
		else if(elf_check_header(&header) != 0) {
		}
		else {
			struct elf_prog_header pheader;
			int i;
			int error;

			// load each segment in memory
			error = 0;
			for(i=0; i<header.phnum && !error; i++) {
				smem_seek(&kernel, header.phoff + i*sizeof(pheader), SEEK_SET);
				if(smem_read(&kernel, (void*)&pheader, sizeof(pheader))
						!= sizeof(pheader))
				{
					error = 1;
				}
				else if(pheader.type == ELFP_TYPE_LOAD) {
					if(elf_load_segment(&kernel, &pheader) != 0)
						error = 1;
				}
				else {
					// not a loadable segment, go to next
					//ret = -2;
				}
			}

			if(!error) {
				// copy the given command line arguments
				if(cmdline != NULL) {
					for(i=0; i<cmd_max && cmdline[i] != '\0'; i++)
						((char*)cmd_addr)[i] = cmdline[i];

					if(i >= cmd_max)
						i--;
					((char*)cmd_addr)[i] = '\0';
				}
				// success, jump to entry point!
				void (*entry)() = (void*)header.entry;
				entry();
				// never reach this line...
			}
			else
				ret = -2;
		}

	}
	else {
		// error
		ret = -2;
	}

	__asm__ volatile ("ldc %0, sr" : : "r"(old_sr) );
	return ret;
}

