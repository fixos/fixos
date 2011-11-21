#include "../display/T6K11/T6K11.h"
#include "../display/graphic.h"
#include "terminal.h"
#include <string.h>

// constant variables definition
static int g_front_c = TCOLOR_BLACK;
static int g_back_c = TCOLOR_WHITE;
static int g_posx = 0;
static int g_posy = 0;
static vram_t g_terminal_vram = NULL;


void terminal_write(const char *str) {
	int i;
	int maxx = termchar_width();
	int maxy = termchar_height();

	for(i=0; str[i]!='\0'; i++) {
		if(str[i] == '\n') {
			g_posx = 0;
			g_posy++;
		}
		else if(str[i] == '\r') g_posx=0;
		else {
			write_character(g_posx, g_posy, g_front_c, g_back_c, str[i], g_terminal_vram);
			g_posx++;
		}
		if(g_posx>=maxx) {
			g_posx = 0;
			g_posy++;
		}
		if(g_posy>=maxy) {
			scroll_up(g_terminal_vram, g_back_c);
			g_posy = maxy-1;
		}
	}

	copy_to_dd(g_terminal_vram);
}


void terminal_set_colors(int front_c, int back_c) {
	g_back_c = back_c;
	g_front_c = front_c;
}


void terminal_clear() {
	// for now, direct VRAM access, only compatible with fx-9860 like models!
	// TODO a better system of course!
	unsigned int back = mask_color(g_back_c);
	int i;
	unsigned int *l_vram = (unsigned int *)g_terminal_vram;
	for(i=0; i<4*64; i++) {
		l_vram[i] = back;
	}
	g_posx = 0;
	g_posy = 0;
	copy_to_dd(g_terminal_vram);
}


void terminal_set_pos(int posx, int posy) {
	g_posx = posx;
	g_posy = posy;
}


int terminal_posx() {
	return g_posx;
}

int terminal_posy() {
	return g_posy;
}



void terminal_set_vram(vram_t vram) {
	g_terminal_vram = vram;
}


