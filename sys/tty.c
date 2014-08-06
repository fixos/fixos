#include "tty.h"


int tty_ioctl(struct tty *tty, int cmd, void *data) {
	switch(cmd) {
		case TIOCGWINSZ:
			return tty_getwinsize(tty, data);
			break;
		case TIOCSWINSZ:
			return tty_setwinsize(tty, data);
			break;
		case TIOCSCTTY:
			return tty_setctty(tty, (int)data);
			break;
		case TIOCNOTTY:
			return tty_noctty(tty);
			break;
		case TIOCGPGRP:
			return tty_getpgrp(tty, data);
			break;
		case TIOCSPGRP:
			return tty_setpgrp(tty, data);
			break;
		case TIOCGSID:
			return tty_getsid(tty, data);
			break;
		default:
			return -EFAULT;
	}
}
