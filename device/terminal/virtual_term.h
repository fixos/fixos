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

#ifndef _DEVICE_TERMINAL_VIRTUAL_TERM_H
#define _DEVICE_TERMINAL_VIRTUAL_TERM_H

#include <device/device.h>
#include <fs/file.h>

/**
 * Virtual terminal TTY, built on top of text_display interface.
 * This TTY allow to have one or more VT100-like terminals, each of them
 * using a private instance of text display and keyboard handling.
 *
 * Only one (or zero) virtual terminal can be enable at a time.
 * FIXME for now the keyboard handling use some fx9860-specific functions.
 */

// special characters for cursor
#define VT_CURSOR_CHAR			((char)177)

// first minor number reserver in TTY device, and number of terminals
#define VT_MINOR_BASE			0
#define VT_MAX_TERMINALS		2


// call this function to add the given character as keyboard input
void vt_key_stroke(int code);

/**
 * Try to make the given terminal active (eventualy deactivating the previous)
 * If term is -1, deactivate all terminals.
 */
void vt_set_active(int term);


/**
 * Initialize virtual terminal subsystem and add each corresponding TTY to
 * TTY device.
 */
void vt_init();


#endif //_DEVICE_TERMINAL_VIRTUAL_TERM_H
