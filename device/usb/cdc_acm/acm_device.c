#include "acm_device.h"
#include "cdc_acm.h"
#include <fs/file_operations.h>
#include <utils/strutils.h>
#include <interface/errno.h>


const struct device _acm_usb_device = {
	.name = "usb-acm",
	.init = acm_usb_init,
	.open = acm_usb_open
};


static const struct file_operations _fop_acm_usb = {
	.release = acm_usb_release,
	.write = acm_usb_write,
	.read = acm_usb_read,
	.ioctl = acm_usb_ioctl
};




void acm_usb_init() {
	cdc_acm_init();
}


int acm_usb_open(uint16 minor, struct file *filep) {
	if(minor == ACM_DEVICE_MINOR) {
		filep->op = &_fop_acm_usb;
		return 0;
	}

	return -ENXIO;
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
		return cdc_acm_receive(dest, len);
	}
	
	return -EIO;
}



int acm_usb_ioctl(struct file *filep, int cmd, void *data) {
	return -EINVAL;
}
