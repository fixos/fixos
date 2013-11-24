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
#define SD_CMD_RESET		0
//TODO


struct sd_resp32 {
	uint16 w0;
	uint16 w1;
};

struct sd_resp128 {
	uint16 w0;
	uint16 w1;
	uint16 w2;
	uint16 w3;
	uint16 w4;
	uint16 w5;
	uint16 w6;
	uint16 w7;
};


/**
 * Send CMD[command] with given argument.
 * Returns 0 if success, negative value else.
 */
//int sd_send_command(int command, uint32 arg);
int sd_send_command(int command, uint16 arg0, uint16 arg1);


/**
 * Get 32bits response of previous executed command into given struct.
 */
int sd_get_resp32(struct sd_resp32 *resp);


/**
 * Get 128bits response of previous executed command into given struct.
 */
int sd_get_resp128(struct sd_resp128 *resp);


int sd_init();




#endif //_DEVICE_SD_CARD_H
