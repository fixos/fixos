#ifndef _DEVICE_DISPLAY_DISPLAY_H
#define _DEVICE_DISPLAY_DISPLAY_H

/**
 * Display device, to allow direct screen access in userland.
 * For now this device only support monochrome screen, but it is designed
 * to be able to handle any display type in the future.
 *
 * The main way to control this device is through ioctl (see interface/display.h).
 */


#define DISPLAY_DEFAULT_MINOR	1

extern struct device _display_device;


void display_init();

struct file_operations* display_get_file_op(uint16 minor);

int display_open(inode_t *inode, struct file *filep);

int display_release(struct file *filep);

int display_ioctl(struct file *filep, int cmd, void *data);

#endif //_DEVICE_DISPLAY_DISPLAY_H
