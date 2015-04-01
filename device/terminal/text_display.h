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

#ifndef _DEVICE_TERMINAL_TEXT_DISPLAY_H
#define _DEVICE_TERMINAL_TEXT_DISPLAY_H

#include <utils/types.h>

// implementation-dependant structure
struct tdisp_data;

// possible colors, based on VT100
enum text_color {
	TEXT_COLOR_BLACK = 0,
	TEXT_COLOR_RED,
	TEXT_COLOR_GREEN,
	TEXT_COLOR_YELLOW,
	TEXT_COLOR_BLUE,
	TEXT_COLOR_MAGENTA,
	TEXT_COLOR_CYAN,
	TEXT_COLOR_WHITE
};

// possible kind of cursor (will be extended to display shift/alpha modes)
enum text_cursor {
	TEXT_CURSOR_DISABLE,
	TEXT_CURSOR_NORMAL
};

struct text_display {
	size_t cwidth;
	size_t cheight;
	
	/**
	 * Used to initialize an instance of the display (tdisp_data).
	 * Should be called before any other functions.
	 */
	void (*init_disp)(struct tdisp_data *disp);

	/**
	 * Print a character with current back/front colors at (posx ; posy) on the
	 * given display instance.
	 */
	void (*print_char)(struct tdisp_data *disp, size_t posx, size_t posy, char c);

	/**
	 * Scroll up one character line from the display.
	 */
	void (*scroll)(struct tdisp_data *disp);

	/**
	 * Change colors used for the subsequent call to print_char().
	 * If the implementation can't display all the possible colors, it can
	 * freely use appropriate ones.
	 */
	void (*set_color)(struct tdisp_data *disp, enum text_color front, 
			enum text_color back);

	/**
	 * Set the display status (active is 1 if called to activate it, 0 to
	 * deactivate it).
	 * After initialization, the display should be inactive.
	 */
	void (*set_active)(struct tdisp_data *disp, int active);


	/**
	 * Flush the display (any changes done before this call may be invisible,
	 * depending the implementation).
	 * Should be call each time we need to be sure the display is up to date.
	 */
	void (*flush)(struct tdisp_data *disp);

	/**
	 * Set cursor position, if supported.
	 * Cursor in text_display interface is only a visual hint, it does nothing
	 * like controlling where characters are displayed!
	 */
	void (*set_cursor_pos)(struct tdisp_data *disp, size_t posx, size_t posy);

	/**
	 * Set cursor type, if supported.
	 */
	void (*set_cursor)(struct tdisp_data *disp, enum text_cursor curs);


	/**
	 * Clear the whole screen (using the current background color).
	 * TODO should be more generic, to allow erasing of a given part
	 * of the screen.
	 */
	void (*clear)(struct tdisp_data *disp);
};


#endif //_DEVICE_TERMINAL_TEXT_DISPLAY_H
