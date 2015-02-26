#ifndef _FIXOS_INTERFACE_TTY_H
#define _FIXOS_INTERFACE_TTY_H

/**
 * Data structure and ioctls related to TTY control.
 */

#include <fixos/ioctl.h>


struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	// no ws_xpixel/ws_ypixel ?
};


// type for special characters, termios' c_cc field
typedef unsigned char cc_t;

// type for terminal baud rate
typedef unsigned int speed_t;

// type for terminal modes
typedef unsigned int tcflags_t;

// subscripts for c_cc field :
#define VEOF		0
#define VEOL		1
#define VERASE		2
#define VINTR		3
#define VKILL		4
#define VMIN		5
#define VQUIT		6
#define VSTART		7
#define VSTOP		8
#define VSUSP		9
#define VTIME		10

#define NCCS		11

struct termios {
	tcflags_t c_iflag;
	tcflags_t c_oflag;
	tcflags_t c_cflag;
	tcflags_t c_lflag;
	cc_t c_cc[NCCS];
};


/**
 * Flags for local mode (c_lflag field)
 */
#define ECHO		(1<<0)
#define ECHOE		(1<<1)
#define ECHOK		(1<<2)
#define ECHONL		(1<<3)
#define ICANON		(1<<4)
#define IEXTEN		(1<<5)
#define ISIG		(1<<6)
#define NOFLSH		(1<<7)
#define TOSTOP		(1<<8)
// POSIX extension
#define ECHOCTL		(1<<9)


// IOCTL namespace
#define TTYCTL			IOCTL_NAMESPACE('T', 'T')

/**
 * Get/set the window size if available.
 */
#define TIOCGWINSZ		IOCTL_W( TTYCTL, 0x0001, struct winsize *)
#define TIOCSWINSZ		IOCTL_R( TTYCTL, 0x0002, const struct winsize *)

/**
 * Make terminal the controlling terminal of the given process, if it
 * do not have one already.
 * arg should be 0 (the value of 1 have special behavior not implemented)
 */
#define TIOCSCTTY		IOCTL_R( TTYCTL, 0x0003, int)

/**
 * Give up this terminal if it was the controlling terminal of the
 * calling process.
 */
#define TIOCNOTTY		IOCTL( TTYCTL, 0x0004)


/**
 * Get/set the foreground process group.
 * WARNING: argument is in both case a *pointer* to a pid_t variable!
 */
#define TIOCGPGRP		IOCTL_W( TTYCTL, 0x0005, __kernel_pid_t *)
#define TIOCSPGRP		IOCTL_R( TTYCTL, 0x0006, const __kernel_pid_t *)

/**
 * Get the session ID of the given terminal.
 */
#define TIOCGSID		IOCTL_W( TTYCTL, 0x0007, __kernel_pid_t *)

#endif //_FIXOS_INTERFACE_TTY_H
