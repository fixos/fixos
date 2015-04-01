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

#ifndef _CPU_SH_7705_CASIO_H
#define _CPU_SH_7705_CASIO_H

#include "7705.h"

/**
 * Specific register definition for Casio's modified version of sh7705.
 * Some devices are not present in sh7705, such as SDHI, some ports, etc...
 *
 * Should be use with caution, all come from reverse-engineering on specific
 * models!
 */



/**
 * Contains information about known area of 0xA45500xx (probably SDHI or MMC
 * control and access).
 */

struct st_sdhi {
	unsigned short command; //word_u1; // 0, maybe Command Register
	char wk1[2]; // 2 unknown bytes


	// arg0 is the low word of command, arg1 the high word!!!
	unsigned short arg0; //word_u2; // 4, maybe first command argument
	unsigned short arg1; //word_u3; // 6, maybe second command argument

	unsigned short word_u4; // 8
	unsigned short word_u5; // 10

	unsigned short resp0; //word_u6; // 12, maybe a response register (first word)
	unsigned short resp1; //word_u7; // 14, maybe a response register (second word)
	unsigned short resp2; //word_u8; // 16, maybe a response register (only for 128 bits)
	unsigned short resp3; //word_u9; // 18, maybe a response register (only for 128 bits)
	unsigned short resp4; //word_u10; // 20, maybe a response register (only for 128 bits)

	unsigned short resp5; //word_u11; // 22, maybe a response register (only for 128 bits)
	unsigned short resp6; //word_u12; // 24, maybe a response register (only for 128 bits)
	unsigned short resp7; //word_u13; // 26, maybe a response register (only for 128 bits)

	unsigned short word_u14; // 28
	unsigned short word_u15; // 30


	unsigned short config0; //word_u16; // 32
	unsigned short config1; //word_u17; // 34
	unsigned short config2; //word_u18; // 36
	unsigned short config3; //word_u19; // 38
	unsigned short config4; //word_u20; // 40

	/*
	unsigned short word_u2; // 10
	unsigned short word_u2; // 10
	unsigned short word_u2; // 10
	*/

	// TODO

	unsigned short data; //transfert; // 48 or 0x30

	// TODO
	
	unsigned short word_u21; // 64, seems to start transfert when set to 1

	unsigned short word_far_1; // 216

	unsigned short word_far_2; // 224
};



union un_ipri {
	unsigned short WORD;
	struct {
		unsigned char		:8;
		unsigned char _SDI	:4;
		unsigned char		:4;
	} BIT;
};

union un_sdclkr {
	unsigned char BYTE;
	struct {
		unsigned char		:6;
		unsigned char u1	:1;
		unsigned char u2	:1;
	} BIT;
};

union un_uknport {
	unsigned char BYTE;
	struct {
		unsigned char u1	:1; // seems to be related to SD card
		unsigned char u2	:1; // seems to be related to USB port
		unsigned char		:6;
	} BIT;
};


#define TOTALY_UNKNOWN_1	(*(volatile unsigned char*)	0xA4000184)
#define UKNPORT	(*(volatile union un_uknport*)	0xA40001AC)
#define SDCLKCR	(*(volatile union un_sdclkr*)	0xA40A0004)
#define IPRI	(*(volatile union un_ipri*)		0xA4080006)
#define SDHI	(*(volatile struct st_sdhi*)	0xA4550000)

#endif // _CPU_SH_7705_CASIO_H
