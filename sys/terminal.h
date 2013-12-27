#ifndef SYS_TERMINAL_H
#define SYS_TERMINAL_H

#include <device/display/terminal.h>


// write something into the terminal
void terminal_write(const char *str);

// change the terminal character colors
void terminal_set_colors(int front_c, int back_c);

// clear the terminal (and put the cursor in (1,1) )
// WARNING : the terminal is filled with the back color used
void terminal_clear();

// move the cursor at the given position
void terminal_set_pos(int posx, int posy);

// get current position
int terminal_posx();
int terminal_posy();


// set the VRAM used
void terminal_set_vram(vram_t vram);

#endif
