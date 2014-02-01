#include "terminal.h"
#include <fs/file_operations.h>
#include <device/display/generic_mono.h>
#include <device/keyboard/fx9860/keyboard.h>
#include <utils/strutils.h>
#include "print_primitives.h"

// temp stuff for blocking screen
#include <device/keyboard/fx9860/keymatrix.h>
#include <device/keyboard/fx9860/matrix_codes.h>


// temp before kmalloc() implementation
static unsigned char _term_vram[1024];
static int _term_posx;
static int _term_posy;

static int _term_back_c;
static int _term_front_c;

// for input data
#define FX9860_TERM_INPUT_BUFFER	128
static unsigned char _term_inbuf[FX9860_TERM_INPUT_BUFFER];
static int _term_inbuf_pos;


#define TERM9860_COLOR_WHITE	0
#define TERM9860_COLOR_BLACK	1


struct device _fx9860_term_device = {
	.name = "fx-term",
	.init = fx9860_term_init,
	.get_file_op = fx9860_term_get_file_op
};


static struct file_operations _fop_screen = {
	.open = fx9860_term_open,
	.release = fx9860_term_release,
	.write = fx9860_term_write,
	.ioctl = fx9860_term_ioctl
};


// function used to print characters
static void fx9860_term_print(void *source, size_t len) {
	// write on display_mono interface, using print_primitives.h for writing character
	// for now, implementation is close to the fx9860 early term, but the goal is to
	// allow future extensions to support VT100-like escape codes
	
	int i;
	unsigned char *str = source;
	// TODO remove tmp stuff for blocking the terminal
	static int line_nb = 0;

	for(i=0; i<len; i++) {
		if(str[i] == '\n') {
			// remove the current cursor display before line feed
			term_prim_write_character(_term_posx, _term_posy, _term_front_c,
					_term_back_c, ' ', _term_vram);

			_term_posx = 0;
			_term_posy++;
			line_nb++;
		}
		else if(str[i] == '\r') _term_posx=0;
		else {
			term_prim_write_character(_term_posx, _term_posy, _term_front_c, _term_back_c, str[i], _term_vram);
			_term_posx++;
		}
		if(_term_posx>=FX9860_TERM_WIDTH) {
			_term_posx = 0;
			_term_posy++;
			line_nb++;
		}

		
		if(_term_posy>=FX9860_TERM_HEIGHT) {
			// tmp stuff
			/*if(line_nb >= FX9860_TERM_HEIGHT-3) {
				int j;
				char * warn_str = "--- PRESS [EXE] TO SKIP ---";
				int warn_strlen = sizeof("--- PRESS [EXE] TO SKIP ---")-1;
				for(j=0; j<warn_strlen; j++)
					term_prim_write_character(j, 0, TERM9860_COLOR_WHITE, TERM9860_COLOR_BLACK, warn_str[j], _term_vram);
				disp_mono_copy_to_dd(_term_vram);

				
				while(!hwkbd_real_keydown(K_EXE));
				while(hwkbd_real_keydown(K_EXE));
				static volatile int tricks;
				for(tricks = 0; tricks<100000; tricks++);
				line_nb = 0;
			}*/

			term_prim_scroll_up(_term_vram, _term_back_c);
			_term_posy = FX9860_TERM_HEIGHT-1;
		}
	}

	// print cursor at current position
	term_prim_write_character(_term_posx, _term_posy, _term_front_c,
			_term_back_c, FX9860_TERM_CURSOR_CHAR, _term_vram);

	disp_mono_copy_to_dd(_term_vram);
}



// callback function called when a key is stroke
void fx9860_term_key_stroke(int code) {
	//TODO
	if(code < 0x80) {
		char ccode = (char)code;
		// basic echo, need to be improved (do not copy_to_dd() each time...)
		fx9860_term_print(&ccode, 1);
	}
}

void fx9860_term_init() {
	// initialize VRAM, position and mode
	
	memset(_term_vram, 0, 1024);
	_term_posx = 0;
	_term_posy = 0;

	_term_inbuf_pos = 0;
	(void)_term_inbuf;

	_term_back_c = TERM9860_COLOR_WHITE;
	_term_front_c = TERM9860_COLOR_BLACK;
}


struct file_operations* fx9860_term_get_file_op(uint16 minor) {
	if(minor == FX9860_TERM_MINOR_OUTPUT) {
		return &_fop_screen;
	}
	return NULL;
}


int fx9860_term_open(inode_t *inode, struct file *filep) {
	// consider this function may be called many times, and only for minor 1
	
	if(inode->typespec.dev.minor == FX9860_TERM_MINOR_OUTPUT) {
		return 0;
	}

	return -1;
}


int fx9860_term_release(struct file *filep) {
	// nothing to do?
	
	return 0;
}


size_t fx9860_term_write(struct file *filep, void *source, size_t len) {
	fx9860_term_print(source, len);
	return 0;
}


int fx9860_term_ioctl(struct file *filep, int cmd, void *data) {

	return -1;
}


