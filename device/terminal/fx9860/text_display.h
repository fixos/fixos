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
