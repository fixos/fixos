#ifndef _DEVICE_TERMINAL_FX9860_TERM_H
#define _DEVICE_TERMINAL_FX9860_TERM_H

/**
 * Terminal device for fx9860-like calculators (T6K11 display driver and same
 * keyboard type).
 * Goal is to support VT100-like interface (WIP).
 * The MINOR used by this device are :
 * 1 : I/O terminal using keyboard as input and screen as dislay
 */

#include <device/device.h>
#include <fs/inode.h>
#include <fs/file.h>

#define FX9860_TERM_MINOR_TERMINAL	1


extern struct device _fx9860_term_device;


void fx9860_term_init();

// call this function to add the given character as keyboard input
void fx9860_term_key_stroke(int code);

struct file_operations* fx9860_term_get_file_op(uint16 minor);


int fx9860_term_open(inode_t *inode, struct file *filep);

int fx9860_term_release(struct file *filep);

size_t fx9860_term_write(struct file *filep, void *source, size_t len);

size_t fx9860_term_read(struct file *filep, void *dest, size_t len);

int fx9860_term_ioctl(struct file *filep, int cmd, void *data);

#endif //_DEVICE_TERMINAL_FX9860_TERM_H
