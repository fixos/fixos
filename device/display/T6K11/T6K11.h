#ifndef DISPLAY_T6K11_H
#define DISPLAY_T6K11_H


// real type of vram_t
typedef unsigned char *vram_t;
typedef int color_t;
struct _mono_bitmap {
	unsigned char *bitmap;
	int w;
	int h;
};
typedef struct _mono_bitmap bitmap_t;

// Colors Typedef :
#define PIXEL_WHITE	0
#define PIXEL_BLACK	1
#define PIXEL_XOR	2


// Return 0xFFFFFFFF if it's a dark color, 0x00000000 else
unsigned int mask_color();

#endif  //DISPLAY_T6K11_H

