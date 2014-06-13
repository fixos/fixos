#include "acm_device.h"
#include "cdc_acm.h"
#include <fs/file_operations.h>
#include <utils/strutils.h>


struct device _acm_usb_device = {
	.name = "usb-acm",
	.init = acm_usb_init,
	.get_file_op = acm_usb_get_file_op
};


static struct file_operations _fop_acm_usb = {
	.open = acm_usb_open,
	.release = acm_usb_release,
	.write = acm_usb_write,
	.read = acm_usb_read,
	.ioctl = acm_usb_ioctl
};




void acm_usb_init() {
	cdc_acm_init();
}


struct file_operations* acm_usb_get_file_op(uint16 minor) {
	if(minor == ACM_DEVICE_MINOR)
		return &_fop_acm_usb;

	return NULL;
}


int acm_usb_open(inode_t *inode, struct file *filep) {
	if(inode->typespec.dev.minor == ACM_DEVICE_MINOR) {
		filep->op = &_fop_acm_usb;
		return 0;
	}

	return -1;
}



int acm_usb_release(struct file *filep) {
	return 0;
}



size_t acm_usb_write(struct file *filep, void *source, size_t len) {
	if(cdc_acm_is_ready()) {
		return cdc_acm_send(source, len);
	}
	
	return 0; //-1; -> ssize_t!
}



size_t acm_usb_read(struct file *filep, void *dest, size_t len) {
	if(cdc_acm_is_ready()) {
		return cdc_acm_receive(dest, len);
	}
	
	return 0; //-1; -> ssize_t!
}



int acm_usb_ioctl(struct file *filep, int cmd, void *data) {
	return -1;
}
