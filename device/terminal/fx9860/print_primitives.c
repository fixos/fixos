#ifndef DISPLAY_T6K11_TERMINAL_H
#define DISPLAY_T6K11_TERMINAL_H

#include <device/display/generic_mono.h>
#include <device/terminal/generic_early_term.h>

#include "print_primitives.h"

// Font 3x5 data
unsigned int _iFont3x5[128]={
 3960, //Character 000 [NUL]
 3960, //Character 001 [SOH]
 3960, //Character 002 [STX]
 3960, //Character 003 [ETX]
 3960, //Character 004 [EOT]
 3960, //Character 005 [ENQ]
 3960, //Character 006 [ACK]
 3960, //Character 007 [BEL]
 3960, //Character 008 [BS]
 3960, //Character 009 [TAB]
 3960, //Character 010 [LF]
 3960, //Character 011 [VT]
32000, //Character 012 [FF]
 3960, //Character 013 [CR]
 3960, //Character 014 [SO]
 3960, //Character 015 [SI]
 3960, //Character 016 [DEL]
 3960, //Character 017 [XON]
 3960, //Character 018 [DC2]
 3960, //Character 019 [XOFF]
 3960, //Character 020 [DC4]
 3960, //Character 021 [NAK]
 3960, //Character 022 [SYN]
 3960, //Character 023 [ETB]
 3960, //Character 024 [CAN]
 3960, //Character 025 [EM]
 3960, //Character 026 [SUB]
 3960, //Character 027 [ESC]
 3960, //Character 028 [FS]
 3960, //Character 029 [GS]
 3960, //Character 030 [RS]
 3960, //Character 031 [US]
    0, //Character 032 [SP]
 8338, //Character 033 [!]
   45, //Character 034 ["]
16254, //Character 035 [#]
32223, //Character 036 [$]
21157, //Character 037 [%]
10922, //Character 038 [&]
  166, //Character 039 [']
17556, //Character 040 [(]
 5265, //Character 041 [)]
 2728, //Character 042 [*]
 1488, //Character 043 [+]
10624, //Character 044 [,]
  448, //Character 045 [-]
13824, //Character 046 [.]
 4756, //Character 047 [/]
31599, //Character 048 [0]
29850, //Character 049 [1]
29671, //Character 050 [2]
31207, //Character 051 [3]
18925, //Character 052 [4]
31183, //Character 053 [5]
31695, //Character 054 [6]
18727, //Character 055 [7]
31727, //Character 056 [8]
31215, //Character 057 [9]
13851, //Character 058 [:]
 5147, //Character 059 [;]
17492, //Character 060 [<]
 3640, //Character 061 [=]
 5393, //Character 062 [>]
 8355, //Character 063 [?]
11171, //Character 064 [@]
23530, //Character 065 [A]
15083, //Character 066 [B]
25166, //Character 067 [C]
15211, //Character 068 [D]
29647, //Character 069 [E]
 5071, //Character 070 [F]
27470, //Character 071 [G]
23533, //Character 072 [H]
29847, //Character 073 [I]
11044, //Character 074 [J]
23277, //Character 075 [K]
29257, //Character 076 [L]
23421, //Character 077 [M]
23403, //Character 078 [N]
11114, //Character 079 [O]
 4843, //Character 080 [P]
17770, //Character 081 [Q]
23275, //Character 082 [R]
14478, //Character 083 [S]
 9367, //Character 084 [T]
31597, //Character 085 [U]
11117, //Character 086 [V]
12141, //Character 087 [W]
23213, //Character 088 [X]
 9389, //Character 089 [Y]
29351, //Character 090 [Z]
25750, //Character 091 [[]
17545, //Character 092 [\]
26918, //Character 093 []]
   42, //Character 094 [^]
28672, //Character 095 [_]
  139, //Character 096 [`]
27554, //Character 097 [a]
15225, //Character 098 [b]
25200, //Character 099 [c]
27516, //Character 100 [d]
29674, //Character 101 [e]
 9684, //Character 102 [f]
14766, //Character 103 [g]
23385, //Character 104 [h]
 9346, //Character 105 [i]
11012, //Character 106 [j]
22345, //Character 107 [k]
29843, //Character 108 [l]
23408, //Character 109 [m]
23376, //Character 110 [n]
11088, //Character 111 [o]
 5976, //Character 112 [p]
19824, //Character 113 [q]
 4840, //Character 114 [r]
14448, //Character 115 [s]
17586, //Character 116 [t]
27496, //Character 117 [u]
11112, //Character 118 [v]
32616, //Character 119 [w]
23208, //Character 120 [x]
 5480, //Character 121 [y]
29496, //Character 122 [z]
25814, //Character 123 [{]
 9362, //Character 124 [|]
13715, //Character 125 [}]
  408, //Character 126 [~]
 3960  //Character 127 []
};


#define _PCDOT(n, cwidth) disp_mono_set_pixel(((n)%(cwidth))+x,((n)/(cwidth))+y,(map & (1<<(n)))>>(n),(unsigned char*)vram)

void term_prim_write_character(unsigned int posx, unsigned int posy, int front_c, int back_c, char c, void *vram) {

	unsigned int backcolor, frontcolor;
	int x, y;
	backcolor = term_prim_mask_color(back_c);
	frontcolor = term_prim_mask_color(front_c);
	x = posx * 4;
	y = posy * 6;
	if((c >= 0) && (x>-3) && (x<128) && (y>-5) && (y<64)) {
		int tmpmap = _iFont3x5[(int)c];
		unsigned int map = (frontcolor&tmpmap) | (backcolor&(~tmpmap));
		_PCDOT(0, 3); _PCDOT(1, 3); _PCDOT(2, 3); disp_mono_set_pixel(x+3, y+0, backcolor&0x01, vram);
		_PCDOT(3, 3); _PCDOT(4, 3); _PCDOT(5, 3); disp_mono_set_pixel(x+3, y+1, backcolor&0x01, vram);
		_PCDOT(6, 3); _PCDOT(7, 3); _PCDOT(8, 3); disp_mono_set_pixel(x+3, y+2, backcolor&0x01, vram);
		_PCDOT(9, 3); _PCDOT(10, 3); _PCDOT(11, 3); disp_mono_set_pixel(x+3, y+3, backcolor&0x01, vram);
		_PCDOT(12, 3); _PCDOT(13, 3); _PCDOT(14, 3); disp_mono_set_pixel(x+3, y+4, backcolor&0x01, vram);
		disp_mono_set_pixel(x+0, y+5, backcolor&0x01, vram);
		disp_mono_set_pixel(x+1, y+5, backcolor&0x01, vram);
		disp_mono_set_pixel(x+2, y+5, backcolor&0x01, vram);
		disp_mono_set_pixel(x+3, y+5, backcolor&0x01, vram);
	}
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

