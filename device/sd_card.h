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

#ifndef _DEVICE_SD_CARD_H
#define _DEVICE_SD_CARD_H

#include <utils/types.h>

/**
 * Misc definitions and functions related to SD and MMC card protocol.
 *
 * Only high-level is described here, implementation should only show
 * this API if possible...
 */

// sd protocol commands
#define SD_CMD_RESET				0
#define SD_CMD_READ_SINGLE_BLOCK	17
#define SD_CMD_READ_MULTIPLE_BLOCK	18
//TODO


// get full c_size from a sd_reg_csd union, using c_size_l and c_size_h fields
#define SD_CSD_GET_C_SIZE(csd)	(((csd).content.c_size_h << 2) | (csd).content.c_size_l)


typedef uint32 sd_resp32_t;

// ordered in "big-endian" style (r0 high)
struct _sd_resp128 {
	uint32 r0;
	uint32 r1;
	uint32 r2;
	uint32 r3;
};

typedef struct _sd_resp128 sd_resp128_t;

// unions to represent SD registers

// CID (Card IDentification register)
union sd_reg_cid {
	sd_resp128_t resp;
	struct {
		uint8 mid;				// manufacturer ID, 8bits
		char oid[2];			// application ID, 2 ASCII chars
		char pnm[5];			// product name
		uint8 prv;				// product revision in 2 BCD digits (Major.Minor)
		uint32 psn;				// product serial number
		char 			:4;		// 4 unused bits
		uint16 mdt		:12;	// manufacturing date (4 bits for month, 8 bits for year - 2000)
		uint8 crc		:7;		// CRC7 of message
		char			:1;		// not used, always 1...
	} content;
};

//CSD (Card Specific Data)
union sd_reg_csd {
	sd_resp128_t resp;
	
	// this strange struct definition is used because of C alignement constraints
	// to ensure each sub-part is a full 32 bits slice, and no field is between two
	// byte/long alignement (so each field is uint32 even if is less than 8 bits length)
	struct {
		struct {
		uint32 csd_ver			:2;		// Version (0b00 for SD, 0x01 for SDHC and SDXC
		uint32					:6;
		uint32 taac				:8;		// data read access time-1
		uint32 nsac				:8;		// data read access time-2 in CLK cycles
		uint32 tran_speed		:8;		// max data transfer speed (0x32 or 0x5A)
		};

		struct {
		uint32 ccc				:12;	// Card Command Class (in theory 0b01x110110101)
		uint32 read_bl_len 		:4;		// Read Block Length
		uint32 read_bl_partial 	:1;		// 1 for SD card
		uint32 write_blk_misalign:1;	// Write Block Misalignement
		uint32 read_blk_misalign :1;	// Read Block Misalignement
		uint32 dsr_imp			:1;		// DSR implemented
		uint32 					:2;
		uint32 c_size_h			:10;	// device size (10 high weight bits)
		};

		struct {
		uint32 c_size_l			:2;		// device size (2 low weight bits)
		uint32 vdd_r_curr_min	:3;		// max read current at VDD min
		uint32 vdd_r_curr_max	:3;		// max read current at VDD max
		uint32 vdd_w_curr_min	:3;		// max write current at VDD min
		uint32 vdd_w_curr_max	:3;		// max write current at VDD max
		uint32 c_size_mult		:3;		// device size multiplier
		uint32 erase_blk_en		:1;		// Erase Block Enable
		uint32 sector_size		:7;		// Erase Sector Size
		uint32 wp_grp_size		:7;		// Write Protect Group Size
		};

		struct {
		uint32 wp_grp_enable		:1;		// Write Protect Group Enable
		uint32					:2;
		uint32 r2w_factor		:3;		// Write Speed Factor
		uint32 write_bl_len		:4;		// Write Block Length
		uint32 write_bl_partial	:1;		// Partial Block for Write allowed
		uint32					:5;
		uint32 file_format_grp	:1;		// File Format Group (W(1))
		uint32 copy				:1;		// Copy Flag (W(1))
		uint32 perm_write_protect:1;		// Permanent Write Protection (W(1))
		uint32 tmp_write_protect :1;		// emporary Write Protection (W)
		uint32 file_format		:2;		// File Format (W(1))
		uint32					:2;
		uint32 crc				:7;		// CRC7 (W)
		uint32					:1;		// Allways 1
		};
	} content;
};


/**
 * Send CMD[command] with given argument.
 * Returns 0 if success, negative value else.
 */
//int sd_send_command(int command, uint32 arg);
int sd_send_command(int command, uint32 arg);


/**
 * Get 32bits response of previous executed command into given struct.
 */
int sd_get_resp32(sd_resp32_t *resp);


/**
 * Get 128bits response of previous executed command into given struct.
 */
int sd_get_resp128(sd_resp128_t *resp);


int sd_init();


/**
 * Read a single block of 512 bytes from SD card.
 * TODO better read function, DMAC usage
 */
int sd_read_block(int firstblock, char *dest);



#endif //_DEVICE_SD_CARD_H
