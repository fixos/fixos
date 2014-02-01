#ifndef _DEVICE_KEYBOARD_FX9860_KEYMATRIX_H
#define _DEVICE_KEYBOARD_FX9860_KEYMATRIX_H

/**
 * Low-level keyboard handling functions for fx9860 and similar platforms.
 * The keyboard is represented as a 10*7 matrix (with many not used keys),
 * and the matrix is regularly updated by calling hwkbd_update_status().
 * Changes in key state is repported by callback functions, allowing the
 * higher layer to know when a key is pressed or released.
 */

#include <utils/types.h>

typedef void (*hwkbd_key_handler)(int keycode);


/**
 * Initialize low-level keyboard handling module.
 */
void hwkbd_init();

/**
 * Check each row/column of the keyboard, check for any differences with
 * previous call, and if any, call the callback function to signal each key
 * pressed (indicated as unpress when previous call happen, and now pressed)
 * or released.
 */
void hwkbd_update_status();


void hwkbd_set_kpressed_callback(hwkbd_key_handler handler);

void hwkbd_set_kreleased_callback(hwkbd_key_handler handler);

#endif //_DEVICE_KEYBOARD_FX9860_KEYMATRIX_H
