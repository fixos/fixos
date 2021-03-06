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

#include "keyboard.h"
#include "keymatrix.h"
#include "key_codes.h"
#include "matrix_codes.h"

// to enable virtual term switching
#include <device/terminal/virtual_term.h>


static kbd_key_handler _kbd_key_stroke = NULL;

// maximum key pressed at a time
#define KBD_MAX_KEYS	6

struct kbd_keypressed {
	int code;
	// TODO add informations needed for repetition...
};

static struct kbd_keypressed _kbd_keys[KBD_MAX_KEYS];

#define KBD_STATE_SHIFT		0x01
#define KBD_STATE_ALPHA		0x02
#define KBD_STATE_ALPHALOCK	(0x04 | KBD_STATE_ALPHA) 


static int _kbd_status;

void kbd_init() {
	int i;

	hwkbd_init();
	
	for(i=0; i<KBD_MAX_KEYS; i++)
		_kbd_keys[i].code = -1;

	hwkbd_set_kpressed_callback(&kbd_kpressed_handler);
	hwkbd_set_kreleased_callback(&kbd_kreleased_handler);
}





int kdb_convert_keymatrix_code(int matrixcode) {

	if(_kbd_status & KBD_STATE_ALPHA) {
		switch (matrixcode) {
			case K_F1:	return KEY_F1; break;
			case K_F2: return KEY_F2; break;
			case K_F3: return KEY_; break;
			case K_F4: return KEY_; break;
			case K_F5: return KEY_; break;
			case K_F6: return KEY_; break;

			case K_SHIFT: return KEY_; break;
			case K_OPTN: return ASCII_CTRL('Z'); break;
			case K_VARS: return KEY_; break;
			case K_MENU: return KEY_; break;
			case K_LEFT: return KEY_; break;
			case K_UP: return KEY_; break;

			case K_ALPHA: return KEY_; break;
			case K_SQR: return KEY_; break;
			case K_EXPO: return KEY_; break;
			case K_EXIT: return ASCII_CTRL('C'); break;
			case K_DOWN: return KEY_; break;
			case K_RIGHT: return KEY_; break;

			case K_THETA: return 'A'; break;
			case K_LOG: return 'B'; break;
			case K_LN: return 'C'; break;
			case K_SIN: return 'D'; break;
			case K_COS: return 'E'; break;
			case K_TAN: return 'F'; break;

			case K_FRAC: return 'G'; break;
			case K_FD: return 'H'; break;
			case K_LPAR: return 'I'; break;
			case K_RPAR: return 'J'; break;
			case K_COMMA: return 'K'; break;
			case K_STORE: return 'L'; break;

			case K_7: return 'M'; break;
			case K_8: return 'N'; break;
			case K_9: return 'O'; break;
			case K_DEL: return '\x08'; break;

			case K_4: return 'P'; break;
			case K_5: return 'Q'; break;
			case K_6: return 'R'; break;
			case K_MULT: return 'S'; break;
			case K_DIV: return 'T'; break;

			case K_1: return 'U'; break;
			case K_2: return 'V'; break;
			case K_3: return 'W'; break;
			case K_PLUS: return 'X'; break;
			case K_MINUS: return 'Y'; break;

			case K_0: return 'Z'; break;
			case K_DOT: return ' '; break;
			case K_EXP: return '"'; break;
			case K_NEG: return KEY_; break;
			case K_EXE: return '\n'; break;

			case K_AC: return ASCII_CTRL('U'); break;
		}
	}
	else if(_kbd_status & KBD_STATE_SHIFT) {
		switch (matrixcode) {
			case K_F1:	return KEY_F1; break;
			case K_F2: return KEY_F2; break;
			case K_F3: return KEY_; break;
			case K_F4: return KEY_; break;
			case K_F5: return KEY_; break;
			case K_F6: return KEY_; break;

			case K_SHIFT: return KEY_; break;
			case K_OPTN: return ASCII_CTRL('Z'); break;
			case K_VARS: return KEY_; break;
			case K_MENU: return KEY_; break;
			case K_LEFT: return KEY_; break;
			case K_UP: return KEY_; break;

			case K_ALPHA: return KEY_; break;
			case K_SQR: return KEY_; break;
			case K_EXPO: return KEY_; break;
			case K_EXIT: return ASCII_CTRL('C'); break;
			case K_DOWN: return KEY_; break;
			case K_RIGHT: return KEY_; break;

			case K_THETA: return KEY_; break;
			case K_LOG: return KEY_; break;
			case K_LN: return KEY_; break;
			case K_SIN: return KEY_; break;
			case K_COS: return KEY_; break;
			case K_TAN: return KEY_; break;

			case K_FRAC: return KEY_; break;
			case K_FD: return KEY_; break;
			case K_LPAR: return KEY_; break;
			case K_RPAR: return KEY_; break;
			case K_COMMA: return KEY_; break;
			case K_STORE: return KEY_; break;

			case K_7: return KEY_; break;
			case K_8: return KEY_; break;
			case K_9: return KEY_; break;
			case K_DEL: return '\x08'; break;

			case K_4: return KEY_; break;
			case K_5: return KEY_; break;
			case K_6: return KEY_; break;
			case K_MULT: return '{'; break;
			case K_DIV: return '}'; break;

			case K_1: return KEY_; break;
			case K_2: return KEY_; break;
			case K_3: return KEY_; break;
			case K_PLUS: return '['; break;
			case K_MINUS: return ']'; break;

			case K_0: return KEY_; break;
			case K_DOT: return KEY_; break;
			case K_EXP: return KEY_; break;
			case K_NEG: return KEY_; break;
			case K_EXE: return '\n'; break;

			case K_AC: return ASCII_CTRL('U'); break;
		}
	}
	else {
		switch (matrixcode) {
			case K_F1:	return KEY_F1; break;
			case K_F2: return KEY_F2; break;
			case K_F3: return KEY_; break;
			case K_F4: return KEY_; break;
			case K_F5: return KEY_; break;
			case K_F6: return KEY_; break;

			case K_SHIFT: return KEY_; break;
			case K_OPTN: return ASCII_CTRL('Z'); break;
			case K_VARS: return ASCII_CTRL('A'); break;
			case K_MENU: return ASCII_CTRL('['); break;
			case K_LEFT: return KEY_; break;
			case K_UP: return KEY_; break;

			case K_ALPHA: return KEY_; break;
			case K_SQR: return KEY_; break;
			case K_EXPO: return '^'; break;
			case K_EXIT: return ASCII_CTRL('C'); break;
			case K_DOWN: return KEY_; break;
			case K_RIGHT: return KEY_; break;

			case K_THETA: return KEY_; break;
			case K_LOG: return KEY_; break;
			case K_LN: return KEY_; break;
			case K_SIN: return KEY_; break;
			case K_COS: return KEY_; break;
			case K_TAN: return KEY_; break;

			case K_FRAC: return KEY_; break;
			case K_FD: return KEY_; break;
			case K_LPAR: return '('; break;
			case K_RPAR: return ')'; break;
			case K_COMMA: return ','; break;
			case K_STORE: return KEY_; break;

			case K_7: return '7'; break;
			case K_8: return '8'; break;
			case K_9: return '9'; break;
			case K_DEL: return '\x08'; break;

			case K_4: return '4'; break;
			case K_5: return '5'; break;
			case K_6: return '6'; break;
			case K_MULT: return '*'; break;
			case K_DIV: return '/'; break;

			case K_1: return '1'; break;
			case K_2: return '2'; break;
			case K_3: return '3'; break;
			case K_PLUS: return '+'; break;
			case K_MINUS: return '-'; break;

			case K_0: return '0'; break;
			case K_DOT: return '.'; break;
			case K_EXP: return KEY_; break;
			case K_NEG: return '_'; break;
			case K_EXE: return '\n'; break;

			case K_AC: return ASCII_CTRL('U'); break;
		}
	}
	return -1;
}




void kbd_kpressed_handler(int code) {
	// first, test if the key is a status modifier or a 'normal' key
	if(code == K_SHIFT || code == K_ALPHA) {
		// change the status and return
		if(code == K_SHIFT) {
			if(_kbd_status & KBD_STATE_SHIFT)
				_kbd_status &= ~KBD_STATE_SHIFT;
			else
				_kbd_status |= KBD_STATE_SHIFT;
		}
		else {
			if(_kbd_status & KBD_STATE_ALPHA) {
				_kbd_status &= ~KBD_STATE_ALPHALOCK;
			}
			else {
				if(_kbd_status & KBD_STATE_SHIFT)
					_kbd_status |= KBD_STATE_ALPHALOCK;
				else
					_kbd_status |= KBD_STATE_ALPHA;
			}
		}
	}
	else {
		int i;
		struct kbd_keypressed *kfree = NULL;
		int converted;

		// check in the list of key pressed if this one exists, else add it
		for(i=0; i<KBD_MAX_KEYS; i++) {
			if(_kbd_keys[i].code == code) {
				i = KBD_MAX_KEYS; // exit the loop
			}
			else if(kfree == NULL && _kbd_keys[i].code == -1) {
				kfree = &(_kbd_keys[i]);
			}
		}

		// kfree is NULL if all keys are used or if the key code is already
		// in the list...
		if(kfree != NULL) {
			kfree->code = code;
		}


		converted = kdb_convert_keymatrix_code(code);
		// low-level key events : switch virtual terminals
		if(converted >= KEY_F1 && converted <= KEY_F6) {
			vt_set_active(converted - KEY_F1);
		}
		else if(_kbd_key_stroke != NULL) {
			_kbd_key_stroke(converted);
		}

		// clear SHIFT/ALPHA flags
		if((_kbd_status & KBD_STATE_ALPHALOCK) == KBD_STATE_ALPHALOCK)
			_kbd_status &= ~KBD_STATE_SHIFT;
		else
			_kbd_status &= ~(KBD_STATE_ALPHA | KBD_STATE_SHIFT);
	}
}



void kbd_kreleased_handler(int code) {
	int i;

	// look for key pressed, and clear the first entry with the given code
	for(i=0; i<KBD_MAX_KEYS; i++) {
		if(_kbd_keys[i].code == code) {
			_kbd_keys[i].code = -1;
		}
	}
}



void kbd_set_kstroke_handler(kbd_key_handler handler) {
	_kbd_key_stroke = handler;
}

