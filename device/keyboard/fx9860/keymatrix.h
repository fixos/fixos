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


/**
 * Start the periodic call of hwkbd_update_status() using a soft timer.
 */
void hwkbd_start_periodic_update();


void hwkbd_set_kpressed_callback(hwkbd_key_handler handler);

void hwkbd_set_kreleased_callback(hwkbd_key_handler handler);


/**
 * Get the state of a given key, without updating keyboard state.
 * Returns 1 if the key is pressed, 0 else.
 * Be careful when using this function in a non-interruptable code, the status
 * may never be updated...
 * code is a 'keymatrix' code defined in matrix_codes.h
 */
int hwkbd_keydown(int code);

/**
 * Get the 'real' state of a given key, not based on saved keyboard state.
 * This function do not call hwkbd_update_status(), so callback functions
 * will not be called.
 */
int hwkbd_real_keydown(int code);

#endif //_DEVICE_KEYBOARD_FX9860_KEYMATRIX_H
