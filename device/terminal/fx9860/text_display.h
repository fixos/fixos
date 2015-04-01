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

#ifndef _DEVICE_TERMINAL_FX9860_TEXT_DISPLAY_H
#define _DEVICE_TERMINAL_FX9860_TEXT_DISPLAY_H

#include <device/terminal/text_display.h>

struct tdisp_data {
	int front;
	int back;

	// data to handle the visual cursor
	size_t cursx;
	size_t cursy;
	enum text_cursor cursor;

	unsigned char *vram;
};

extern const struct text_display fx9860_text_display;


void fx9860_tdisp_init_disp(struct tdisp_data *disp);

void fx9860_tdisp_print_char(struct tdisp_data *disp, size_t posx,
		size_t posy, char c);

void fx9860_tdisp_scroll(struct tdisp_data *disp);

void fx9860_tdisp_set_color(struct tdisp_data *disp, enum text_color front, 
		enum text_color back);

void fx9860_tdisp_set_active(struct tdisp_data *disp, int active);

void fx9860_tdisp_flush(struct tdisp_data *disp);

void fx9860_tdisp_set_cursor_pos(struct tdisp_data *disp, size_t posx,
		size_t posy);

void fx9860_tdisp_set_cursor(struct tdisp_data *disp, enum text_cursor curs);

void fx9860_tdisp_clear(struct tdisp_data *disp);

#endif //_DEVICE_TERMINAL_FX9860_TEXT_DISPLAY_H
