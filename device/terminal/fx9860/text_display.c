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
	.flush = &fx9860_tdisp_flush
};



void fx9860_tdisp_init_disp(struct tdisp_data *disp) {
	disp->back = 0; //TODO white
	disp->front = 1; //TODO black
	
	disp->vram = mem_pm_get_free_page(MEM_PM_CACHED);
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
		disp_mono_copy_to_dd(disp->vram);
	}
}


void fx9860_tdisp_flush(struct tdisp_data *disp) {
	disp_mono_copy_to_dd(disp->vram);
}
