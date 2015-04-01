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

#ifndef _ARCH_SH_MEMORY_EEPROM_H
#define _ARCH_SH_MEMORY_EEPROM_H

/**
 * Contains some primitives to abstract a bit EEPROM controlers.
 * For now, the goal is not realy abstraction, so some functions may have
 * no sense for some chips.
 *
 * The current design allow only one EEPROM type to be use by the kernel.
 * A better design may be interesting but not useful in pratice for now.
 */


/**
 * Return the Device ID of the used chip.
 */
int eeprom_get_device_id();


/**
 * CFI Commands (Common Flash Interface)
 */
// return log2(size) in bits (size = 2^ret bits)
#define EEPROM_CFI_DEVICE_SIZE	0x4E 

unsigned short eeprom_get_cfi(int cfi_query);


/**
 * Program word (2 bytes length) byte in EEPROM
 * One of the two bytes may be 0xFF if unused (neutral in EEPROM programming)
 */
void eeprom_program_word(unsigned int addr, unsigned short data);


/**
 * Program any byte array in a unique call (may be quicker than multiple
 * eeprom_program_word() calls )
 * Byte/Word alignement is checked internaly.
 */
void eeprom_program_array(unsigned int addr_dest, unsigned char *from, unsigned int size);



/**
 * Erase a Flash Sector at the given address (only sector bytes are used)
 * (blocking function)
 */
void eeprom_erase_sector(unsigned int addr);

#endif // _ARCH_SH_MEMORY_EEPROM_H
