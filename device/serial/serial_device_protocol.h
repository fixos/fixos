#ifndef _DEVICE_SERIAL_SERIAL_DEVICE_PROTOCOL_H
#define _DEVICE_SERIAL_SERIAL_DEVICE_PROTOCOL_H

void serial_init(void);

void serial_transmit(unsigned char value);
unsigned char serial_receive(void);

#endif