#include <arch/sh/7705_Casio.h>

volatile static int totime;

static void delay() {
	for (totime=0; totime<20; totime++);
}

int is_key_down(unsigned char code) {
	unsigned char column, row, portm = 0;
	unsigned char tmp;
	unsigned short pfc_a, pfc_b, pfc_m;
	int ret=0;
	
	column = 0x01 << (code >> 4);
	
	tmp = code & 0x0F;
	if (tmp > 7) {
		portm = 1;
		tmp -= 8;
	}
	row = 0x01 << tmp;

	pfc_a = PFC.PACR.WORD;
	pfc_b = PFC.PBCR.WORD;
	pfc_m = PFC.PMCR.WORD;

	PFC.PBCR.WORD = 0xAAAA;
	PFC.PMCR.WORD = (0xAAAA & 0x000F) | (PFC.PMCR.WORD & ~0x000F);
	PFC.PACR.WORD = 0x5555;
	delay();
	PA.DR.BYTE = ~column;
	if (portm) ret = !(PM.DR.BYTE & row);
	else ret = !(PB.DR.BYTE & row);
	
	if (ret == 0) {
		PFC.PBCR.WORD = 0x5555;
		PFC.PMCR.WORD = (0x5555 & 0x000F) | (PFC.PMCR.WORD & ~0x000F);	
		PFC.PACR.WORD = 0xAAAA;
		delay();
		if (portm) {
			PM.DR.BYTE = (~row & 0x0B) | (PM.DR.BYTE & ~0x0B);
			PB.DR.BYTE = 0xFF;
		}
		else {
			PM.DR.BYTE = 0x0B | (PM.DR.BYTE & ~0x0B);
			PB.DR.BYTE = ~row;
		}
		ret = !(PA.DR.BYTE & column);
	}

	delay();
	PFC.PACR.WORD = pfc_a;
	PFC.PBCR.WORD = pfc_b;
	PFC.PMCR.WORD = pfc_m;
	delay();
	return ret;
}
