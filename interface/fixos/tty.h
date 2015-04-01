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


/**
 * Flags for input mode (c_iflag field)
 */
#define BRKINT		(1<<0)
#define ICRNL		(1<<1)
#define IGNBRK		(1<<2)
#define IGNCR		(1<<3)
#define IGNPAR		(1<<4)
#define INLCR		(1<<5)
#define INPCK		(1<<6)
#define ISTRIP		(1<<7)
#define IXANY		(1<<8)
#define IXOFF		(1<<9)
#define IXON		(1<<10)
#define PARMRK		(1<<11)


/**
 * Baud rate (internaly set in c_cflag lower byte)
 */
#define BAUD_MASK	(0xFF<<0)
#define B0			0x00
#define B50			0x01
#define B75			0x02
#define B110		0x03
#define B134		0x04
#define B150		0x05
#define B200		0x06
#define B300		0x07
#define B600		0x08
#define B1200		0x09
#define B1800		0x0A
#define B2400		0x0B
#define B4800		0x0C
#define B9600		0x0D
#define B19200		0x0E
#define B38400		0x0F
// TODO non-POSIX, faster baud rates


/**
 * Flags for control modes (c_cflag field)
 */
#define CLOCAL		(1<<8)
#define CSIZE		(3<<9)
#define CS5			(0<<9)
#define CS6			(1<<9)
#define CS7			(2<<9)
#define CS8			(3<<9)
#define CSTOPB		(1<<11)
#define HUPCL		(1<<12)
#define PARENB		(1<<13)
#define PARODD		(1<<14)


/**
 * Flags for output mode (c_oflag field)
 * Everything is defined as XSI extension by POSIX, and not implemented.
 */
#define OPOST		(1<<0)


// IOCTL namespace
#define TTYCTL			IOCTL_NAMESPACE('T', 'T')

/**
 * Get/set the termios struct of a TTY (same as Linux ioctls)
 */
#define TCGETS			IOCTL_R( TTYCTL, 0x0008, const struct termios *)
#define TCSETS			IOCTL_W( TTYCTL, 0x0009, struct termios *)

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
