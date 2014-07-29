#ifndef _FIXOS_INTERFACE_TTY_H
#define _FIXOS_INTERFACE_TTY_H

/**
 * Data structure and ioctls related to TTY control.
 */

#include "ioctl.h"


struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	// no ws_xpixel/ws_ypixel ?
};


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
#define TIOCGPGRP		IOCTL_W( TTYCTL, 0x0005, pid_t *)
#define TIOCSPGRP		IOCTL_R( TTYCTL, 0x0006, const pid_t *)

/**
 * Get the session ID of the given terminal.
 */
#define TIOCGSID		IOCTL_W( TTYCTL, 0x0007, pid_t *)

#endif //_FIXOS_INTERFACE_TTY_H
