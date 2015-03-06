#ifndef DISPLAY_T6K11_TERMINAL_H
#define DISPLAY_T6K11_TERMINAL_H

#include <device/display/generic_mono.h>
#include <device/terminal/generic_early_term.h>

#include "print_primitives.h"

// Font 3x5 (usual bitmap format)
static const unsigned char _font_3x5[129][5] = {
{0, 224, 160, 224, 0}, //Character 000 [NUL]
{0, 224, 160, 224, 0}, //Character 001 [SOH]
{0, 224, 160, 224, 0}, //Character 002 [STX]
{0, 224, 160, 224, 0}, //Character 003 [ETX]
{0, 224, 160, 224, 0}, //Character 004 [EOT]
{0, 224, 160, 224, 0}, //Character 005 [ENQ]
{0, 224, 160, 224, 0}, //Character 006 [ACK]
{0, 224, 160, 224, 0}, //Character 007 [BEL]
{0, 224, 160, 224, 0}, //Character 008 [BS]
{0, 224, 160, 224, 0}, //Character 009 [TAB]
{0, 224, 160, 224, 0}, //Character 010 [LF]
{0, 224, 160, 224, 0}, //Character 011 [VT]
{0, 0, 32, 96, 224}, //Character 012 [FF]
{0, 224, 160, 224, 0}, //Character 013 [CR]
{0, 224, 160, 224, 0}, //Character 014 [SO]
{0, 224, 160, 224, 0}, //Character 015 [SI]
{0, 224, 160, 224, 0}, //Character 016 [DEL]
{0, 224, 160, 224, 0}, //Character 017 [XON]
{0, 224, 160, 224, 0}, //Character 018 [DC2]
{0, 224, 160, 224, 0}, //Character 019 [XOFF]
{0, 224, 160, 224, 0}, //Character 020 [DC4]
{0, 224, 160, 224, 0}, //Character 021 [NAK]
{0, 224, 160, 224, 0}, //Character 022 [SYN]
{0, 224, 160, 224, 0}, //Character 023 [ETB]
{0, 224, 160, 224, 0}, //Character 024 [CAN]
{0, 224, 160, 224, 0}, //Character 025 [EM]
{0, 224, 160, 224, 0}, //Character 026 [SUB]
{0, 224, 160, 224, 0}, //Character 027 [ESC]
{0, 224, 160, 224, 0}, //Character 028 [FS]
{0, 224, 160, 224, 0}, //Character 029 [GS]
{0, 224, 160, 224, 0}, //Character 030 [RS]
{0, 224, 160, 224, 0}, //Character 031 [US]
{0, 0, 0, 0, 0}, //Character 032 [SP]
{64, 64, 64, 0, 64}, //Character 033 [!]
{160, 160, 0, 0, 0}, //Character 034 ["]
{96, 224, 160, 224, 192}, //Character 035 [#]
{224, 192, 224, 96, 224}, //Character 036 [$]
{160, 32, 64, 128, 160}, //Character 037 [%]
{64, 160, 64, 160, 64}, //Character 038 [&]
{96, 32, 64, 0, 0}, //Character 039 [']
{32, 64, 64, 64, 32}, //Character 040 [(]
{128, 64, 64, 64, 128}, //Character 041 [)]
{0, 160, 64, 160, 0}, //Character 042 [*]
{0, 64, 224, 64, 0}, //Character 043 [+]
{0, 0, 96, 32, 64}, //Character 044 [,]
{0, 0, 224, 0, 0}, //Character 045 [-]
{0, 0, 0, 192, 192}, //Character 046 [.]
{32, 64, 64, 128, 128}, //Character 047 [/]
{224, 160, 160, 160, 224}, //Character 048 [0]
{64, 192, 64, 64, 224}, //Character 049 [1]
{224, 32, 224, 128, 224}, //Character 050 [2]
{224, 32, 224, 32, 224}, //Character 051 [3]
{160, 160, 224, 32, 32}, //Character 052 [4]
{224, 128, 224, 32, 224}, //Character 053 [5]
{224, 128, 224, 160, 224}, //Character 054 [6]
{224, 32, 32, 32, 32}, //Character 055 [7]
{224, 160, 224, 160, 224}, //Character 056 [8]
{224, 160, 224, 32, 224}, //Character 057 [9]
{192, 192, 0, 192, 192}, //Character 058 [:]
{192, 192, 0, 64, 128}, //Character 059 [;]
{32, 64, 128, 64, 32}, //Character 060 [<]
{0, 224, 0, 224, 0}, //Character 061 [=]
{128, 64, 32, 64, 128}, //Character 062 [>]
{192, 32, 64, 0, 64}, //Character 063 [?]
{192, 32, 96, 160, 64}, //Character 064 [@]
{64, 160, 224, 160, 160}, //Character 065 [A]
{192, 160, 192, 160, 192}, //Character 066 [B]
{96, 128, 128, 128, 96}, //Character 067 [C]
{192, 160, 160, 160, 192}, //Character 068 [D]
{224, 128, 224, 128, 224}, //Character 069 [E]
{224, 128, 224, 128, 128}, //Character 070 [F]
{96, 128, 160, 160, 96}, //Character 071 [G]
{160, 160, 224, 160, 160}, //Character 072 [H]
{224, 64, 64, 64, 224}, //Character 073 [I]
{32, 32, 32, 160, 64}, //Character 074 [J]
{160, 160, 192, 160, 160}, //Character 075 [K]
{128, 128, 128, 128, 224}, //Character 076 [L]
{160, 224, 160, 160, 160}, //Character 077 [M]
{192, 160, 160, 160, 160}, //Character 078 [N]
{64, 160, 160, 160, 64}, //Character 079 [O]
{192, 160, 192, 128, 128}, //Character 080 [P]
{64, 160, 160, 64, 32}, //Character 081 [Q]
{192, 160, 192, 160, 160}, //Character 082 [R]
{96, 128, 64, 32, 192}, //Character 083 [S]
{224, 64, 64, 64, 64}, //Character 084 [T]
{160, 160, 160, 160, 224}, //Character 085 [U]
{160, 160, 160, 160, 64}, //Character 086 [V]
{160, 160, 160, 224, 64}, //Character 087 [W]
{160, 160, 64, 160, 160}, //Character 088 [X]
{160, 160, 64, 64, 64}, //Character 089 [Y]
{224, 32, 64, 128, 224}, //Character 090 [Z]
{96, 64, 64, 64, 96}, //Character 091 [[]
{128, 128, 64, 64, 32}, //Character 092 []
{96, 32, 32, 32, 96}, //Character 093 []]
{64, 160, 0, 0, 0}, //Character 094 [^]
{0, 0, 0, 0, 224}, //Character 095 [_]
{192, 128, 64, 0, 0}, //Character 096 [`]
{64, 32, 96, 160, 96}, //Character 097 [a]
{128, 224, 160, 160, 192}, //Character 098 [b]
{0, 96, 128, 128, 96}, //Character 099 [c]
{32, 224, 160, 160, 96}, //Character 100 [d]
{64, 160, 224, 128, 224}, //Character 101 [e]
{32, 64, 224, 64, 64}, //Character 102 [f]
{96, 160, 96, 32, 192}, //Character 103 [g]
{128, 192, 160, 160, 160}, //Character 104 [h]
{64, 0, 64, 64, 64}, //Character 105 [i]
{32, 0, 32, 160, 64}, //Character 106 [j]
{128, 128, 160, 192, 160}, //Character 107 [k]
{192, 64, 64, 64, 224}, //Character 108 [l]
{0, 96, 160, 160, 160}, //Character 109 [m]
{0, 64, 160, 160, 160}, //Character 110 [n]
{0, 64, 160, 160, 64}, //Character 111 [o]
{0, 192, 160, 192, 128}, //Character 112 [p]
{0, 96, 160, 96, 32}, //Character 113 [q]
{0, 160, 192, 128, 128}, //Character 114 [r]
{0, 96, 128, 32, 192}, //Character 115 [s]
{64, 96, 64, 64, 32}, //Character 116 [t]
{0, 160, 160, 160, 96}, //Character 117 [u]
{0, 160, 160, 160, 64}, //Character 118 [v]
{0, 160, 160, 224, 224}, //Character 119 [w]
{0, 160, 64, 160, 160}, //Character 120 [x]
{0, 160, 160, 64, 128}, //Character 121 [y]
{0, 224, 32, 128, 224}, //Character 122 [z]
{96, 64, 192, 64, 96}, //Character 123 [{]
{64, 64, 64, 64, 64}, //Character 124 [|]
{192, 64, 96, 64, 192}, //Character 125 [}]
{0, 192, 96, 0, 0}, //Character 126 [~]
{0, 224, 160, 224, 0}, //Character 127 []
{0xA0, 0x40, 0xA0, 0x40, 0xA0} //Character 177, used for cursor position
};



// union used to be sure char_bmp is 4-bytes aligned
union bmp_fast_4x6{
	unsigned char char_bmp[6]; // 4*6 bitmap, containing character and borders
	unsigned int fast[2]; // for fast operations
};


// used to store/restore a single character area
static union bmp_fast_4x6 _stored_char;


void term_prim_write_character(unsigned int posx, unsigned int posy, int front_c, int back_c, char c, void *vram) {

	unsigned int backcolor, frontcolor;
	int x, y;
	backcolor = term_prim_mask_color(back_c);
	frontcolor = term_prim_mask_color(front_c);
	x = posx * 4;
	y = posy * 6;
	if((c >= 0 || c==FX9860_TERM_CURSOR_CHAR) && (x>-3) && (x<128) 
			&& (y>-5) && (y<64))
	{
		const unsigned char *raw_char;
		union bmp_fast_4x6 bmp;

		if(c>=0)
			raw_char = _font_3x5[(int)c];
		else
			raw_char = _font_3x5[128];

		bmp.char_bmp[0] = raw_char[0];
		bmp.char_bmp[1] = raw_char[1];
		bmp.char_bmp[2] = raw_char[2];
		bmp.char_bmp[3] = raw_char[3];
		bmp.char_bmp[4] = raw_char[4];
		bmp.char_bmp[5] = 0x00;
		
		bmp.fast[0] = (frontcolor & bmp.fast[0]) | (backcolor & (~bmp.fast[0]));
		bmp.fast[1] = (frontcolor & bmp.fast[1]) | (backcolor & (~bmp.fast[1]));

		disp_mono_draw_bitmap(x, y, bmp.char_bmp, 4, 6, vram);
	}
}


void term_prim_store_character(unsigned int posx, unsigned int posy,
		void *vram)
{
	int x = posx * 4;
	int y = posy * 6;
	int i, j;

	// the algorithm used here is... hum... a bit stupid and slow?
	for(j=0; j<6; j++) {
		char curline = 0x00;
		for(i=0; i<4; i++) {
			curline |= (disp_mono_get_pixel(x+i, y+j, vram)) << i;
		}
		_stored_char.char_bmp[j] = curline;
	}
}


void term_prim_restore_character(unsigned int posx, unsigned int posy,
		void *vram)
{
	int x = posx * 4;
	int y = posy * 6;
	disp_mono_draw_bitmap(x, y, _stored_char.char_bmp, 4, 6, vram);
}


void term_prim_scroll_up(void *vram, int back_c) {
	int i;
	unsigned int *l_vram = (unsigned int*)vram;
	unsigned int backcolor = term_prim_mask_color(back_c);
	for(i=0; i<(4*(64-10)); i++) {
		l_vram[i] = l_vram[i+4*6];
	}
	for(i=(4*(64-10)); i<(4*(64-4)); i++) {
		l_vram[i] = backcolor;
	}
}


unsigned int term_prim_mask_color(int color) {
	if(color == EARLYTERM_COLOR_BLACK)
		return 0xFFFFFFFF;
	return 0x00000000; 
}

#endif //DISPLAY_T6K11_TERMINAL_H

