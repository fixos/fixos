#include "console.h"
#include <utils/log.h>
#include <device/device.h>
#include <device/device_registering.h>
#include <fs/file.h>
#include <utils/log.h>
#include <utils/strutils.h>
#include <sys/cmdline.h>


#define CONSOLE_USB_MAJOR		3
#define CONSOLE_USB_MINOR_BASE	0

#define CONSOLE_TTY_MAJOR		4
#define CONSOLE_TTY_MINOR_BASE	0


static uint32 _console_node = (CONSOLE_TTY_MAJOR << 16) | CONSOLE_TTY_MINOR_BASE;

// static file used for the console
static struct file _console_file;

int parse_console(const char *val) {
	//printk("ARG: console='%s'\n", val);
	if(val != NULL) {
		if(val[0]=='t' && val[1]=='t' && val[2]=='y') {
			// check for ttyn
			if(val[3] >= '1' && val[3] <= '9') {
				_console_node = (CONSOLE_TTY_MAJOR << 16) | 
					(CONSOLE_TTY_MINOR_BASE + val[3] - '1');

				printk("console: will use tty%d soon\n", val[3]-'0');
				return 0;
			}

			// check for USB0
			else if(!strcmp(val+3, "USB0")) {
				_console_node = (CONSOLE_USB_MAJOR << 16) | CONSOLE_USB_MINOR_BASE;
				printk("console: will use USB soon\n");
				return 0;
			}
		}
	}
	printk("malformed 'console' parameter\n");
	return -1;
}


KERNEL_BOOT_ARG(console, parse_console);



void console_make_active() {
	const struct device *console_dev;

	console_dev = dev_device_from_major((uint16)(_console_node >> 16));
	if(console_dev != NULL) {
		_console_file.flags = 0;
		_console_file.inode = NULL;
		if(console_dev->open((uint16)(_console_node & 0xFFFF), &_console_file) == 0) {
			// set printk() callback func
			set_kernel_print_file(&_console_file);
			printk("Now using console device for printk()!\n");
		}
		else {
			printk("console: unable to open minor %d!\n", _console_node & 0xFFFF);
		}
	}
	else {
		printk("console: unable to find device %d!\n", _console_node >> 16);
	}
}



uint32 console_get_device() {
	return _console_node;
}
