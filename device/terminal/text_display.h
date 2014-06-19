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
};


#endif //_DEVICE_TERMINAL_TEXT_DISPLAY_H
