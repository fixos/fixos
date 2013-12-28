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

// TODO :
// draw a bitmap into the VRAM. The real type of bitmap_t depends of the driver used.
//void draw_bitmap(int x, int y, bitmap_t bitmap, vram_t vram); 

#endif //_DEVICE_DISPLAY_GENERIC_MONO_H
