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

#include <device/terminal/text_display.h>
#include <device/display/generic_mono.h>
#include "print_primitives.h"
#include "text_display.h"

#include <sys/memory.h>
#include <utils/strutils.h>

const struct text_display fx9860_text_display = {
	.cwidth = FX9860_TERM_WIDTH,
	.cheight = FX9860_TERM_HEIGHT,

	.init_disp = &fx9860_tdisp_init_disp,
	.print_char = &fx9860_tdisp_print_char,
	.scroll = &fx9860_tdisp_scroll,
	.set_color = &fx9860_tdisp_set_color,
	.set_active = &fx9860_tdisp_set_active,
	.flush = &fx9860_tdisp_flush,

	.set_cursor_pos = &fx9860_tdisp_set_cursor_pos,
	.set_cursor = &fx9860_tdisp_set_cursor,

	.clear = &fx9860_tdisp_clear
};



void fx9860_tdisp_init_disp(struct tdisp_data *disp) {
	disp->back = 0; //TODO white
	disp->front = 1; //TODO black

	disp->cursx = 0;
	disp->cursy = 0;
	disp->cursor = TEXT_CURSOR_NORMAL;
	
	disp->vram = arch_pm_get_free_page(MEM_PM_CACHED);
	memset(disp->vram, 0, 1024);
}


void fx9860_tdisp_print_char(struct tdisp_data *disp, size_t posx,
		size_t posy, char c)
{
	term_prim_write_character(posx, posy, disp->front, disp->back, c, disp->vram);
}


void fx9860_tdisp_scroll(struct tdisp_data *disp) {
	term_prim_scroll_up(disp->vram, disp->back);
}


void fx9860_tdisp_set_color(struct tdisp_data *disp, enum text_color front, 
		enum text_color back)
{
	// as we have only black and white, a filter is applied on the front color
	// (if front is dark, black on white is used, else it's white on black)
	(void)back;
	switch(front) {
		case TEXT_COLOR_BLACK:
		case TEXT_COLOR_BLUE:
		case TEXT_COLOR_GREEN:
		case TEXT_COLOR_MAGENTA:
			disp->back = 0;
			disp->front = 1;
			break;
		
		default:
			disp->back = 1;
			disp->front = 0;
			break;
	}
}



void fx9860_tdisp_set_active(struct tdisp_data *disp, int active) {
	// not a lot of stuff to do here, only update the display after activation
	if(active == 1) {
		fx9860_tdisp_flush(disp);
	}
}


void fx9860_tdisp_flush(struct tdisp_data *disp) {
	// here is the little trick for cursor display : we display it only before
	// flushing, and remove it juste before (so the VRAM is always "clean")
	int cursor_displayed = 0;

	if(disp->cursor != TEXT_CURSOR_DISABLE && disp->cursx < FX9860_TERM_WIDTH
			&& disp->cursy < FX9860_TERM_HEIGHT)
	{
		cursor_displayed = 1;
		term_prim_store_character(disp->cursx, disp->cursy, disp->vram);
		// TODO display other kind of cursors (alpha/shift...)
		term_prim_write_character(disp->cursx, disp->cursy,
				DISPLAY_COLOR_BLACK, DISPLAY_COLOR_WHITE,
				FX9860_TERM_CURSOR_CHAR, disp->vram);
	}

	disp_mono_copy_to_dd(disp->vram);
	
	if(cursor_displayed)
		term_prim_restore_character(disp->cursx, disp->cursy, disp->vram);
}


// cursor related functions are not very useful because of some tricks done
// to display it just before a display flush, but may be useful
void fx9860_tdisp_set_cursor_pos(struct tdisp_data *disp, size_t posx,
		size_t posy)
{
	disp->cursx = posx;
	disp->cursy = posy;
}

void fx9860_tdisp_set_cursor(struct tdisp_data *disp, enum text_cursor curs) {
	disp->cursor = curs;
}


void fx9860_tdisp_clear(struct tdisp_data *disp) {
	memset(disp->vram, disp->back ? 0xFF : 0x00, 1024);
}
