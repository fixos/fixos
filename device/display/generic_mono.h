/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
 * Read a pixel from the VRAM, and return its color.
 */
uint32 disp_mono_get_pixel(int x, int y, void *vram);

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
