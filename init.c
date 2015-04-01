/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "arch/sh/7705_Casio.h"
#include "device/terminal/generic_early_term.h"
#include "arch/sh/interrupt.h"
#include "arch/sh/mmu.h"
#include "arch/sh/physical_memory.h"
#include "utils/log.h"
#include "sys/process.h"
#include "sys/scheduler.h"
#include "sys/time.h"
#include "sys/stimer.h"

#include "fs/casio_smemfs/file_system.h"
#include "fs/protofs/file_system.h"
#include "fs/vfs.h"
#include "fs/vfs_op.h"
#include "fs/vfs_file.h"

#include "device/device_registering.h"
#include "device/terminal/virtual_term.h"

#include "device/usb/usb_device_protocol.h"
#include "device/usb/cdc_acm/acm_device.h"
#include "device/usb/cdc_acm/cdc_acm.h"

#include "device/keyboard/fx9860/keyboard_device.h"
#include "device/keyboard/fx9860/keyboard.h"
#include "device/keyboard/fx9860/keymatrix.h"
#include "arch/sh/rtc.h"

#include "device/display/display.h"

#include "arch/sh/freq.h"

#include "tests.h"
#include "utils/strutils.h"

#include "sys/cmdline.h"
#include "sys/console.h"

#include "sys/sysctl.h"
#include "sys/mem_area.h"

#include "device/tty.h"

extern char cmdargs_begin;
extern char cmdargs_end;


volatile int _magic_lock = 0;

void print_usb_ep2(const char *str) {
	int i;

	for(i=0; str[i]!=0; i++);

	usb_send(USB_EP_ADDR_EP2IN/*_epdesc2.b_endpoint_addr*/, str, i);
}



// Real entry point of the OS :
void init() {
	unsigned int freq;

	interrupt_init();

	earlyterm_init();
	earlyterm_clear();

	kbd_init();
	rtc_init();
	time_init();

	earlyterm_write("Kernel initialization...\n");

	set_kernel_print(&earlyterm_write);
	printk(LOG_INFO, "cmd args: '%s'\n", &cmdargs_begin);

	cmdline_parse(&cmdargs_begin, 1024);

	mmu_init();
	pm_init_pages();

	stimer_init();
	hwkbd_start_periodic_update();

	DBG_WAIT;

	interrupt_inhibit_all(0);


	// console initialisation as soon as possible
	dev_init();
	// add TTY device (on major 4)
	ttydev_device.init();
	dev_register_device(&ttydev_device, 4);

	// add virtual terminal TTYs
	vt_init();

	// USB initialisation
	usb_init();
	// add usb-acm TTY
	acm_usb_init();

	DBG_WAIT;

	// will be the last message displayed on early console
	printk(LOG_INFO, "Switching screen to tty1...\n  The display will be cleared.\n");
	console_make_active();

	// in all cases, Virtual Terminals should be made active (tty1)
	DBG_WAIT;
	vt_set_active(0);


	// need to be changed for "overclocking" :
	//freq_change(FREQ_STC_4, FREQ_DIV_1, FREQ_DIV_4);
	
	freq_time_calibrate();

	freq = freq_get_internal_hz();
	printk(LOG_INFO, "CPU freq : %d.%dMHz\n", freq/1000000, (freq/100000)%10);

	freq = freq_get_peripheral_hz();
	printk(LOG_INFO, "Peripheral freq : %d.%dMHz\n", freq/1000000, (freq/100000)%10);

	// initialize sysctl tables
	ctl_init();

	//test_keyboard_int();

	//test_virtual_mem();

	//asm volatile ("trapa #50");

	//DBG_WAIT;
	

	// Initializing VFS and device sub-sytems, mount platform filesystems,
	// register platform devices...
	
	vfs_init();
	vfs_file_init();

	vfs_register_fs(&smemfs_file_system, VFS_REGISTER_STATIC);
	vfs_register_fs(&protofs_file_system, VFS_REGISTER_STATIC);
	vfs_mount("protofs", NULL, VFS_MOUNT_ROOT);

	vfs_create("/", "dev", INODE_TYPE_PARENT, INODE_FLAG_READ | INODE_FLAG_EXEC, 0);
	vfs_create("/dev", "tty1", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00040000);
	vfs_create("/dev", "tty2", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00040001);
	vfs_create("/dev", "serial", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00030000);

	vfs_create("/dev", "console", INODE_TYPE_DEV, INODE_FLAG_WRITE, 
			console_get_device());

	DBG_WAIT;

	// keyboard input for virtual terminals
	kbd_set_kstroke_handler(&vt_key_stroke);


	// mount additional filesystems
	vfs_create("/", "mnt", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);
	vfs_create("/mnt", "smem", INODE_TYPE_PARENT, INODE_FLAG_WRITE, 0);

	vfs_mount("smemfs", "/mnt/smem", VFS_MOUNT_NORMAL);
	
	DBG_WAIT;

	// set /dev/display device
	_display_device.init();
	dev_register_device(&_display_device, 0x20);
	vfs_create("/dev", "display", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00200001);


	// direct keyboard device on major 0x21
	fxkeyboard_device.init();
	dev_register_device(&fxkeyboard_device, 0x21);
	vfs_create("/dev", "keyboard", INODE_TYPE_DEV, INODE_FLAG_WRITE, 0x00210000);


	DBG_WAIT;

	//test_keymatrix();
//	test_keyboard();
	/*while(1) {
		char c;
		if(vfs_read(console, &c, 1) == 1) {
			vfs_write(console, &c, 1);
		}
	}*/

	DBG_WAIT;

	//test_vfs();

	//test_sdcard();

	//test_sleep_funcs();


	// EEPROM-related code commented to avoid useless write cycles ;)
	//test_eeprom();
	
	/*char mybuf[128];
	int len;

	len = usb_receive(USB_EP_ADDR_EP1OUT, mybuf, 10, 0);
	printk(LOG_DEBUG, "usb_receive ret=%d\n", len);
	if(len > 0) {
		mybuf[len] = '\0';
		printk(LOG_DEBUG, "content = '%s'\n", mybuf);
	}
	

	while(!_magic_lock);
	set_kernel_print(&print_usb_ep2);

	test_vfs();
*/

	// memory area subsystem
	mem_area_init();
	

	process_init();
	sched_init();
	test_process();
	

	printk(LOG_WARNING, "End of init job, sleeping...\n");
	while(1)
		printk(LOG_WARNING, "IER: 0x%x 0x%x\n", USB.IFR0.BYTE, USB.IFR1.BYTE);
}

