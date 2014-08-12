#ifndef _FIXOS_INTERFACE_FXKEYBOARD_H
#define _FIXOS_INTERFACE_FXKEYBOARD_H

/**
 * fx9860-like direct keyboard handling device definition.
 * Keycodes are based on their "matrix value" on fx9860 to be more efficient
 * in the kernel, maybe it should be a problem for other casio platforms.
 */

// type used to read the direct keyboard device
struct fxkey_event {
	unsigned char key;
	unsigned char event;
};


// event type
#define K_EVENT_PRESSED		0x01
#define K_EVENT_RELEASED	0x02


// key codes :
#define K_F1	0x69
#define K_F2	0x59
#define K_F3	0x49
#define K_F4	0x39
#define K_F5	0x29
#define K_F6	0x19

#define K_SHIFT	0x68
#define K_OPTN	0x58
#define K_VARS	0x48
#define K_MENU	0x38
#define K_LEFT	0x28
#define K_UP	0x18

#define K_ALPHA	0x67
#define K_SQR	0x57
#define K_EXPO	0x47
#define K_EXIT	0x37
#define K_DOWN	0x27
#define K_RIGHT	0x17

#define K_THETA	0x66
#define K_LOG	0x56
#define K_LN	0x46
#define K_SIN	0x36
#define K_COS	0x26
#define K_TAN	0x16

#define K_FRAC	0x65
#define K_FD	0x55
#define K_LPAR	0x45
#define K_RPAR	0x35
#define K_COMMA	0x25
#define K_STORE	0x15

#define K_7		0x64
#define K_8		0x54
#define K_9		0x44
#define K_DEL	0x34

#define K_4		0x63
#define K_5		0x53
#define K_6		0x43
#define K_MULT	0x33
#define K_DIV	0x23

#define K_1		0x62
#define K_2		0x52
#define K_3		0x42
#define K_PLUS	0x32
#define K_MINUS	0x22

#define K_0		0x61
#define K_DOT	0x51
#define K_EXP	0x41
#define K_NEG	0x31
#define K_EXE	0x21

#define K_AC	0x00

#endif //_FIXOS_INTERFACE_FXKEYBOARD_H
