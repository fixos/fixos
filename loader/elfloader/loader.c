#include "loader.h"
#include "elf.h"

#include <fs/vfs_file.h>
#include <arch/sh/virtual_memory.h>
#include <sys/memory.h>
#include <utils/log.h>


// check static expected informations of the given header, returns non-zero
// if anything seems to be wrong
static int check_elf_header(struct elf_header *header);


// read the given segment from elf file, copy it in RAM using physical page
// allocation, and add corresponding virtual memory mapping
static int elfloader_load_segment(struct file *filep, const struct elf_prog_header *pheader, process_t *dest);


int elfloader_load(struct file *filep, process_t *dest) {
	struct elf_header header;

	// first step : read ELF header and check if it seems correct
	if(vfs_read(filep, &header, sizeof(header)) != sizeof(header)) {
		printk("elfloader: unable to read header\n");
		return -1;
	}
	else if(check_elf_header(&header) != 0) {
		printk("elfloader: unexpected value in header\n");
		return -1;
	}
	else {
		struct elf_prog_header pheader;
		// now, basic implementation : load the first LOAD program entry

		vfs_lseek(filep, header.phoff, SEEK_SET);
		if(vfs_read(filep, &pheader, sizeof(pheader)) != sizeof(pheader)) {
			printk("elfloader: unable to read prog header\n");
			return -1;
		}

		vm_init_table(&(dest->vm));
		if(pheader.type == ELFP_TYPE_LOAD
				&& elfloader_load_segment(filep, &pheader, dest) == 0)
		{
			vm_page_t page;
			void *pageaddr;

			// alloc physical page and set it as the VM process stack
			pageaddr = mem_pm_get_free_page(MEM_PM_CACHED);
			if(pageaddr == NULL) {
				printk("elfloader: no physical page\n");
				return -1;
			}


			page.size = VM_PAGE_1K;
			page.cache = 1;
			page.valid = 1;
			page.ppn = PM_PHYSICAL_PAGE(pageaddr);
			// warning : the first valid stack address is in the previous page
			// of the 'base stack address'!
			page.vpn = VM_VIRTUAL_PAGE(ARCH_UNEWPROC_DEFAULT_STACK) - 1;
			vm_add_entry(&(dest->vm), &page);

			// set kernel stack address, for now any physical memory
			pageaddr = mem_pm_get_free_page(MEM_PM_CACHED);
			if(pageaddr == NULL) {
				printk("elfloader: no physical page\n");
				return -1;
			}

			// kernel stack begins at the end of pageaddr
			dest->acnt.kernel_stack = pageaddr + PM_PAGE_BYTES; 
			dest->acnt.reg[15] = ARCH_UNEWPROC_DEFAULT_STACK;
			dest->acnt.pc = header.entry;
			dest->acnt.sr = ARCH_UNEWPROC_DEFAULT_SR;
		}
		else {
			printk("elfloader: unable to load segment\n");
			return -1;
		}
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



int elfloader_load_segment(struct file *filep,
		const struct elf_prog_header *ph, process_t *dest)
{
	int i;
	vm_page_t page;
	void *pageaddr;
	void *vm_segaddr;

	page.size = VM_PAGE_1K;
	page.cache = 1;
	page.valid = 1;

	vfs_lseek(filep, ph->offset, SEEK_SET);

	vm_segaddr = (void*)(ph->vaddr);
	for(i=0; i<ph->memsz; i += PM_PAGE_BYTES, vm_segaddr += PM_PAGE_BYTES) {
		ssize_t nbread;

		pageaddr = mem_pm_get_free_page(MEM_PM_CACHED);
		if(pageaddr == NULL) {
			printk("elfloader: no physical page\n");
			// TODO really dirty way to exit, need to clean all done job!
			return -1;
		}

		// if we have a page, copy data from file
		nbread = vfs_read(filep, pageaddr, PM_PAGE_BYTES);
		printk("[I] %d bytes read from ELF.", nbread);

		page.ppn = PM_PHYSICAL_PAGE(pageaddr);
		page.vpn = VM_VIRTUAL_PAGE(vm_segaddr);
		vm_add_entry(&(dest->vm), &page);

		printk("[I] Added VM page (%p -> %p)\n", pageaddr, vm_segaddr);
	}

	return 0;
}
