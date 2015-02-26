#include "acm_device.h"
#include "cdc_acm.h"
#include <fs/file_operations.h>
#include <utils/strutils.h>
#include <interface/fixos/errno.h>
#include <sys/tty.h>


static struct tty *acm_get_tty(uint16 minor);

const struct device _acm_usb_device = {
	.name = "usb-acm",
	.init = acm_usb_init,
	.open = acm_usb_open,
	.get_tty = acm_get_tty
};


static const struct file_operations _fop_acm_usb = {
	.release = acm_usb_release,
	.write = acm_usb_write,
	.read = acm_usb_read,
	.ioctl = acm_usb_ioctl
};


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
}


int acm_usb_open(uint16 minor, struct file *filep) {
	if(minor == ACM_DEVICE_MINOR) {
		filep->op = &_fop_acm_usb;
		return 0;
	}

	return -ENXIO;
}


static struct tty *acm_get_tty(uint16 minor) {
	if(minor == ACM_DEVICE_MINOR) {
		return &_acm_tty;
	}
	return NULL;
}


int acm_usb_release(struct file *filep) {
	return 0;
}



ssize_t acm_usb_write(struct file *filep, void *source, size_t len) {
	if(cdc_acm_is_ready()) {
		return cdc_acm_send(source, len);
	}
	
	return -EIO;
}



ssize_t acm_usb_read(struct file *filep, void *dest, size_t len) {
	if(cdc_acm_is_ready()) {
		return tty_read(&_acm_tty, dest, len);
		//return cdc_acm_receive(dest, len);
	}
	
	return -EIO;
}



int acm_usb_ioctl(struct file *filep, int cmd, void *data) {
	int ret;
	ret = tty_ioctl(&_acm_tty, cmd, data);
	if(ret == -EFAULT) {
		// device-level ioctl command, if any
		return -EINVAL;
	}
	else {
		return ret;
	}
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
