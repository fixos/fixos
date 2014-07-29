#ifndef _SYS_TTY_H
#define _SYS_TTY_H

/**
 * TTY data structure, used by device driver implementing a TTY, and by
 * process to point to the controlling terminal.
 */

#include <utils/types.h>
#include <interface/tty.h>

struct tty {
	// pid of the process that control the given TTY (should be a session leader,
	// but sessions are not implemented currently)
	// 0 should be used if no process control the given terminal
	pid_t controler;
	// foreground process group
	pid_t fpgid;

	// private data, device-dependant information
	void *private;
};

#endif //_SYS_TTY_H
