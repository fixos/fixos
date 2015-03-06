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
