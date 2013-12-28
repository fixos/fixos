#include <device/display/generic_mono.h>
#include <utils/strutils.h>
#include "print_primitives.h"
#include <device/terminal/generic_early_term.h>


/**
 * Implementation of early terminal (interface in device/terminal/early_terminal.h)
 * for fx9860-like platform.
 */

// constant variables definition
static int g_front_c = EARLYTERM_COLOR_BLACK;
static int g_back_c = EARLYTERM_COLOR_WHITE;
static int g_posx = 0;
static int g_posy = 0;
static void *g_terminal_vram = NULL;


// tmp
#include <device/keyboard/keyboard.h>

void earlyterm_write(const char *str) {
	int i;
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
			term_prim_write_character(g_posx, g_posy, g_front_c, g_back_c, str[i], g_terminal_vram);
			g_posx++;
		}
		if(g_posx>=FX9860_TERM_WIDTH) {
			g_posx = 0;
			g_posy++;
			line_nb++;
		}

		if(g_posy>=FX9860_TERM_HEIGHT) {
			// tmp stuff
			if(line_nb >= FX9860_TERM_HEIGHT-3) {
				int j;
				char * warn_str = "--- PRESS [EXE] TO SKIP ---";
				int warn_strlen = sizeof("--- PRESS [EXE] TO SKIP ---")-1;
				for(j=0; j<warn_strlen; j++)
					term_prim_write_character(j, 0, EARLYTERM_COLOR_WHITE, EARLYTERM_COLOR_BLACK, warn_str[j], g_terminal_vram);
				disp_mono_copy_to_dd(g_terminal_vram);
				while(!is_key_down(K_EXE));
				while(is_key_down(K_EXE));
				static volatile int tricks;
				for(tricks = 0; tricks<100000; tricks++);
				line_nb = 0;
			}

			term_prim_scroll_up(g_terminal_vram, g_back_c);
			g_posy = FX9860_TERM_HEIGHT-1;
		}
	}

	disp_mono_copy_to_dd(g_terminal_vram);
}


void earlyterm_set_colors(int front_c, int back_c) {
	g_back_c = back_c;
	g_front_c = front_c;
}


void earlyterm_clear() {
	// for now, direct VRAM access, only compatible with fx-9860 like models!
	// TODO a better system of course!
	unsigned int back = term_prim_mask_color(g_back_c);
	int i;
	unsigned int *l_vram = (unsigned int *)g_terminal_vram;
	for(i=0; i<4*64; i++) {
		l_vram[i] = back;
	}
	g_posx = 0;
	g_posy = 0;
	disp_mono_copy_to_dd(g_terminal_vram);
}


void earlyterm_set_pos(int posx, int posy) {
	g_posx = posx;
	g_posy = posy;
}


int earlyterm_posx() {
	return g_posx;
}

int earlyterm_posy() {
	return g_posy;
}



void earlyterm_init(void *vram) {
	g_terminal_vram = vram;
}


