#include <device/display/generic_mono.h>

size_t disp_mono_width() {
	return 128;
}


size_t disp_mono_height() {
	return 64;
}


// TODO find a way to inline these functions AND to keep generic aspect
uint32 disp_mono_get_pixel(int x, int y, void *vram) {
	// no check on (x;y) to keep performances...
	return !!( ((char*)vram)[y*16 + x/8] & (1 << x%8));
}


void disp_mono_draw_bitmap(int x, int y, unsigned char *bitmap, short w, short h, void *vvram)
{
	unsigned char *vram = vvram;
	// no clipping, only check of limits
	if(x>=0 && y>=0 && x+w<=128 && y+h<=64) {
		char shift;
		short start = y*16+x/8;
		int i;

		shift = x%8;

		// for each line, 4 optimized patterns :
		// if the whole line is exactly 1 byte in destination (8 pixel and aligned)
		if(w==8 && shift==0) {
			for(i=0; i<h; i++) {
				vram[start]=bitmap[i];
				start += 16; // next VRAM line
			}
		}
		
		// if the whole line is copied in 1 byte in destination, but using masks
		else if(w<8 && shift+w <= 8) {
			int end_pad = 8-(shift+w);
			unsigned char mask = ((0xFF << end_pad) & (0xFF >> shift));

			for(i=0; i<h; i++) {
				vram[start] = (vram[start] & ~mask) | ((bitmap[i] >> shift) & mask);
				start += 16; // next VRAM line
			}
		}

		// if the whole line is 1 byte in source, but between 2 bytes in dest
		else if(w<=8) {
			int end_pad = 8-((shift+w) % 8);
			unsigned char l_mask = 0xFF >> shift;
			unsigned char r_mask = 0xFF << end_pad;
			int end_shift = (x+w)%8;

			for(i=0; i<h; i++) {
				vram[start] = (vram[start] & ~l_mask) | ((bitmap[i] >> shift) & ~l_mask);
				vram[start+1] = (vram[start+1] & ~r_mask) | ((bitmap[i] << end_shift) & ~r_mask);
				start += 16; // next VRAM line
			}

		}

		// general case : more than 1 full byte
		else {
			int end_pad = 8-((shift+w) % 8);
			unsigned char l_mask = 0xFF >> shift;
			unsigned char r_mask = 0xFF << end_pad;
			int end_shift = (x+w)%8;
			char line_len = (w-1)/8+1;
			int bitmap_pos = 0;

			// if no initial shifting, simplest algorithm
			if(shift == 0) {
				int nb_full = w/8;
				for(i=0; i<h; i++) {
					int offset;

					// copy all full bytes
					for(offset=0; offset<nb_full; offset++)
						vram[start+offset] = bitmap[bitmap_pos+offset];

					// if last byte need to be shifted
					if(end_shift != 0) {
						vram[start+offset] = (vram[start+offset] & ~r_mask) | ((bitmap[bitmap_pos+offset] << end_shift) & ~r_mask);
					}

					start += 16; // next VRAM line
					bitmap_pos += line_len;
				}
			}

			// each byte need 2 shifting before to be copied (not aligned)
			else {
				int nb_full = (w-shift)/8;
				for(i=0; i<h; i++) {
					int offset;

					vram[start] = (vram[start] & ~l_mask) | ((bitmap[bitmap_pos] >> shift) & ~l_mask);

					// copy all full bytes
					for(offset=1; offset<nb_full+1; offset++) {
						vram[start+offset] = (bitmap[bitmap_pos+offset-1] << (8-shift))
							| (bitmap[bitmap_pos+offset] >> shift);
					}


					// if last byte need to be shifted
					if(end_shift != 0) {
						// TODO not sure if its working fine
						vram[start+offset] = (vram[start+offset] & ~r_mask) | ((bitmap[bitmap_pos + line_len-1] << end_shift) & ~r_mask);
					}

					start += 16; // next VRAM line
					bitmap_pos += line_len;
				}
			}
		}
	}
}

