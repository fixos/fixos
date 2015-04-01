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

#include "acm_device.h"
#include "cdc_acm.h"
#include <fs/file_operations.h>
#include <utils/strutils.h>
#include <interface/fixos/errno.h>
#include <sys/tty.h>
#include <device/tty.h>


static int acm_tty_is_ready(struct tty *tty);

static int acm_tty_write(struct tty *tty, const char *data, size_t len);

static int acm_tty_putchar(struct tty *tty, char c);

static const struct tty_ops _acm_tty_ops = {
	.ioctl_setwinsize = NULL,
	.ioctl_getwinsize = NULL,
	.is_ready = &acm_tty_is_ready,
	.tty_write = &acm_tty_write,
	.putchar = &acm_tty_putchar
};

static struct tty _acm_tty;

// callback used to do TTY job for each input character
static int acm_input_char(char c) {
	tty_input_char(&_acm_tty, c);
	return 0;
}


void acm_usb_init() {
	tty_default_init(&_acm_tty);
	_acm_tty.ops = &_acm_tty_ops;
	_acm_tty.private = NULL;

	cdc_acm_init();
	cdc_acm_set_receive_callback(&acm_input_char);

	// add the CDC/ACM TTY to the TTY device
	ttydev_set_minor(ACM_USB_TTY_MINOR, &_acm_tty);
}


static int acm_tty_is_ready(struct tty *tty) {
	(void)tty;
	return cdc_acm_is_ready();
}


static int acm_tty_write(struct tty *tty, const char *data, size_t len) {
	(void)tty;
	if(cdc_acm_is_ready()) {
		return cdc_acm_send(data, len);
	}
	return -EIO;
}


static int acm_tty_putchar(struct tty *tty, char c) {
	acm_tty_write(tty, &c, 1);
	return 0;
}
