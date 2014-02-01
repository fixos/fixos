#ifndef _DEVICE_KEYBOARD_FX9860_KEY_CODES_H
#define _DEVICE_KEYBOARD_FX9860_KEY_CODES_H

/**
 * Key code used by keyboard.h functions (high-level keyboard handling,
 * with modifiers like SHIFT/ALPHA).
 * This file contains all possible key, but each printable and not
 * calc-specific key is represented by it's ASCII value.
 * All other keys are higher than KEY__SPECIFIC value and are probably not to
 * be used in generic I/O functions...
 */

#define KEY__SPECIFIC	0x100

#define KEY_F1			0x101
#define KEY_F2			0x102
#define KEY_F3			0x103
#define KEY_F4			0x104
#define KEY_F5			0x105
#define KEY_F6			0x106

// SHIFT and ALPHA key stroke is not reported by default
#define KEY_SHIFT		0x201
#define KEY_ALPHA		0x202
#define KEY_ALPHALOCK	0x203

#define KEY_OPTN		0x301
//TODO TODO TODO
#define KEY_			0x1000

#endif //_DEVICE_KEYBOARD_FX9860_KEY_CODES_H
