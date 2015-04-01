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

#include "console.h"
#include <utils/log.h>
#include <device/device.h>
#include <device/device_registering.h>
#include <fs/file.h>
#include <utils/log.h>
#include <utils/strutils.h>
#include <sys/cmdline.h>
#include <sys/tty.h>

#include <device/tty.h>
#include <device/terminal/virtual_term.h>
#include <device/usb/cdc_acm/acm_device.h>

#define CONSOLE_TTY_MAJOR		4
#define CONSOLE_VT_MINOR_BASE	VT_MINOR_BASE
#define CONSOLE_USB_MINOR_BASE	ACM_USB_TTY_MINOR


static uint32 _console_node = makedev(CONSOLE_TTY_MAJOR,
		CONSOLE_VT_MINOR_BASE);

int parse_console(const char *val) {
	//printk(LOG_DEBUG, "ARG: console='%s'\n", val);
	if(val != NULL) {
		if(val[0]=='t' && val[1]=='t' && val[2]=='y') {
			// check for ttyn
			if(val[3] >= '1' && val[3] <= '9') {
				uint16 minorvt = val[3] - '1' + CONSOLE_VT_MINOR_BASE;

				if(minorvt < CONSOLE_VT_MINOR_BASE + VT_MAX_TERMINALS) {
					_console_node = makedev(CONSOLE_TTY_MAJOR, minorvt);
					printk(LOG_INFO, "console: will use tty%d soon\n",
							val[3]-'0');
					return 0;
				}
			}

			// check for USB0
			else if(!strcmp(val+3, "USB0")) {
				_console_node = makedev(CONSOLE_TTY_MAJOR, 
						CONSOLE_USB_MINOR_BASE);
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

		tty = ttydev_get_minor(minor(_console_node));
		if(tty != NULL) {
			tty_open(tty);
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
