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

#ifndef _DEVICE_TERMINAL_FX9860_PRINT_PRIMITIVES_H
#define _DEVICE_TERMINAL_FX9860_PRINT_PRIMITIVES_H


#define FX9860_TERM_WIDTH	32
#define FX9860_TERM_HEIGHT	10


#define FX9860_TERM_CURSOR_CHAR	((char)177)


/**
 * write a character with the terminal font
 */
void term_prim_write_character(unsigned int posx, unsigned int posy,
		int front_c, int back_c, char c, void *vram);


/**
 * Special set of functions designed for cursor implementation.
 * Save/restore the VRAM content of a character location (saved internally
 * in a single slot, so each call to term_prim_store_character() destroy the
 * previously stored data).
 */
void term_prim_store_character(unsigned int posx, unsigned int posy,
		void *vram);
void term_prim_restore_character(unsigned int posx, unsigned int posy,
		void *vram);

/**
 * Scroll up the terminal display from 1 character high
 */
void term_prim_scroll_up(void *vram, int back_c);


unsigned int term_prim_mask_color(int color);

#endif //_DEVICE_TERMINAL_FX9860_PRINT_PRIMITIVES_H
