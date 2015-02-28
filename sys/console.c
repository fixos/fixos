#include "console.h"
#include <utils/log.h>
#include <device/device.h>
#include <device/device_registering.h>
#include <fs/file.h>
#include <utils/log.h>
#include <utils/strutils.h>
#include <sys/cmdline.h>
#include <sys/tty.h>


#define CONSOLE_USB_MAJOR		3
#define CONSOLE_USB_MINOR_BASE	0

#define CONSOLE_TTY_MAJOR		4
#define CONSOLE_TTY_MINOR_BASE	0


static uint32 _console_node = (CONSOLE_TTY_MAJOR << 16) | CONSOLE_TTY_MINOR_BASE;

int parse_console(const char *val) {
	//printk(LOG_DEBUG, "ARG: console='%s'\n", val);
	if(val != NULL) {
		if(val[0]=='t' && val[1]=='t' && val[2]=='y') {
			// check for ttyn
			if(val[3] >= '1' && val[3] <= '9') {
				_console_node = (CONSOLE_TTY_MAJOR << 16) | 
					(CONSOLE_TTY_MINOR_BASE + val[3] - '1');

				printk(LOG_INFO, "console: will use tty%d soon\n", val[3]-'0');
				return 0;
			}

			// check for USB0
			else if(!strcmp(val+3, "USB0")) {
				_console_node = (CONSOLE_USB_MAJOR << 16) | CONSOLE_USB_MINOR_BASE;
				printk(LOG_INFO, "console: will use USB soon\n");
				return 0;
			}
		}
	}
	printk(LOG_ERR, "malformed 'console' parameter\n");
	return -1;
}


KERNEL_BOOT_ARG(console, parse_console);



void console_make_active() {
	const struct device *console_dev;

	console_dev = dev_device_from_major(major(_console_node));
	if(console_dev != NULL) {
		struct tty *tty;

		tty = console_dev->get_tty(minor(_console_node));
		if(tty != NULL) {
			if(!tty_is_ready(tty)) {
				// wait until the TTY is ready
				printk(LOG_INFO, "[console tty not ready...]\n");
				while(!tty_is_ready(tty));
				printk(LOG_INFO, "[ready!]\n");
			}
			// set printk() TTY
			printk_set_console_tty(tty);
			printk(LOG_INFO, "Now using console device for printk()!\n");
		}
		else {
			printk(LOG_ERR, "console: unable to open minor %d!\n", minor(_console_node));
		}
	}
	else {
		printk(LOG_ERR, "console: unable to find device %d!\n", major(_console_node));
	}
}



uint32 console_get_device() {
	return _console_node;
}
