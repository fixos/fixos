#ifndef _DEVICE_TERMINAL_VIRTUAL_TERM_H
#define _DEVICE_TERMINAL_VIRTUAL_TERM_H

#include <device/device.h>
#include <fs/file.h>

/**
 * Virtual terminal device, built on top of text_display interface.
 * This device allow to control one or more VT100-like terminals, each of them
 * using a private instance of text display and keyboard handling.
 *
 * Only one (or zero) virtual terminal can be enable at a time.
 * Each virtual term is accessible using minor from 0 to (VT_MAX_TERMINALS - 1).
 * FIXME for now the keyboard handling use some fx9860-specific functions.
 */

// special characters for cursor
#define VT_CURSOR_CHAR			((char)177)

#define VT_MAX_TERMINALS		2

extern struct device virtual_term_device;


// call this function to add the given character as keyboard input
void vt_key_stroke(int code);

/**
 * Try to make the given terminal active (eventualy deactivating the previous)
 * If term is -1, deactivate all terminals.
 */
void vt_set_active(int term);


void vt_init();

int vt_open(uint16 minor, struct file *filep);

int vt_release(struct file *filep);

size_t vt_write(struct file *filep, void *source, size_t len);

size_t vt_read(struct file *filep, void *dest, size_t len);

int vt_ioctl(struct file *filep, int cmd, void *data);


#endif //_DEVICE_TERMINAL_VIRTUAL_TERM_H
