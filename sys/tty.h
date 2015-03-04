#ifndef _SYS_TTY_H
#define _SYS_TTY_H

/**
 * TTY data structure, used by device driver implementing a TTY, and by
 * process to point to the controlling terminal.
 */

#include <utils/types.h>
#include <interface/fixos/tty.h>
#include <sys/process.h>
#include <interface/fixos/errno.h>
#include <utils/cyclic_fifo.h>
#include <sys/waitqueue.h>


struct tty_ops;

// size of the input buffers
#define TTY_INPUT_BUFFER	256
#define TTY_LINE_BUFFER		128

struct tty {
	// pid of the process that control the given TTY (should be a session leader,
	// but sessions are not implemented currently)
	// 0 should be used if no process control the given terminal
	pid_t controler;
	// foreground process group
	pid_t fpgid;

	// terminal I/O control settings
	struct termios termios;

	// input buffering with cyclic buffer
	char fifo_buf[TTY_INPUT_BUFFER];
	struct cyclic_fifo fifo;

	// for line edition (canonical mode...)
	char line_buf[TTY_LINE_BUFFER];
	int line_pos;

	// wait queue used to wait for available input
	struct wait_queue wqueue;

	// tty-specific operations
	const struct tty_ops *ops;

	// private data, device-dependant information
	void *private;
};


struct tty_ops {
	int (*is_ready) (struct tty *tty);
	int (*tty_write) (struct tty *tty, const char *data, size_t len);

	// read/write a single character
	int (*getchar) (struct tty *tty);
	int (*putchar) (struct tty *tty, char c);
	
	// ioctl support
	int (*ioctl_setwinsize)(struct tty *tty, const struct winsize *size);
	int (*ioctl_getwinsize)(struct tty *tty, struct winsize *size);

	/**
	 * Called when termios value should be changed.
	 * If non-null, this callback should check and *set* the accepted value
	 * in tty->termios before returning.
	 * Partially accepted change behavior is implementation defined.
	 */
	int (*set_termios)(struct tty *tty, const struct termios *queried);

	// last chance to flush output from a TTY (e.g. after a oops)
	int (*force_flush)(struct tty *tty);
};


/**
 * Initialize given tty structure with default values.
 */
int tty_default_init(struct tty *tty);


extern inline int tty_is_ready(struct tty *tty) {
	if(tty->ops->is_ready == NULL)
		return 1;
	return tty->ops->is_ready(tty);
}

extern inline int tty_write(struct tty *tty, const char *data, size_t len) {
	if(tty->ops->tty_write == NULL)
		return -EIO;
	return tty->ops->tty_write(tty, data, len);
}

int tty_read(struct tty *tty, char *dest, size_t len);


extern inline int tty_getwinsize(struct tty *tty, struct winsize *size) {
	if(tty->ops->ioctl_getwinsize == NULL) {
		size->ws_col = 80;
		size->ws_row = 24;
		return 0;
	}
	return tty->ops->ioctl_getwinsize(tty, size);
}

extern inline int tty_setwinsize(struct tty *tty, const struct winsize *size) {
	if(tty->ops->ioctl_setwinsize == NULL)
		return -EINVAL;
	return tty->ops->ioctl_setwinsize(tty, size);
}


extern inline int tty_setctty(struct tty *tty, int arg) {
	(void)arg;
	if(_proc_current->ctty != NULL)
		return -EINVAL;

	_proc_current->ctty = tty;
	if(tty->controler == 0)
		tty->controler = _proc_current->pid;
	return 0;
}


extern inline int tty_noctty(struct tty *tty) {
	if(_proc_current->ctty != tty)
		return -EINVAL;

	if(_proc_current->pid == tty->controler) {
		// FIXME session leader give up the terminal!
		tty->controler = 0;
		tty->fpgid = 0;
	}
	_proc_current->ctty = NULL;
	return 0;
}

extern inline int tty_setpgrp(struct tty *tty, const pid_t *pid) {
	if(pid == NULL)
		return -EINVAL;

	tty->fpgid = *pid;
	return 0;
}

extern inline int tty_getpgrp(struct tty *tty, pid_t *pid) {
	if(pid == NULL)
		return -EINVAL;

	*pid = tty->fpgid;
	return 0;
}

extern inline int tty_getsid(struct tty *tty, pid_t *sid) {
	if(sid == NULL)
		return -EINVAL;

	*sid = tty->controler;
	return 0;
}


extern inline int tty_set_termios(struct tty *tty, const struct termios *ios)
{
	if(tty->ops->set_termios != NULL) {
		return tty->ops->set_termios(tty, ios);
	}
	tty->termios = *ios;
	return 0;
}


/**
 * Used to dispatch tty-level ioctls using tty->ops and tty data.
 * return special value -EFAULT if ioctl is not tty-level command
 */
int tty_ioctl(struct tty *tty, int cmd, void *data);


/**
 * Add a character to given tty input, and do appropriate job with  it
 * (canonical mode, echo, signal generation...) depending termios values.
 */
int tty_input_char(struct tty *tty, char c);


/**
 * Try to flush the output buffer of the given TTY (to be visible from user).
 * Should be as simple as possible, because it may be called from unstable
 * states, as a last chance to show something to the user.
 */
extern inline int tty_force_flush(struct tty *tty) {
	if(tty->ops->force_flush != NULL)
		return tty->ops->force_flush(tty);
	return -EIO;
}

#endif //_SYS_TTY_H
