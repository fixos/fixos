#include "cdc_acm.h"
#include <device/usb/usb_device_protocol.h>
#include <device/usb/communication_class.h>
#include <utils/log.h>
#include <utils/strutils.h>


// called when a setup packet is received on EP0 with request type GET_DESCRIPTOR
static int cdc_acm_setup_getdescriptor(const struct usb_setup *setup);

// all other setup commands (including class-specific commands)
static int cdc_acm_setup_other(const struct usb_setup *setup);


// retains the state of the configuration, to allow to know if CDC is
// initialized (device descriptor sent, etc...)
// TODO for now it's based on 
static int _cdc_acm_configured = 0;


static const struct usb_device_desc _desc = {
	.b_length = 18,
	.b_desc_type = USB_DESC_DEV_TYPE,
	.bcd_USB = USB_WORD_TO(0x0200),
	.b_dev_class = CDC_DEV_CLASS,
	.b_dev_subclass = CDC_DEV_ACM_SUBCLASS,
	.b_dev_protocol = CDC_DEV_ACM_PROTOCOL,
	.b_max_size = 8,

	// use the vendor/product of Arduino Uno CDC (avoid problems with vendor/product drivers auto-loading)
	.id_vendor = USB_WORD_TO(0x2341),
	.id_product = USB_WORD_TO(0x0001),

	.bcd_device = USB_WORD_TO(0x0001),
	.i_manufacturer = 1,
	.i_product = 2,
	.i_serial_num = 0,
	.b_num_config = 1
};

static const struct usb_config_desc _confdesc = {
	.b_length = 9,
	.b_desc_type = USB_DESC_CONFIG_TYPE,
	// WARNING : total size is hard-coded and not realy explicit for now :
	.w_total_length = USB_WORD_TO(9 + 9+7+5+4+5 + 9+7+7),
	.b_num_inter = 2,
	.b_config_value = 1, // index begin at 1 (0 means unconfigured)
	.i_config = 0,
	.bm_attributes = USB_CONF_ATTR_SELFPOW,
	.b_max_power = 50
};


// first interface (master ACM, with 1 endpoint which is the interrupt one)
static const struct usb_interface_desc _intdesc1 = {
	.b_length = 9,
	.b_desc_type = USB_DESC_INTER_TYPE,
	.b_interface_nb = 0,
	.b_alt_settings = 0,
	.b_num_endpoints = 1,
	.b_int_class = CDC_INT_CLASS,
	.b_int_subclass = CDC_INT_ACM_SUBCLASS,
	.b_int_protocol = CDC_INT_ACM_PROTOCOL,
	.i_interface = 0
};

// second interface (slave, with 2 endpoints for bulk IN and OUT)
static const struct usb_interface_desc _intdesc2 = {
	.b_length = 9,
	.b_desc_type = USB_DESC_INTER_TYPE,
	.b_interface_nb = 1,
	.b_alt_settings = 0,
	.b_num_endpoints = 2,
	.b_int_class = CDC_INT_DATA_CLASS,
	.b_int_subclass = CDC_INT_DATA_ACM_SUBCLASS,
	.b_int_protocol = CDC_INT_DATA_ACM_PROTOCOL,
	.i_interface = 0
};


// WARNING : EndPoint descriptors are *not* const, because the abstraction
// layer do not allow to use hard-coded endpoints (the cdc_acm_init() look
// for a valid configuration of endpoints, and set appropriately)

// endpoint 2 : bulk IN for second interface
static struct usb_endpoint_desc _epdesc2 = {
	.b_length = 7,
	.b_desc_type = USB_DESC_EP_TYPE,
	//.b_endpoint_addr = USB_EP_ADDR_EP2IN,
	.bm_attributes = USB_EP_ATTR_BULK,
	.w_max_packet_size = USB_WORD_TO(64),
	.b_interval = 1
};

// endpoint 1 : bulk OUT for second interface
static struct usb_endpoint_desc _epdesc1 = {
	.b_length = 7,
	.b_desc_type = USB_DESC_EP_TYPE,
	//.b_endpoint_addr = USB_EP_ADDR_EP1OUT,
	.bm_attributes = USB_EP_ATTR_BULK,
	.w_max_packet_size = USB_WORD_TO(64),
	.b_interval = 1
};

// endpoint 3 : interrupt IN for first interface
static struct usb_endpoint_desc _epdesc3 = {
	.b_length = 7,
	.b_desc_type = USB_DESC_EP_TYPE,
	//.b_endpoint_addr = USB_EP_ADDR_EP3IN,
	.bm_attributes = USB_EP_ATTR_INTER,
	.w_max_packet_size = USB_WORD_TO(8),

	// interval for interrupt, may be changed later
	.b_interval = 255
};

static const struct usb_devqualifier_desc _devqualdesc = {
	.b_length = 10,
	.b_desc_type = USB_DESC_DEVQUAL_TYPE,
	.bcd_USB = USB_WORD_TO(0x0200),
	.b_dev_class = CDC_DEV_CLASS,
	.b_dev_subclass = CDC_DEV_ACM_SUBCLASS,
	.b_dev_protocol = CDC_DEV_ACM_PROTOCOL,
	.b_max_size = 8,
	.b_num_config = 1,
	._reserved = 0
};


// special CDC informations (functionnal descriptors)
static const struct cdc_header_fundesc _cdc_header = {
	.desc = { 3+2, CDC_DESC_FUNCTIONNAL_TYPE, CDC_FUNC_SUBTYPE_HEADER},
	.bcd_CDC = USB_WORD_FROM(CDC_BCD_VERS)
};

static const struct cdc_capability_fundesc _cdc_cap = { 
	.desc = { 3+1, CDC_DESC_FUNCTIONNAL_TYPE, CDC_FUNC_SUBTYPE_CAP},
	.bm_cap = CDC_CAP_SEND_BREAK | CDC_CAP_LINE_CODING
};

static const struct cdc_union_fundesc _cdc_union = { 
	.desc = { 3+2, CDC_DESC_FUNCTIONNAL_TYPE, CDC_FUNC_SUBTYPE_UNION},
	.b_master = 0,
	.b_slave = 1
};




static struct usb_endpoint_ability _cdc_enpoints[3] = {
	// first endpoint -> interrupt
	{ USB_EP_DIR_IN, USB_EP_ATTR_INTER, 8},
	// second endpoint -> bulk in (size is the minimum needed)
	{ USB_EP_DIR_IN, USB_EP_ATTR_BULK, 8},
	// third endpoint -> bulk out (size is the minimum needed)
	{ USB_EP_DIR_OUT, USB_EP_ATTR_BULK, 8},
};





void cdc_acm_init() {
	// get a configuration for needed endpoints
	if(usb_find_endpoint_config(3, _cdc_enpoints) == 0) {
		// We have a working config, set appropriate values
		// for endpoint descriptors
		printk(LOG_DEBUG, "cdc/acm: ep -> {0x%x, 0x%x, 0x%x}\n", _cdc_enpoints[0].endpoint,
				_cdc_enpoints[1].endpoint, _cdc_enpoints[2].endpoint);

		_epdesc1.b_endpoint_addr = _cdc_enpoints[2].endpoint;
		_epdesc1.w_max_packet_size = USB_WORD_TO(_cdc_enpoints[2].max_size);

		_epdesc2.b_endpoint_addr = _cdc_enpoints[1].endpoint;
		_epdesc2.w_max_packet_size = USB_WORD_TO(_cdc_enpoints[1].max_size);
		
		_epdesc3.b_endpoint_addr = _cdc_enpoints[0].endpoint;
		_epdesc3.w_max_packet_size = USB_WORD_TO(_cdc_enpoints[0].max_size);

		usb_set_setup_callback(cdc_acm_setup_handler);
	}
}


int cdc_acm_setup_handler(const struct usb_setup *setup) {
	// call the appropriate sub-handler
	if(setup->b_request == USB_SETUP_REQ_GET_DESCRIPTOR) {
		return cdc_acm_setup_getdescriptor(setup);
	}
	else {
		return cdc_acm_setup_other(setup);
	}
}





static int cdc_acm_setup_getdescriptor(const struct usb_setup *setup) {
	static char usbBuffer[100];

	static char manufacturer_uni[] = "H\0i\0p\0p\0i\0e\0 \0U\0n\0c\0o\0m\0p\0a\0n\0y\0";
	static char product_uni[] = "F\0i\0X\0o\0s\0 \0S\0e\0r\0i\0a\0l\0 \0e\0m\0u\0l\0a\0t\0o\0r\0";

	int data_size; // temp
	data_size = USB_WORD_FROM(setup->w_length);

	// set to 0 if a reply is sent
	int ret = -1;

	int desc_type = USB_WORD_FROM(setup->w_value) >> 8;
	int desc_index = USB_WORD_FROM(setup->w_value) & 0xFF;

	if(desc_type == USB_DESC_DEV_TYPE) {
		usb_send(USB_EP_ADDR_EP0IN, (char*)&_desc, _desc.b_length);
		ret = 0;
	}
	else if(desc_type == USB_DESC_CONFIG_TYPE) {
		int nbbuf;

		nbbuf = 0;
		memcpy(usbBuffer + nbbuf, &_confdesc, _confdesc.b_length);
		nbbuf += _confdesc.b_length;

		// first interface and endpoint :
		memcpy(usbBuffer + nbbuf, &_intdesc1, _intdesc1.b_length);
		nbbuf += _intdesc1.b_length;
		// do not forget CDC specific informations :
		memcpy(usbBuffer + nbbuf, &_cdc_header, _cdc_header.desc.b_length);
		nbbuf += _cdc_header.desc.b_length;
		memcpy(usbBuffer + nbbuf, &_cdc_cap, _cdc_cap.desc.b_length);
		nbbuf += _cdc_cap.desc.b_length;
		memcpy(usbBuffer + nbbuf, &_cdc_union, _cdc_union.desc.b_length);
		nbbuf += _cdc_union.desc.b_length;
		memcpy(usbBuffer + nbbuf, &_epdesc3, _epdesc3.b_length);
		nbbuf += _epdesc3.b_length;

		// first interface and endpoint :
		memcpy(usbBuffer + nbbuf, &_intdesc2, _intdesc2.b_length);
		nbbuf += _intdesc2.b_length;
		memcpy(usbBuffer + nbbuf, &_epdesc1, _epdesc1.b_length);
		nbbuf += _epdesc1.b_length;
		memcpy(usbBuffer + nbbuf, &_epdesc2, _epdesc2.b_length);
		nbbuf += _epdesc2.b_length;

		nbbuf = nbbuf > data_size ? data_size : nbbuf;
		usb_send(USB_EP_ADDR_EP0IN, usbBuffer, nbbuf);

		ret = 0;
	}
	else if(desc_type == USB_DESC_STRING_TYPE) {
		struct usb_string_desc str_desc = { 0, USB_DESC_STRING_TYPE };
		int tosend = 0;

		switch(desc_index) {
		case 0:
			{
				uint16 supported[1] = {USB_WORD_TO(USB_STRING_LANG_EN_US)};
				// supported languages
				str_desc.b_length = USB_DESC_STRING_LENGTH + sizeof(supported);
				memcpy(usbBuffer, &str_desc, USB_DESC_STRING_LENGTH);
				memcpy(usbBuffer + USB_DESC_STRING_LENGTH, supported, sizeof(supported));
				tosend = str_desc.b_length;
			}
			break;

		case 1:
			str_desc.b_length = USB_DESC_STRING_LENGTH + sizeof(manufacturer_uni)-1;
			memcpy(usbBuffer, &str_desc, USB_DESC_STRING_LENGTH);
			memcpy(usbBuffer + USB_DESC_STRING_LENGTH, manufacturer_uni, sizeof(manufacturer_uni)-1); 
			tosend = str_desc.b_length;
			break;

		case 2:
			str_desc.b_length = USB_DESC_STRING_LENGTH + sizeof(product_uni)-1;
			memcpy(usbBuffer, &str_desc, USB_DESC_STRING_LENGTH);
			memcpy(usbBuffer + USB_DESC_STRING_LENGTH, product_uni, sizeof(product_uni)-1); 
			tosend = str_desc.b_length;
			break;

		default:
			break;
		}

		if(tosend != 0) {
			usb_send(USB_EP_ADDR_EP0IN, usbBuffer, tosend);
			ret = 0;
		}

	}
	else if(desc_type == USB_DESC_INTER_TYPE) {
	}
	else if(desc_type == USB_DESC_EP_TYPE) {
	}
	else if(desc_type == USB_DESC_DEVQUAL_TYPE) {
		usb_send(USB_EP_ADDR_EP0IN, (char*)&_devqualdesc, _devqualdesc.b_length);
		ret = 0;
	}

	return ret;
}


static int cdc_acm_setup_other(const struct usb_setup *setup) {
	static struct cdc_line_coding _line_coding = {
		.dw_DTE_rate = USB_DWORD_TO(9600),
		.b_char_format = CDC_LINE_CHAR_STOP_1,
		.b_parity_type = CDC_LINE_PARITY_NONE,
		.b_data_bits = 8
	};


	switch(setup->b_request) {
	case CDC_REQ_SET_LINE_CODING:
		// set line coding structure
		// FIXME maybe an implementation problem, but Linux doesn't seem to
		// send anything after this request?!
		//usb_receive(USB_EP_ADDR_EP0OUT, (char*)&_line_coding, 7, 0);
		usb_send(USB_EP_ADDR_EP0IN, NULL, 0);
		break;

	case CDC_REQ_GET_LINE_CODING:
		// return line coding structure
		usb_send(USB_EP_ADDR_EP0IN, (char*)&_line_coding, 7);
		break;

	case CDC_REQ_SET_CONTROL_LINE_STATE:
		// consider this request is a good way to know if the ACM is now configured
		_cdc_acm_configured = 1;

		// TODO ?
		// send a 0-length response
		usb_send(USB_EP_ADDR_EP0IN, NULL, 0);
		break;


	default:
		printk(LOG_DEBUG, "USB: Unknown setup req %d\n", setup->b_request);
		return -1;
	}

	return 0;
}


int cdc_acm_is_ready() {
	// TODO better implementation (for now, USB disconnection do not reset
	// the return value).
	return _cdc_acm_configured != 0;
}


size_t cdc_acm_send(const char *data, size_t size) {
	if(_cdc_acm_configured) {
		return usb_send(_epdesc2.b_endpoint_addr, data, size);
	}
	return 0;
}


size_t cdc_acm_receive(char *dest, size_t size) {
	if(_cdc_acm_configured) {
		// blocking receive
		return usb_receive(_epdesc1.b_endpoint_addr, dest, size, 0);
	}
	return 0;
}

