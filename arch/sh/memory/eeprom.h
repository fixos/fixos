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
#define EEPROM_CFI_DEVICE_SIZE	0x4E

unsigned short eeprom_get_cfi(int cfi_query);


/**
 * Program byte in EEPROM
 */
void eeprom_program_byte(unsigned int addr, unsigned char data);


/**
 * Erase a Flash Sector at the given address (only sector bytes are used)
 * (blocking function)
 */
void eeprom_erase_sector(unsigned int addr);

#endif // _ARCH_SH_MEMORY_EEPROM_H
