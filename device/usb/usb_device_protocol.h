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

#ifndef _DEVICE_USB_USB_DEVICE_PROTOCOL_H
#define _DEVICE_USB_USB_DEVICE_PROTOCOL_H


/**
 * This file describe the interface and definitions needed to use USB at the
 * device part (this is *NOT* the host protocol).
 * Functions are designed to be implemented by arch-specific modules.
 */


#include <utils/types.h>


/**
 * USB Standard descriptors and descriptor size (used during Status step) :
 */
#define USB_DESC_DEV_TYPE		0x01
#define USB_DESC_DEV_LENGTH		18

#define USB_DESC_CONFIG_TYPE	0x02
#define USB_DESC_CONFIG_LENGTH	9

#define USB_DESC_STRING_TYPE	0x03
#define USB_DESC_STRING_LENGTH	2

#define USB_DESC_INTER_TYPE		0x04
#define USB_DESC_INTER_LENGTH	9

#define USB_DESC_EP_TYPE		0x05
#define USB_DESC_EP_LENGTH		7

#define USB_DESC_DEVQUAL_TYPE	0x06
#define USB_DESC_DEVQUAL_LENGTH	10



// String Language identifiers :
#define USB_STRING_LANG_NONE	0x0000
#define USB_STRING_LANG_EN_US	0x0409



/**
 * Status descriptors structures.
 * For each, b_desc_type and b_length MUST be set according to related definitions.
 * WARNING : do *NOT* use sizeof(struct usb_...) as the b_length field value
 * (because of alignement and pading done)!
 *
 * For more informations about how to use these structures and fields value,
 * please read USB documentation.
 */

struct usb_device_desc {
	uint8 b_length; // should be 18
	uint8 b_desc_type; // should be 0x01
	uint16 bcd_USB;
	uint8 b_dev_class;
	uint8 b_dev_subclass;
	uint8 b_dev_protocol;
	uint8 b_max_size;

	uint16 id_vendor;
	uint16 id_product;

	uint16 bcd_device;

	uint8 i_manufacturer;
	uint8 i_product;
	uint8 i_serial_num;

	uint8 b_num_config;
};


struct usb_config_desc {
	uint8 b_length; // should be 9
	uint8 b_desc_type; // should be 0x02
	uint16 w_total_length;
	uint8 b_num_inter;
	uint8 b_config_value;
	uint8 i_config;
	uint8 bm_attributes;
	uint8 b_max_power;
};


struct usb_interface_desc {
	uint8 b_length; // should be 9
	uint8 b_desc_type; // should be 0x04
	uint8 b_interface_nb; // index of this interface
	uint8 b_alt_settings;
	uint8 b_num_endpoints;
	uint8 b_int_class;
	uint8 b_int_subclass;
	uint8 b_int_protocol;
	uint8 i_interface;
};


struct usb_endpoint_desc {
	uint8 b_length; // should be 7
	uint8 b_desc_type; // should be 0x05
	uint8 b_endpoint_addr;
	uint8 bm_attributes;
	uint16 w_max_packet_size;
	uint8 b_interval;
};


struct usb_string_desc {
	uint8 b_length; // should be 2  + content length
	uint8 b_desc_type; // should be 0x03
	// content (usualy Unicode string, so 2n bytes)...
};

// needed for USB 2.0 in Full-Speed (else the device must STALL the
// STATUS step for Device Qualifier (0x06)
struct usb_devqualifier_desc {
	uint8 b_length; // should be 10
	uint8 b_desc_type; // should be 0x06
	uint16 bcd_USB;
	uint8 b_dev_class;
	uint8 b_dev_subclass;
	uint8 b_dev_protocol;
	uint8 b_max_size; // max size for endpoint 0
	uint8 b_num_config;
	uint8 _reserved;
};



/**
 * Describe the structure of a SETUP data, received on control EP.
 */
struct usb_setup {
	union {
		uint8 BYTE;
		struct {
			uint8 direction		:1;
			uint8 type			:2;
			uint8 recipient		:5;
		} BIT;
	} bm_request_type;
	
	uint8 b_request;
	uint16 w_value;
	uint16 w_index;
	uint16 w_length;
};

/**
 * Endpoints address definitions.
 * This is used at many places in USB protocol (for example, in endpoint
 * descriptors).
 */
#define USB_EP_ADDR(ep,dir) ((ep)|(dir))

#define USB_EP_EP0				0x00
#define USB_EP_EP1				0x01
#define USB_EP_EP2				0x02
#define USB_EP_EP3				0x03

#define USB_EP_DIR_OUT			0x00
#define USB_EP_DIR_IN			0x80

#define USB_EP_ADDR_EP0OUT		USB_EP_ADDR(USB_EP_EP0, USB_EP_DIR_OUT)
#define USB_EP_ADDR_EP0IN		USB_EP_ADDR(USB_EP_EP0, USB_EP_DIR_IN)
#define USB_EP_ADDR_EP1OUT		USB_EP_ADDR(USB_EP_EP1, USB_EP_DIR_OUT)
#define USB_EP_ADDR_EP2IN		USB_EP_ADDR(USB_EP_EP2, USB_EP_DIR_IN)
#define USB_EP_ADDR_EP3IN		USB_EP_ADDR(USB_EP_EP3, USB_EP_DIR_IN)
#define USB_EP_ADDR_EP3OUT		USB_EP_ADDR(USB_EP_EP3, USB_EP_DIR_OUT)


/**
 * Values for bm_attribute of endpoints descriptors.
 */
#define USB_EP_ATTR_BULK		0x02
#define USB_EP_ATTR_INTER		0x03

/**
 * Values for bm_attribute of configuration descriptors.
 */
#define USB_CONF_ATTR_SELFPOW	0xC0


/** 
 * b_request values for SETUP data (usb_setup structure).
 */
#define USB_SETUP_REQ_GET_DESCRIPTOR	0x06



/**
 * USB protocol use little-endian encoding for numbers larger than 1 byte.
 * These macros allow to access (USB_xxx_FROM) and to set (USB_xxx_TO) word and
 * long word values, from either little or big endian architecture.
 */
// LSB word for big endian arch :
#define USB_WORD_TO(x) ( (((x)&0xFF00) >> 8 ) | ( ((x)&0xFF) << 8) )
#define USB_WORD_FROM(x) ( (((x)&0xFF00) >> 8 ) | ( ((x)&0xFF) << 8) )

// LSB double word for big endian arch :
#define USB_DWORD_TO(x) ( ((x) >> 24 ) | (((x)&0x00FF0000) >> 8 ) | (((x)&0x0000FF00) << 8 ) | ( ((x)&0x000000FF) << 24) )
#define USB_DWORD_FROM(x) ( ((x) >> 24 ) | (((x)&0x00FF0000) >> 8 ) | (((x)&0x0000FF00) << 8 ) | ( ((x)&0x000000FF) << 24) )





// USB Device interface finition (must be implemented in arch-specific code)

/**
 * Initialize USB internal module if any.
 */
void usb_init();

/**
 * Send size bytes of data from given endpoint (USB_EP_ADDR or USB_EP_ADDR_xxx).
 * Negative value is returned if endpoint can't send data for any reason.
 * Else, number of bytes successfuly sent is returned.
 */
int usb_send(int endpoint, const char *data, size_t size);


#define USB_RECV_PARTIAL		0x01
#define USB_RECV_NONBLOCK		0x02

/**
 * Receive size bytes of data from given endpoint.
 * If flags contains USB_RECV_PARTIAL, size is the maximum size allowed, but
 * the function may returns once at least 1 byte is received (whereas, without
 * this flag, the function returns only when exactly size bytes are received).
 * If flags contains USB_RECV_NONBLOCK, the function acts like with
 * USB_RECV_PARTIAL, but returns immediatly if no data were stored in temp
 * buffer from the last call (may returns 0 for '0 bytes received').
 *
 * Returns the number of bytes effectively received, or negative value
 * if any error occurs or if endpoint can't receive any data.
 * TODO should be marked obsolete (synchronous receive!)
 */
int usb_receive(int endpoint, char *data, size_t size, int flags);


typedef int (*usb_receive_callback_t)(char c);

/**
 * Callback used for each byte received asynchronously (a NULL callback
 * will do nothing).
 * Only some endpoints may support that.
 */
void usb_set_receive_callback(int endpoint, usb_receive_callback_t callback);


/**
 * Structure used to describe endpoints at kernel level.
 */
struct usb_endpoint_ability {
	uint8 endpoint; // endpoint identifier
	uint8 attributes; // same as endpoint descriptor bm_attribute (like USB_EP_ATTR_BULK)
	uint16 max_size; // endpoint max data size
};


/**
 * This function use partialy filed usb_endpoint_ability structures to find the best
 * configuration that respect endpoints attribute and size.
 * eps is a ep_number length array. Each element contains :
 * - attributes field filled (to specify the pipe type, like burst, interrupt,
 *   isosynchronous).
 * - max_size field represent the *minimum* size required for this endpoint
 * - endpoint field must contains either USB_EP_DIR_OUT or USB_EP_DIR_IN
 *
 * If a configuration of existing endpoints matches to given requierements,
 * returns 0 and set endpoint field of given structures. In addition, max_size
 * field may be modified, but is guaranted to be more than or equal the initial
 * value.
 * If no working configurations exist, returns negative value (eps content may
 * be partialy changed in this case).
 */
int usb_find_endpoint_config(int ep_number, struct usb_endpoint_ability *eps);



// type used for setup callback function
// return value must be 0 if setup was handled properly, -1 else.
typedef int (*usb_setup_callback_t)(const struct usb_setup*);


/**
 * Set the callback function call each time USB Setup is received on EP0.
 */
void usb_set_setup_callback(usb_setup_callback_t callback);

#endif //_DEVICE_USB_USB_DEVICE_PROTOCOL_H
