//#include "arch/sh/7705_casio.h"
#include "display/T6K11/T6K11.h"
#include "sys/terminal.h"
#include "keyboard/keyboard.h"
#include "arch/sh/interrupt.h"
#include "arch/sh/virtual_memory.h"
#include "arch/sh/mmu.h"
#include "sys/process.h"
#include "arch/sh/physical_memory.h"
#include "utils/log.h"
#include "fs/casio_smemfs/file_system.h"
#include "fs/protofs/file_system.h"
#include "fs/vfs.h"
#include "fs/vfs_op.h"

#include "loader/ramloader/loader.h"
#include "arch/sh/process.h"

#include "arch/sh/memory/eeprom.h"

extern unsigned int * euser;
extern unsigned int * buser;
extern unsigned int * osram_buser;
extern unsigned int * usersize;

extern void * fixos_vbr;  // see fixos.ld

void test();

void print_content(void *addr, int size);

/**
 * Function to help debugging fs's and vfs : print the tree
 * of ALL the mounted file system (wondereful)
 */
void ls_tree();

#define DBG_WAIT  while(is_key_down(K_EXE)); \
	while(!is_key_down(K_EXE))


static unsigned char eepromarray[] = {0x11, 0x12, 0x13, 0x14, 0x69, 0x69, 0x80, 0x0B, 0x50, 0x00, 0x42, 0x88, 0x88, 0x88, 0x88};
	

// Real entry point of the OS :
void init() {
	unsigned char vram[1024];
	int j;

	// init exceptions/interruptions handling
	interrupt_set_vbr(&fixos_vbr); // don't forget we want the ADDRESS of linked vbr 
	// tmp stuff
	 INTERRUPT_PRIORITY_IRQ0 = 0;
	 INTERRUPT_PRIORITY_IRQ1 = 0;
	 INTERRUPT_PRIORITY_IRQ2 = 0;
	 INTERRUPT_PRIORITY_IRQ3 = 0;
	 INTERRUPT_PRIORITY_IRQ4 = 0;
	 INTERRUPT_PRIORITY_IRQ5 = 0;
	 INTERRUPT_PRIORITY_PINT0_7 = 0;
	 INTERRUPT_PRIORITY_PINT8_15 = 0;
	 INTERRUPT_PRIORITY_DMAC = 0;
	 INTERRUPT_PRIORITY_SCIF0 = 0;
	 INTERRUPT_PRIORITY_SCIF2 = 0;
	 INTERRUPT_PRIORITY_ADC = 0;
	 INTERRUPT_PRIORITY_USB	= 0;
	 INTERRUPT_PRIORITY_TPU0 = 0;
	 INTERRUPT_PRIORITY_TPU1 = 0;
	 INTERRUPT_PRIORITY_TPU2 = 0;
	 INTERRUPT_PRIORITY_TPU3 = 0;
	 INTERRUPT_PRIORITY_TMU0 = 0;
	 INTERRUPT_PRIORITY_TMU1 = 0;
	 INTERRUPT_PRIORITY_TMU2 = 0;
	 INTERRUPT_PRIORITY_RTC = 0;
	 INTERRUPT_PRIORITY_WDT = 0;

	 INTERRUPT_PRIORITY_REF = 0;

	terminal_set_vram(vram);
	terminal_clear();

	set_kernel_print(&terminal_write);

	terminal_write("Boostrap... OK!\n");
	terminal_set_colors(TCOLOR_WHITE, TCOLOR_BLACK);
	terminal_write("WARNING : ");
	terminal_set_colors(TCOLOR_BLACK, TCOLOR_WHITE);
	terminal_write("it's working!\n  It's really strange...\n");

	mmu_init();
	pm_init_pages();

//	INTERRUPT_PRIORITY_PINT0_7 = 0xF;
//	INTERRUPT_PRIORITY_PINT8_15 = 0xF;

	interrupt_set_callback(INT_PINT_0_7, &test);
	interrupt_set_callback(INT_PINT_8_15, &test);
	
	while(is_key_down(K_EXE));
	while(!is_key_down(K_EXE));

	process_t *mock = process_from_asid(0xFF);

	vm_page_t page;
	int magic_integer = 589;
	int magic_offset;
	page.cache = 0;
	page.size = VM_DEFAULT_SIZE;
	page.valid = 1;
	page.vpn = (0x01000000 >> 10); // a translatable address
	page.ppn = ((((int)&magic_integer) & 0x1FFFFFFF) >> 10); // address of a stack page ;)

	magic_offset = ((int)(&magic_integer)) & 0x000003FF;

	vm_init_table(&(mock->vm));
	vm_add_entry(&(mock->vm), &page);

	while(is_key_down(K_EXE));
	while(!is_key_down(K_EXE));

	interrupt_inhibit_all(0);


	asm volatile ("trapa #50");

	int *surprise = (int*)(0x01000000 + magic_offset);

	printk("s: %p -> %p\nppn = %d, vpn = %d\n", &magic_integer, surprise, page.ppn, page.vpn);
	printk("[D] ASID = %d\n", mmu_getasid());
	DBG_WAIT;

	printk("Magic? %d\n", *surprise);
	*surprise = 986;

	unsigned int ppn1;
	if(! pm_get_free_page(&ppn1)) {
		printk("Page number : %d\n", ppn1);
	}

	// just for fun
	/*asm volatile ("mov #3, r0");
	asm volatile ("mov.l @r0, r1");
*/

	DBG_WAIT;

	// eeprom test
	int deviceid = eeprom_get_device_id();
	unsigned short eepromsz = eeprom_get_cfi(EEPROM_CFI_DEVICE_SIZE);

	printk("EEPROM DevID = %x\nEEPROM Size = %dB\n", deviceid, 1 << eepromsz);

	// try to erase EEPROM page
	unsigned int testaddr = 0xA01C0000;
	volatile unsigned short * shortaddr = (unsigned short *)testaddr;
	if(*shortaddr != 0xFFFF) {
		printk("Erasing EEPROM Sector at %p\n", shortaddr);
		eeprom_erase_sector(testaddr);
		while(*shortaddr != 0xFFFF);
		printk("Erased!\n");
	}

	// test programming eeprom!
	unsigned short valprev, valafter;
	valprev = *(volatile unsigned short *)(testaddr);
	eeprom_program_word(testaddr, 0x55AA);
	valafter = *(volatile unsigned short *)(testaddr);

	printk("EEPROM Write (bfr=%x, aft=%x)\n", valprev, valafter);

	while(*(volatile unsigned char *)(testaddr) != 0x55 && *(volatile unsigned char *)(testaddr) != 0xAA);
	printk("Done! After = %x\n", *(volatile unsigned short*)testaddr);


	DBG_WAIT;

	printk("Trying to write abritrary array in EEPROM.\n");
	eeprom_program_array(testaddr + 0x03, eepromarray, sizeof(eepromarray));


	DBG_WAIT;

	// test for user process load and run
	size_t processl = (void*)(&euser) - (void*)(&buser);
	void *bprocess = &osram_buser;
	void *eprocess = bprocess + processl;
	process_init();

	process_t *proc1;
	proc1 = process_alloc();
	printk("_buser %p (@%p)\n_euser %p (@%p)\nus %p (@%p)\n", &buser, &buser, &euser, &euser, &usersize, &usersize);
	printk("loading %p->%p\n", bprocess, eprocess);
	printk("user proc : %p\n", proc1);

	DBG_WAIT;

	ramloader_load(bprocess, processl, proc1);
	process_set_asid(proc1);
	mmu_setasid(proc1->asid);
	printk("[D] ASID = %d\n", mmu_getasid());
	printk("asid=%d, pid=%d\n", proc1->asid, proc1->pid);
	
	DBG_WAIT;

	arch_kernel_contextjmp(&(proc1->acnt));
	
	DBG_WAIT;


	vfs_init();
	vfs_register_fs(&smemfs_file_system, VFS_REGISTER_STATIC);
/*	vfs_mount("smemfs", NULL, VFS_MOUNT_ROOT);

	// test of the SMEM FS
	DBG_WAIT;

	// try to access to a file in a directory
	inode_t *curi;
	curi = vfs_resolve("/UEDIT/acore.cfg");
*/
	vfs_register_fs(& protofs_file_system, VFS_REGISTER_STATIC);
	vfs_mount("protofs", NULL, VFS_MOUNT_ROOT);

	DBG_WAIT;

	vfs_create("/", "dev", INODE_TYPE_PARENT, INODE_FLAG_READ | INODE_FLAG_EXEC, 0);
	vfs_create("/dev", "mouahah", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00010000);
	vfs_create("/dev", "pouet", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00010000);
	vfs_create("/", "usr", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/", "fmem", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/usr", "chose1", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/usr/chose1", "bidule2", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/usr/chose1/bidule2", "holy_shit", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00010000);
	vfs_create("/dev", "stdin", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00010000);
	vfs_create("/dev", "sda", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00010000);
	vfs_create("/", "mnt", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/mnt", "test", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);

	vfs_mount("smemfs", "/mnt/test", VFS_MOUNT_NORMAL);
	inode_t *curi = vfs_resolve("/mnt/test/UEDIT/acore.cfg");
	
	if(curi != NULL)
		printk("entry : '%s'\n    node=0x%x\n", curi->name, curi->node);
	else
		printk("VFS unfound file.\n");

	DBG_WAIT;

	inode_t *curi2 = vfs_resolve("/mnt/.././mnt/test/../../mnt/test/UEDIT/.././././UEDIT/acore.cfg");

	printk("with '..' and '.' : %s\n", curi2==curi ? "Ok" : "Fail");

	DBG_WAIT;

	ls_tree();

	/*inode_t *root, *curi;
	root = smemfs_get_root_node(smem);

	printk("root node=0x%d\n", root->node);
	DBG_WAIT;

	j=0;
	curi = smemfs_get_sub_node(root, 0);
	while(curi != NULL)
	{
		printk("entry%d : '%s'\n", j, curi->name);
		j++;
		
		vfs_free_inode(curi);
		curi = smemfs_get_sub_node(root, j);
		while(is_key_down(K_EXE));
		while(!is_key_down(K_EXE));
	}*/

/*
	PFC.PBCR.WORD = 0x5555;
	PB.DR.BYTE = 0b00000000;
	PFC.PACR.WORD = 0xAAAA;

	INTX.PINTER.WORD = 0b0000000011111111;
*/

	print_content((void*)0xa4550000, 240);
	j=0;
	while(1) {
		printk("Test a la con : %d\n", j);
		j++;
		//while(is_key_down(K_EXE));
		//while(!is_key_down(K_EXE));
		while(1);
	}

}

void test() {
	static int i = 0;

	printk("Ohohoh! Surprise^%d!\n", i);
	i++;
}

void print_content(void *addr, int size) {
	unsigned char *mem = addr;
	int i;
	for(i=0; i<size; i+=10) {
		printk("%p : %x %x %x %x %x %x %x %x %x %x\n", mem, mem[0], mem[1], mem[2], mem[3], mem[4], mem[5], mem[6], mem[7], mem[8], mem[9]);
		mem += 10;
	}
}


void print_inode_tree(inode_t *from, int tab)
{	
	char space[10] = "         ";
	inode_t *cur;
	int i;
	static int wait = 0;

	space[tab]='\0';
	i=0;

	cur = vfs_first_child(from);
	while(cur != NULL)
	{
		inode_t *swap = NULL;

		if(cur->type_flags & INODE_TYPE_PARENT) {
			printk("%s%s/\n", space, cur->name);
			print_inode_tree(cur, tab+1);
		}
		else
			printk("%s%s\n", space, cur->name);

		i++;
		swap = cur;
		cur = vfs_next_sibling(cur);
		vfs_release_inode(swap);

		wait++;
		if(wait > 6) {
			DBG_WAIT;
			wait = 0;
		}
	}

}

void ls_tree() {
	// for now, using low level routines
	inode_t *cur;
	
	cur = vfs_resolve("/");
	printk("/\n");
	print_inode_tree(cur, 1);

	vfs_release_inode(cur);
}
