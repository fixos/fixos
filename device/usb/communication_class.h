#ifndef _DEVICE_USB_COMMUNICATION_CLASS_H
#define _DEVICE_USB_COMMUNICATION_CLASS_H


/**
 * Contains data structures and definitions used by USB Communication Data Class.
 * Some parts of CDC are present, and subclass Abstract Communication Model (ACM)
 * is partialy included.
 */


#define CDC_DEV_CLASS			2
#define CDC_DEV_ACM_SUBCLASS	0
#define CDC_DEV_ACM_PROTOCOL	0


#define CDC_INT_CLASS			2
#define CDC_INT_ACM_SUBCLASS	2
#define CDC_INT_ACM_PROTOCOL	1

#define CDC_INT_DATA_CLASS		10
#define CDC_INT_DATA_ACM_SUBCLASS	0
#define CDC_INT_DATA_ACM_PROTOCOL	0


#define CDC_BCD_VERS			0x0110


// CDC Class-specific Request types :
#define CDC_REQ_SET_LINE_CODING			0x20
#define CDC_REQ_GET_LINE_CODING			0x21
#define CDC_REQ_SET_CONTROL_LINE_STATE	0x22
#define CDC_REQ_SEND_BREAK				0x23


// CDC specific structures

#define CDC_LINE_PARITY_NONE			0
#define CDC_LINE_PARITY_ODD				1
#define CDC_LINE_PARITY_EVEN			2
#define CDC_LINE_PARITY_MARK			3
#define CDC_LINE_PARITY_SPACE			4

#define CDC_LINE_CHAR_STOP_1			0
#define CDC_LINE_CHAR_STOP_1_5			1
#define CDC_LINE_CHAR_STOP_2			2

struct cdc_line_coding {
	uint32 dw_DTE_rate;	// bauds rate
	uint8 b_char_format;
	uint8 b_parity_type;
	uint8 b_data_bits;	// bits per data (6,7,8,16...)
};

// CDC capability flags
#define CDC_CAP_SEND_BREAK		0x04
#define CDC_CAP_LINE_CODING		0x02

// type for CDC functionnal descriptor :
#define CDC_DESC_FUNCTIONNAL_TYPE	0x24

struct cdc_functionnal_desc {
	uint8 b_length; // 3 + content size
	uint8 b_type;
	uint8 b_subtype;
	// content
};

#define CDC_FUNC_SUBTYPE_HEADER		0x00
struct cdc_header_fundesc {
	struct cdc_functionnal_desc desc;
	uint16 bcd_CDC; // CDC version
};

#define CDC_FUNC_SUBTYPE_CAP		0x02
struct cdc_capability_fundesc {
	struct cdc_functionnal_desc desc;
	uint8 bm_cap;
};


#define CDC_FUNC_SUBTYPE_UNION		0x06
struct cdc_union_fundesc {
	struct cdc_functionnal_desc desc;
	uint8 b_master;
	uint8 b_slave;
};


#endif //_DEVICE_USB_COMMUNICATION_CLASS_H

