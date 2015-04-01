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

#include "eeprom.h"
#include <arch/sh/interrupt.h>


// WARNING : protection address, do not modify if you don't know what you're doing!
#define EEPROM_MODIFY_LOW	0x1A0000
#define EEPROM_MODIFY_HIGH	0x200000 // not included


#define EEPROM_BASE_ADDRESS 0xA0000000

#define EEPROM_ADDRESS_MASK	0x003FFFFF

// EEPROM commands and address (for BYTE mode!)
#define COMMAND_ADDR_A					0xAAA
#define COMMAND_ADDR_B					0x554
#define COMMAND_ADDR_CFI				0x0AA

#define COMMAND_ADDR_AUTOSELECT_A(bank) (0xAAA | ((bank) & 0x3C0000))

#define COMMAND_ADDR_AUTOSELECT_DEVID1(bank) (0x002 | ((bank) & 0x3C0000))
#define COMMAND_ADDR_AUTOSELECT_DEVID2(bank) (0x01C | ((bank) & 0x3C0000))
#define COMMAND_ADDR_AUTOSELECT_DEVID3(bank) (0x01E | ((bank) & 0x3C0000))

#define COMMAND_CMD_A					0xAA
#define COMMAND_CMD_B					0x55
#define COMMAND_CMD_C					0x90
#define COMMAND_CMD_RESET				0xF0
#define COMMAND_CMD_CFI					0x98
#define COMMAND_CMD_PROGRAM				0xA0
#define COMMAND_CMD_ERASE				0x80
#define COMMAND_CMD_ERASE_SECTOR		0x30

#define COMMAND_CMD_BYPASS				0x20
#define COMMAND_CMD_BYPASS_PROGRAM		0xA0
#define COMMAND_CMD_BYPASS_RESET1		0x90
#define COMMAND_CMD_BYPASS_RESET2		0x00



#define BYTEWR(addr,value) *(volatile unsigned short*)(EEPROM_BASE_ADDRESS | (addr)) = (unsigned short)(value)

#define BYTERD(addr) (*(volatile unsigned short*)(EEPROM_BASE_ADDRESS | (addr)))


int eeprom_wait_erase(unsigned int addr, unsigned short expected_data);




int eeprom_get_device_id() {
	int value;

	interrupt_inhibit_all(1);

	BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_A);
	BYTEWR(COMMAND_ADDR_B, COMMAND_CMD_B);
	BYTEWR(COMMAND_ADDR_AUTOSELECT_A(0), COMMAND_CMD_C);

	value = ((0xFF & BYTERD(COMMAND_ADDR_AUTOSELECT_DEVID1(0))) << 16 )
		+ ((0xFF & BYTERD(COMMAND_ADDR_AUTOSELECT_DEVID2(0))) << 8 )
		+ (0xFF & BYTERD(COMMAND_ADDR_AUTOSELECT_DEVID3(0)) );

	BYTEWR(0, COMMAND_CMD_RESET);

	interrupt_inhibit_all(0);

	return value;
}



unsigned short eeprom_get_cfi(int cfi_query) {
	unsigned short ret;

	interrupt_inhibit_all(1);

	BYTEWR(COMMAND_ADDR_CFI, COMMAND_CMD_CFI);
	ret = BYTERD(cfi_query);
	BYTEWR(0, COMMAND_CMD_RESET);

	interrupt_inhibit_all(0);
	
	return ret;
}


/**
 * Inline function, must be called JUST AFTER program command!
 * WARNING : unused since ERASE detection seems to work better for PROGRAM
 */
/*
inline int eeprom_wait_togglebit(unsigned int addr) {
	unsigned short data, ndata;
	data = BYTERD(addr);
	while( ((ndata = BYTERD(addr)) & 0x20) != 0x20) {
		if((ndata & 0x40) != (data & 0x40))
			return 0;
	}
	
	ndata = BYTERD(addr);
	return !((ndata & 0x40) != (data & 0x40));
}*/


void eeprom_program_word(unsigned int addr, unsigned short data) {
	unsigned int maskedaddr = addr & EEPROM_ADDRESS_MASK;

	interrupt_inhibit_all(1);
	// protection to avoid stupid erasement of Casio OS!
	// WARNING 
	// TODO weaker protection
	if((maskedaddr >= EEPROM_MODIFY_LOW && maskedaddr < EEPROM_MODIFY_HIGH)) {
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_A);
		BYTEWR(COMMAND_ADDR_B, COMMAND_CMD_B);
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_PROGRAM);

		BYTEWR(maskedaddr, data);
		eeprom_wait_erase(maskedaddr, data);

		// TODO check if mandatory
		BYTEWR(0, COMMAND_CMD_RESET);
	}

	interrupt_inhibit_all(0);
}


void eeprom_program_array(unsigned int addr_dest, unsigned char *from, unsigned int size) {
	// check if size and address is valid
	unsigned int maskedlow = addr_dest & EEPROM_ADDRESS_MASK;
	unsigned int maskedhigh = (addr_dest + size) & EEPROM_ADDRESS_MASK;


	interrupt_inhibit_all(1);

	if(maskedlow >= EEPROM_MODIFY_LOW && maskedhigh < EEPROM_MODIFY_HIGH) {
		// enter in bypass mode (2 cycles by write)
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_A);
		BYTEWR(COMMAND_ADDR_B, COMMAND_CMD_B);
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_BYPASS);

		// first byte if needed to have addr_dest even
		if(addr_dest % 2) {
			unsigned short tocopy = 0xFF00 + *from;
			BYTEWR(0, COMMAND_CMD_BYPASS_PROGRAM);
			BYTEWR(maskedlow-1, tocopy);
			eeprom_wait_erase(maskedlow-1, tocopy);
			from++;
			maskedlow++;
			size--;
		}

		while(size >= 2) {
			unsigned short tocopy = (from[0] << 8) + from[1];
			// TODO from

			BYTEWR(0, COMMAND_CMD_BYPASS_PROGRAM);
			BYTEWR(maskedlow, tocopy);
			eeprom_wait_erase(maskedlow, tocopy);

			from += 2;
			maskedlow += 2;
			size -= 2;
		}

		if(size != 0) {
			// assume size if 1 and copy last unaligned byte
			unsigned short tocopy = 0x00FF + (*from << 8);
			BYTEWR(0, COMMAND_CMD_BYPASS_PROGRAM);
			BYTEWR(maskedlow, tocopy);
			eeprom_wait_erase(maskedlow, tocopy);
		}


		// reset from bypass mode (special sequence)
		BYTEWR(0, COMMAND_CMD_BYPASS_RESET1);
		BYTEWR(0, COMMAND_CMD_BYPASS_RESET2);
	}

	interrupt_inhibit_all(0);
}


int eeprom_wait_erase(unsigned int addr, unsigned short expected_data) {
	// from S29JL032H manual, page 39 :
	// try to read DQ0~DQ7 from ADDR, wait for DQ7=bit7(DATA) or DQ5=1
	// then read again and DQ7 must be equal to bit7(DATA) (else operation failed)
	
	unsigned short curbyte = BYTERD(addr);
	int quitloop = 0;

	while(!quitloop) {	
		curbyte = BYTERD(addr);
		if ((curbyte & 0x80) == (expected_data & 0x80))
			return 0;
		quitloop = ((curbyte & 0x20) == 0x20);
	}

	// read again
	curbyte = BYTERD(addr);
	return !((curbyte && 0x80) == (expected_data & 0x80));
 }


void eeprom_erase_sector(unsigned int addr) {
	unsigned int maskedaddr = addr & EEPROM_ADDRESS_MASK;

	// protection to avoid stupid erasement of Casio OS!
	// WARNING 
	// TODO weaker protection
	if((maskedaddr >= EEPROM_MODIFY_LOW && maskedaddr < EEPROM_MODIFY_HIGH)) {
		unsigned int sector = maskedaddr >= 0x10000 ?
			maskedaddr & 0xFFFF0000 :
			maskedaddr & 0xFFFFE000;

		interrupt_inhibit_all(1);
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_A);
		BYTEWR(COMMAND_ADDR_B, COMMAND_CMD_B);
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_ERASE);
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_A);
		BYTEWR(COMMAND_ADDR_B, COMMAND_CMD_B);

		BYTEWR(sector, COMMAND_CMD_ERASE_SECTOR);

		// wait for end of erasement :
		eeprom_wait_erase(maskedaddr, 0xFFFF);
		interrupt_inhibit_all(0);

		// no reset mandatory, but Casio's OS use it now
	}
}
