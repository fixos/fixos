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

#ifndef _DEVICE_TERMINAL_GENERIC_EARLY_TERM_H
#define _DEVICE_TERMINAL_GENERIC_EARLY_TERM_H

/**
 * Interface and definitions for the early terminal, used to print informations
 * during first boot steps.
 * The interface need to be implemented by platform-specific files.
 * TODO make early term implementation generic, on the top of text_display interface!
 *
 * The goal of these functions is to allow to use display before initialization
 * of high-level part of the kernel, such as device registering and VFS.
 *
 * Position is incremented automaticaly, and "scroll" must be done internaly
 * if a new line begin at end of screen.
 */


/**
 * Colors used by the terminal, implementation must choose the closest available
 * color if the given one isn't.
 */
#define EARLYTERM_COLOR_WHITE	0
#define EARLYTERM_COLOR_BLACK	1


/**
 * Initialize the early terminal.
 */
void earlyterm_init();


/**
 * Print the given string into the terminal.
 */
void earlyterm_write(const char *str);

/**
 * Change the terminal character colors.
 */
void earlyterm_set_colors(int front_c, int back_c);

/**
 * Clear the terminal (and put the cursor in (1,1) )
 * WARNING : the terminal is filled with the back color used
 */
void earlyterm_clear();

/**
 * Move the cursor at the given position, (1,1) is top-left corner.
 */
void earlyterm_set_pos(int posx, int posy);

/**
 * Get current position.
 */
int earlyterm_posx();
int earlyterm_posy();

#endif //_DEVICE_TERMINAL_GENERIC_EARLY_TERM_H
