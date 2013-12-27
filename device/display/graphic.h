#ifndef DISPLAY_GRAPHIC_H
#define DISPLAY_GRAPHIC_H

/**
  * This file contain the interface of graphic-display related functions.
  * The reals functions used depends of the low-level implementation. 
  */


// this file is the low-level dd used, it provide the 'vram_t' type definition
#include "T6K11/T6K11.h"


// copy the given VRAM onto the screen
void copy_to_dd(vram_t vram);

// put a pixel into the VRAM
void set_pixel(int x, int y, color_t color, vram_t vram);

int graphic_width();
int graphic_height();


// TODO :
// draw a bitmap into the VRAM. The real type of bitmap_t depends of the driver used.
//void draw_bitmap(int x, int y, bitmap_t bitmap, vram_t vram); 

#endif // DISPLAY_GRAPHIC_H
