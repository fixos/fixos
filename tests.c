#include "tests.h"

#include "device/display/T6K11/T6K11.h"
#include "device/terminal/generic_early_term.h"
#include "device/keyboard/keyboard.h"
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
#include "fs/vfs_file.h"

#include "loader/ramloader/loader.h"
#include "loader/elfloader/loader.h"
#include "arch/sh/process.h"

#include "arch/sh/memory/eeprom.h"

#include "device/sd_card.h"
#include "arch/sh/kdelay.h"

#include "device/device_registering.h"


void test_sdcard() {
	// SD Card communication tests
	printk("Trying to init SD card...\n");
	//printk("Trying to send SD command...\n");

	sd_resp128_t resp;
	union sd_reg_csd csd;

	//sd_init_registers();
	
	//printk("SD registers initialized..\n");
	
	//int sdret = sd_send_command(0, 0, 0);

	//sd_get_resp32(&resp);
	//printk("sd_send ret = %d\nCMD0 resp = {%p}\n", sdret, (void*)(resp.w0 + (resp.w1 << 16) ));

	int sdret = sd_init();
	printk("Done, SD init = %d\n", sdret);
	
	sdret = sd_send_command(2, 0x00000000);

	sd_get_resp128(&resp);
	printk("sd_send ret = %d\nCMD2 resp = {\n%p %p\n%p %p }\n", sdret, (void*)(resp.r0), (void*)(resp.r1), (void*)(resp.r2), (void*)(resp.r3));


	DBG_WAIT;

	// publish new RCA
	
	unsigned short card_rca;
	sdret = sd_send_command(3,  0x00000000);

	sd_resp32_t resp32;

	sd_get_resp32(&resp32);
	card_rca = resp32 >> 16;
	printk("sd_send ret = %d\nCMD3 resp = {%p}\nNew RCA = %d\n", sdret, (void*)(resp32), card_rca);


	DBG_WAIT;

	// get CSD with given RCA
	sdret = sd_send_command(9, (card_rca << 16) + 0x0000);

	sd_get_resp128(&(csd.resp));
	printk("sd_send ret = %d\nCMD9 resp = {\n%p %p\n%p %p }\n", sdret, (void*)(csd.resp.r0), (void*)(csd.resp.r1), (void*)(csd.resp.r2), (void*)(csd.resp.r3));

	printk("c_size(%d) sz_mult(%d) len(%d)\n", SD_CSD_GET_C_SIZE(csd), csd.content.c_size_mult, csd.content.read_bl_len);

	// compute max size
	int card_size = (SD_CSD_GET_C_SIZE(csd)+ 1) * (1 << (csd.content.c_size_mult +2))
			* (1 << csd.content.read_bl_len);
	
	printk("Card Size = %dB (aprox. %dMiB)\n", card_size, card_size >> 20);

	DBG_WAIT;

	// select the given SD card before to send/receive data
	sdret = sd_send_command(7, (card_rca << 16) + 0x0000);

	printk("Select RCA %d -> %d\n", card_rca, sdret);

	uint32 blockbuf[128];
	sdret = sd_read_block(0, (char*)blockbuf);

	printk("Read return = %d\n{%p %p}\n", sdret, (void*)blockbuf[0], (void*)blockbuf[1]);

	DBG_WAIT;
}



void test_sleep_funcs() {
	// kdelay/kusleep tests
	printk("Trying kusleep(2000000) in infinite loop...\n");

	DBG_WAIT;

	printk("started!");

	int nbdelay = 0;
	while(1) {
		kusleep(2000000);
		nbdelay++;
		printk("%d done\n", nbdelay);
	}

	DBG_WAIT;
}



static unsigned char eepromarray[] = {0x11, 0x12, 0x13, 0x14, 0x69, 0x69, 0x80, 0x0B, 0x50, 0x00, 0x42, 0x88, 0x88, 0x88, 0x88};
	
void test_eeprom() {
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
}


/*
extern unsigned int * euser;
extern unsigned int * buser;
extern unsigned int * osram_buser;
extern unsigned int * usersize;

void test_process() {
	// test for user process load and run
	size_t processl = (void*)(&euser) - (void*)(&buser);
	void *bprocess = &osram_buser;
	void *eprocess = bprocess + processl;

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
}
*/

void test_process() {
	// test for user process load and run
	inode_t *elf_inode;
	struct file *elf_file;

	elf_inode = vfs_resolve("/mnt/smem/test.elf");
	if(elf_inode == NULL || (elf_file = vfs_open(elf_inode)) == NULL ) {
		printk("Not found /mnt/smem/test.elf\n");
	}
	else {
		process_t *proc1;
		proc1 = process_alloc();
		printk("loading test.elf\n");

		DBG_WAIT;

		elfloader_load(elf_file, proc1);
		process_set_asid(proc1);
		mmu_setasid(proc1->asid);
		printk("[D] ASID = %d\n", mmu_getasid());
		printk("asid=%d, pid=%d\n", proc1->asid, proc1->pid);

		DBG_WAIT;

		arch_kernel_contextjmp(&(proc1->acnt));

		DBG_WAIT;
	}
}


void test_vfs() {
	vfs_create("/dev", "mouahah", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00010000);
	vfs_create("/dev", "pouet", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00010000);
	vfs_create("/", "usr", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/", "fmem", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/usr", "chose1", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/usr/chose1", "bidule2", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/usr/chose1/bidule2", "holy_shit", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00010000);
	vfs_create("/dev", "stdin", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00010000);
	vfs_create("/dev", "sda", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00010000);

	inode_t *curi = vfs_resolve("/mnt/smem/UEDIT/acore.cfg");
	
	if(curi != NULL)
		printk("entry : '%s'\n    node=0x%x\n", curi->name, curi->node);
	else
		printk("VFS unfound file.\n");

	DBG_WAIT;

	inode_t *curi2 = vfs_resolve("/mnt/.././mnt/smem/../../mnt/smem/UEDIT/.././././UEDIT/acore.cfg");

	printk("with '..' and '.' : %s\n", curi2==curi ? "Ok" : "Fail");

	DBG_WAIT;

	// try to open a file in SMEM using vfs's functions
	struct file *filep;
	char file_buf[50];
	int nbread;

	filep = vfs_open(curi);

	nbread = vfs_read(filep, file_buf, 49);
	file_buf[nbread] = '\0';

	printk("Trying to read 49(%d) bytes :\n%s\n", nbread, file_buf);

	vfs_close(filep);

	printk("Done.\n");

	DBG_WAIT;

	// trying to open /dev/console device
	printk("Trying to use fx9860-terminal device...\n");

	inode_t *console = vfs_resolve("/dev/console");
	filep = vfs_open(console);

	vfs_write(filep, "This is a\nSIMPLE\ntest of TeRmInAl console as a DEVICE.\n", sizeof("This is a\nSIMPLE\ntest of TeRmInAl console as a DEVICE.\n")-1); 
	DBG_WAIT;

	vfs_close(filep);

	DBG_WAIT;

	ls_tree();

	DBG_WAIT;
}


void test_virtual_mem() {
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

	DBG_WAIT;

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


}


void test_keyboard_int() {
	INTERRUPT_PRIORITY_PINT0_7 = 0xF;
	INTERRUPT_PRIORITY_PINT8_15 = 0xF;

	interrupt_set_callback(INT_PINT_0_7, &test);
	interrupt_set_callback(INT_PINT_8_15, &test);
	

	PFC.PBCR.WORD = 0x5555;
	PB.DR.BYTE = 0b00000000;
	PFC.PACR.WORD = 0xAAAA;

	INTX.PINTER.WORD = 0b0000000011111111;

	while(1);
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
