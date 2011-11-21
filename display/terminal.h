#ifndef DISPLAY_TERMINAL_H
#define DISPLAY_TERMINAL_H

#include "T6K11/T6K11.h"

#define TCOLOR_BLACK	0
#define TCOLOR_RED	1
#define TCOLOR_GREEN	2
#define TCOLOR_YELLOW	3
#define TCOLOR_BLUE	4
#define TCOLOR_MAGENTA	5
#define TCOLOR_CYAN	6
#define TCOLOR_WHITE	7


// write a character with the terminal police
// TODO : less arguments to write_char!!!
void write_character(unsigned int posx, unsigned int posy, int front_c, int back_c, char c, vram_t vram);

// scroll up the terminal display from 1 character high
void scroll_up(vram_t vram, int back_c);

// return the maximum terminal characters displayed in width and in height
int termchar_width();
int termchar_height();


#endif //DISPLAY_TERMINAL_H

