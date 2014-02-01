#include "arch/sh/7705_Casio.h"
#include "device/terminal/generic_early_term.h"
#include "arch/sh/interrupt.h"
#include "arch/sh/virtual_memory.h"
#include "arch/sh/mmu.h"
#include "arch/sh/physical_memory.h"
#include "utils/log.h"
#include "sys/process.h"
#include "sys/scheduler.h"

#include "fs/casio_smemfs/file_system.h"
#include "fs/protofs/file_system.h"
#include "fs/vfs.h"
#include "fs/vfs_op.h"
#include "fs/vfs_file.h"

#include "device/device_registering.h"
#include "device/terminal/fx9860/terminal.h"

#include "device/usb/usb_device_protocol.h"
#include "device/usb/cdc_acm/acm_device.h"
#include "device/usb/cdc_acm/cdc_acm.h"

#include "device/keyboard/fx9860/keyboard.h"
#include "device/keyboard/fx9860/keymatrix.h"
#include "arch/sh/rtc.h"

#include "tests.h"
#include "utils/strutils.h"

extern void * fixos_vbr;  // see fixos.ld


volatile int _magic_lock = 0;

void print_usb_ep2(const char *str) {
	int i;

	for(i=0; str[i]!=0; i++);

	usb_send(USB_EP_ADDR_EP2IN/*_epdesc2.b_endpoint_addr*/, str, i);
}




// Real entry point of the OS :
void init() {
	unsigned char vram[1024];

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
	INTERRUPT_PRIORITY_TMU3 = 0;
	INTERRUPT_PRIORITY_RTC = 0;
	INTERRUPT_PRIORITY_WDT = 0;

	INTERRUPT_PRIORITY_REF = 0;

	earlyterm_init(vram);
	earlyterm_clear();

	kbd_init();
	// TODO call hwkbd_update_status() in a better place
	rtc_set_interrupt(&hwkbd_update_status, RTC_PERIOD_64_HZ);

	set_kernel_print(&earlyterm_write);

	earlyterm_write("Boostrap... OK!\n");
	earlyterm_set_colors(EARLYTERM_COLOR_WHITE, EARLYTERM_COLOR_BLACK);
	earlyterm_write("WARNING : ");
	earlyterm_set_colors(EARLYTERM_COLOR_BLACK, EARLYTERM_COLOR_WHITE);
	earlyterm_write("it's working!\n  It's really strange...\n");

	mmu_init();
	pm_init_pages();

	DBG_WAIT;

	interrupt_inhibit_all(0);

	//test_keyboard_int();

	//test_virtual_mem();

	//asm volatile ("trapa #50");

	//DBG_WAIT;
	
	// Initializing VFS and device sub-sytems, mount platform filesystems,
	// register platform devices...
	
	vfs_init();
	vfs_file_init();

	// add fx9860 terminal in device list, with major number 20
	dev_init();
	_fx9860_term_device.init();
	dev_register_device(&_fx9860_term_device, 20);

	// add usb-acm device, major number 3
	// USB initialisation
	usb_init();
	_acm_usb_device.init();
	dev_register_device(&_acm_usb_device, 3);

	vfs_register_fs(&smemfs_file_system, VFS_REGISTER_STATIC);
	vfs_register_fs(&protofs_file_system, VFS_REGISTER_STATIC);
	vfs_mount("protofs", NULL, VFS_MOUNT_ROOT);

	vfs_create("/", "dev", INODE_TYPE_PARENT, INODE_FLAG_READ | INODE_FLAG_EXEC, 0);
	vfs_create("/dev", "console", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00140001);
	vfs_create("/dev", "serial", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00030000);

	DBG_WAIT;

	// switch from early_terminal to fx9860 console 
	printk("Trying to use fx9860-terminal device...\n");

	inode_t *console_inode = vfs_resolve("/dev/console");
	if(console_inode != NULL) {
		 struct file *console = vfs_open(console_inode);
		 if(console != NULL) {
			 printk("fx9860 terminal ready...\nThe display will be cleared.\n");
			 DBG_WAIT;

			 // vfs_write(filep, "Hello!\n", 7); 

			 // set printk() callback func
			 set_kernel_print_file(console);
			 printk("From fx9860 terminal : Hello!\nNow using /dev/console device!\n");
			 kbd_set_kstroke_handler(&fx9860_term_key_stroke);
		 }
		 else {
			 printk("[W] Unable to open fx9860 term\n");
		}
	}
	else {
		printk("[W] Not found node /dev/console\n");
	}


	// mount additional filesystems
	vfs_create("/", "mnt", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/mnt", "smem", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);

	vfs_mount("smemfs", "/mnt/smem", VFS_MOUNT_NORMAL);
	

	DBG_WAIT;

	test_time();

	DBG_WAIT;

	//test_keymatrix();
//	test_keyboard();
	rtc_set_interrupt(&hwkbd_update_status, RTC_PERIOD_64_HZ);
	while(1);

	DBG_WAIT;

	//test_vfs();

	//test_sdcard();

	//test_sleep_funcs();


	// EEPROM-related code commented to avoid useless write cycles ;)
	//test_eeprom();
	
	/*char mybuf[128];
	int len;

	len = usb_receive(USB_EP_ADDR_EP1OUT, mybuf, 10, 0);
	printk("usb_receive ret=%d\n", len);
	if(len > 0) {
		mybuf[len] = '\0';
		printk("content = '%s'\n", mybuf);
	}
	

	while(!_magic_lock);
	set_kernel_print(&print_usb_ep2);

	test_vfs();
*/

	process_init();
	sched_init();
	test_process();
	

	printk("End of init job, sleeping...\n");
	while(1);
		printk("IER: 0x%x 0x%x\n", USB.IFR0.BYTE, USB.IFR1.BYTE);
}

