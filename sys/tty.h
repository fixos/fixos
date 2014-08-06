#ifndef _SYS_TTY_H
#define _SYS_TTY_H

/**
 * TTY data structure, used by device driver implementing a TTY, and by
 * process to point to the controlling terminal.
 */

#include <utils/types.h>
#include <interface/tty.h>
#include <sys/process.h>
#include <interface/errno.h>

struct tty_ops;

struct tty {
	// pid of the process that control the given TTY (should be a session leader,
	// but sessions are not implemented currently)
	// 0 should be used if no process control the given terminal
	pid_t controler;
	// foreground process group
	pid_t fpgid;

	// tty-specific operations
	const struct tty_ops *ops;

	// private data, device-dependant information
	void *private;
};


struct tty_ops {
	int (*is_ready) (struct tty *tty);
	int (*tty_write) (struct tty *tty, const char *data, size_t len);
	
	// ioctl support
	int (*ioctl_setwinsize)(struct tty *tty, const struct winsize *size);
	int (*ioctl_getwinsize)(struct tty *tty, struct winsize *size);
};


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


// used to dispatch tty-level ioctls using tty->ops and tty data
// return special value -EFAULT if ioctl is not tty-level command
int tty_ioctl(struct tty *tty, int cmd, void *data);

#endif //_SYS_TTY_H
