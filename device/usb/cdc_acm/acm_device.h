#ifndef _DEVICE_USB_CDC_ACM_ACM_DEVICE_H
#define _DEVICE_USB_CDC_ACM_ACM_DEVICE_H

/**
 * Device for USB communication using CDC/ACM class.
 * The only implemented MINOR is 0, which is the ACM 'pipe'.
 * TODO : support ioctls
 */

#include <device/device.h>
#include <fs/inode.h>
#include <fs/file.h>


#define ACM_DEVICE_MINOR	0

extern struct device _acm_usb_device;

void acm_usb_init();

struct file_operations* acm_usb_get_file_op(uint16 minor);

int acm_usb_open(inode_t *inode, struct file *filep);

int acm_usb_release(struct file *filep);

size_t acm_usb_write(struct file *filep, void *source, size_t len);

size_t acm_usb_read(struct file *filep, void *dest, size_t len);

int acm_usb_ioctl(struct file *filep, int cmd, void *data);

#endif //_DEVICE_USB_CDC_ACM_ACM_DEVICE_H
