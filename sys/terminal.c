#include "../display/T6K11/T6K11.h"
#include "../display/graphic.h"
#include "terminal.h"
#include <utils/strutils.h>

// constant variables definition
static int g_front_c = TCOLOR_BLACK;
static int g_back_c = TCOLOR_WHITE;
static int g_posx = 0;
static int g_posy = 0;
static vram_t g_terminal_vram = NULL;


// tmp
#include "keyboard/keyboard.h"

void terminal_write(const char *str) {
	int i;
	int maxx = termchar_width();
	int maxy = termchar_height();
	// TODO remove tmp stuff for blocking the terminal
	static int line_nb = 0;

	for(i=0; str[i]!='\0'; i++) {
		if(str[i] == '\n') {
			g_posx = 0;
			g_posy++;
			line_nb++;
		}
		else if(str[i] == '\r') g_posx=0;
		else {
			write_character(g_posx, g_posy, g_front_c, g_back_c, str[i], g_terminal_vram);
			g_posx++;
		}
		if(g_posx>=maxx) {
			g_posx = 0;
			g_posy++;
			line_nb++;
		}

		if(g_posy>=maxy) {
			// tmp stuff
			if(line_nb >= maxy-3) {
				int j;
				char * warn_str = "--- PRESS [EXE] TO SKIP ---";
				int warn_strlen = sizeof("--- PRESS [EXE] TO SKIP ---")-1;
				for(j=0; j<warn_strlen; j++)
					write_character(j, 0, TCOLOR_WHITE, TCOLOR_BLACK, warn_str[j], g_terminal_vram);
				copy_to_dd(g_terminal_vram);
				while(!is_key_down(K_EXE));
				while(is_key_down(K_EXE));
				static volatile int tricks;
				for(tricks = 0; tricks<100000; tricks++);
				line_nb = 0;
			}

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

