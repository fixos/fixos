#include <device/serial/serial_device_protocol.h> 
#include <arch/sh/7705_Casio.h>
#include <arch/sh/interrupt.h>
// #include <utils/strutils.h> // str basic functions
#include <utils/cyclic_fifo.h>
#include <arch/sh/kdelay.h>

#include <utils/log.h>

// uncomment to enable verbose debug mode for SERIAL module
// #define DEBUG_SERIAL

/**
 * This file contains the implementation of serial_device_protocol.h interface
 * for SH3 7705 and similar architecture.
 */

void stbcr_init(void) {
	// Start clock for SCIF2
	STBCR3.BIT._SCIF2 = 0;
}

void scscr_init(void) {
	SCIF2.SCSCR.BIT.TIE = SCIF2.SCSCR.BIT.RIE = SCIF2.SCSCR.BIT.TE = SCIF2.SCSCR.BIT.RE = 0;
	SCIF2.SCSCR.BIT.TSIE = 0;
	SCIF2.SCSCR.BIT.ERIE = 0;
	SCIF2.SCSCR.BIT.BRIE = 0;
	SCIF2.SCSCR.BIT.DRIE = 0;
	// clock selection
	SCIF2.SCSCR.BIT.CKE = 0;
}

void scsmr_init(void) {
	// sampling control (asynchronous mode only)
	SCIF2.SCSMR.BIT.SRC = 0;
	// 0: asynchronous mode, 1: clock synchronous mode
	SCIF2.SCSMR.BIT.CA = 1;
	// 0: 8-bits data, 1: 7-bits data
	SCIF2.SCSMR.BIT.CHR = 0;
	// 0: no parity-bit, 1: parity-bit added & checked
	SCIF2.SCSMR.BIT._PE = 0;
	// 0: even-parity, 1: odd-parity
	SCIF2.SCSMR.BIT.OE = 0;
	// 0: 1 stop-bit, 1: 2 stop-bits
	SCIF2.SCSMR.BIT.STOP = 1;
	// internal source clock, 00: src, 01: src/4, 10: src/16, 11: src/64
	SCIF2.SCSMR.BIT.CKS = 0;
}

void scfcr_init(void) {
	// only in asynchronous mode
	if(SCIF2.SCSMR.BIT.CA == 0) {
		SCIF2.SCFCR.BIT.TSE = 1;
		SCIF2.SCFCR.BIT.MCE = 0;
	}
	SCIF2.SCFCR.BIT.LOOP = 0;
	SCIF2.SCFCR.BIT.RFRST = SCIF2.SCFCR.BIT.TFRST = 0;
	// set the number of receive data bytes that sets the receive data full (RDF) flag in the serial status register (SCSSR)
	SCIF2.SCFCR.BIT.RTRG = 0;
	SCIF2.SCFCR.BIT.TTRG = 3;
}

void sci_init(void) {
	scscr_init();

	// reset FIFOs
	SCIF2.SCFCR.BIT.TFRST = SCIF2.SCFCR.BIT.RFRST = 1; 

	scsmr_init();

	// in case of clock synchronous mode
	// SCIF2.SCBRR = Pφ / ( 4 * 2^(2n-1) * B ) * 10^6 - 1
	// Pφ = 14.7456MHz
	// n = SCIF2.SCSMR.BIT.CKS
	// B = Bit rate (Baud * SCIF2.SCSMR.BIT.CHR -Don't know if it's right-)
	SCIF2.SCBRR = 95;
	
	scfcr_init();

	if(SCIF2.SCSSR.BIT.TDFE == 1) SCIF2.SCSSR.BIT.TDFE = 0;
	if(SCIF2.SCSSR.BIT.RDF == 1) SCIF2.SCSSR.BIT.RDF = 0;

	kusleep(1); // 1-bit interval

	SCIF2.SCTDSR = 0;

	SCIF2.SCSCR.BIT.TIE = SCIF2.SCSCR.BIT.RIE = 0;
	// enable Tx/Rx
	SCIF2.SCSCR.BIT.TE = SCIF2.SCSCR.BIT.RE = 1;
}


void serial_init(void) {
	stbcr_init();
	sci_init();

	kusleep(1); // 1-bit interval
}

/**
 * From here to end, I have copied code from RevolutionFX.
 * But if it's like serial_init, I have to recode all
 */

void serial_transmit(unsigned char value) {
	while (SCIF2.SCSSR.BIT.TEND != 1);
	SCIF2.SCFTDR = value;
	kusleep(1);
	SCIF2.SCSSR.BIT.TEND = SCIF2.SCSSR.BIT.TDFE = 0;
}

unsigned char serial_receive(void) {
	int i;
	if (SCIF2.SCSSR.BIT.RDF == 1) {
		i = SCIF2.SCFRDR;
		SCIF2.SCSSR.BIT.RDF = 0;
		return i;
	}
}

void serial_transmit_bytes(unsigned char *data, size_t length) {
	int i;
	for(i = 0; i < length; i++) {
		serial_transmit(data[i]);
	}
}

void serial_receive_bytes(unsigned char *data, size_t max_length) {
	int i;
	for(i = 0; i < max_length; i++) {
		serial_transmit(data[i]);
	}
}

void serial_stop(void) {
	SCIF2.SCSCR.BIT.TE = SCIF2.SCSCR.BIT.RE = 0;
}