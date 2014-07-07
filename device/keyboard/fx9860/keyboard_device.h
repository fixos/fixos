#ifndef _DEVICE_KEYBOARD_FX9860_KEYBOARD_DEVICE
#define _DEVICE_KEYBOARD_FX9860_KEYBOARD_DEVICE

#include <device/device.h>
#include <fs/file.h>
#include <interface/fxkeyboard.h>


// minor device ID for the direct keyboard access
#define FXKDB_DIRECT_MINOR		0


// maximum key event retained in the buffer
#define FXKDB_MAX_KEYS			20

extern const struct device fxkeyboard_device;


/**
 * This function should be called to insert a key event in the buffer.
 * Returns 0 if inserted, -1 if no space is available (event is ignored).
 */
int fxkdb_insert_event(const struct fxkey_event *event);

void fxkdb_init();

int fxkdb_open(uint16 minor, struct file *filep);

int fxkdb_release(struct file *filep);

size_t fxkdb_read(struct file *filep, void *dest, size_t len);


#endif //_DEVICE_KEYBOARD_FX9860_KEYBOARD_DEVICE
