#ifndef _DEVICE_DISPLAY_GENERIC_MONO_H
#define _DEVICE_DISPLAY_GENERIC_MONO_H

/**
 * Generic Monochrome display interface and definitions.
 * This file contain the interface of graphic-display related functions.
 * The reals functions used depends of the low-level implementation. 
 */

#include <utils/types.h>


/**
 * Definition of colors used (black/white for monochrome display)
 */
#define DISPLAY_COLOR_WHITE	0
#define DISPLAY_COLOR_BLACK 1
#define DISPLAY_COLOR_XOR	2 // temp



/**
 * Copy the given VRAM onto the screen.
 */
void disp_mono_copy_to_dd(void *vram);

/**
 * Write a pixel into the VRAM, wih the given color and position (x;y).
 */
void disp_mono_set_pixel(int x, int y, uint32 color, void *vram);

/**
 * Returns width/height of the display, in pixels.
 */
size_t disp_mono_width();
size_t disp_mono_height();

/**
 * Returns the size of the VRAM used by this implementation.
 */
size_t disp_mono_vram_size();

/**
 * Draw a (w;h) sized bitmap on the VRAM at position (x;y) for its top-left corner.
 * Format of bitmap is usual Casio format :
 * each pixel is 1 bit (1 for black, 0 for white), and each line begin with
 * a new byte (if w%8 != 0, last byte of a line contains some padding bits).
 * The implementation doesn't make a full clipping (if at least 1 pixel of the
 * bitmap should be write outside the screen, no partial bitmap is writen).
 */
void disp_mono_draw_bitmap(int x, int y, unsigned char *bitmap, short w, short h, void *vram); 

#endif //_DEVICE_DISPLAY_GENERIC_MONO_H
