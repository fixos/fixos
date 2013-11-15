#include "eeprom.h"

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



#define BYTEWR(addr,value) *(volatile unsigned short*)(EEPROM_BASE_ADDRESS + (addr)) = (unsigned short)(value)

#define BYTERD(addr) (*(volatile unsigned short*)(EEPROM_BASE_ADDRESS + (addr)))

int eeprom_get_device_id() {
		int value;

		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_A);
		BYTEWR(COMMAND_ADDR_B, COMMAND_CMD_B);
		BYTEWR(COMMAND_ADDR_AUTOSELECT_A(0), COMMAND_CMD_C);

		value = ((0xFF & BYTERD(COMMAND_ADDR_AUTOSELECT_DEVID1(0))) << 16 )
			+ ((0xFF & BYTERD(COMMAND_ADDR_AUTOSELECT_DEVID2(0))) << 8 )
			+ (0xFF & BYTERD(COMMAND_ADDR_AUTOSELECT_DEVID3(0)) );

		BYTEWR(0, COMMAND_CMD_RESET);


		return value;
}



unsigned short eeprom_get_cfi(int cfi_query) {
	unsigned short ret;

	BYTEWR(COMMAND_ADDR_CFI, COMMAND_CMD_CFI);
	ret = BYTERD(cfi_query);

	BYTEWR(0, COMMAND_CMD_RESET);
	
	return ret;
}



void eeprom_program_byte(unsigned int addr, unsigned char data) {
	unsigned short sdata = data + 0xAA00;
	unsigned int maskedaddr = addr & EEPROM_ADDRESS_MASK;

	// protection to avoid stupid erasement of Casio OS!
	// WARNING 
	// TODO weaker protection
	if((maskedaddr >= EEPROM_MODIFY_LOW && maskedaddr < EEPROM_MODIFY_HIGH)) {
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_A);
		BYTEWR(COMMAND_ADDR_B, COMMAND_CMD_B);
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_PROGRAM);

		BYTEWR(maskedaddr, sdata);

		// TODO check if mandatory
		BYTEWR(0, COMMAND_CMD_RESET);
	}
	
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

		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_A);
		BYTEWR(COMMAND_ADDR_B, COMMAND_CMD_B);
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_ERASE);
		BYTEWR(COMMAND_ADDR_A, COMMAND_CMD_A);
		BYTEWR(COMMAND_ADDR_B, COMMAND_CMD_B);

		BYTEWR(sector, COMMAND_CMD_ERASE_SECTOR);

		// wait for end of erasement :
		while((BYTERD(0) & 0x80) != 0x80); 
	}
	
}
