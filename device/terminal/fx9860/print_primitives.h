#ifndef _DEVICE_TERMINAL_FX9860_PRINT_PRIMITIVES_H
#define _DEVICE_TERMINAL_FX9860_PRINT_PRIMITIVES_H


#define FX9860_TERM_WIDTH	32
#define FX9860_TERM_HEIGHT	10


/**
 * write a character with the terminal police
 * TODO : less arguments to write_char!!!
 */
void term_prim_write_character(unsigned int posx, unsigned int posy, int front_c, int back_c, char c, void *vram);


/**
 * Scroll up the terminal display from 1 character high
 */
void term_prim_scroll_up(void *vram, int back_c);


unsigned int term_prim_mask_color(int color);

#endif //_DEVICE_TERMINAL_FX9860_PRINT_PRIMITIVES_H