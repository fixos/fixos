#include "tty.h"

// default termios settings used
static const struct termios _tty_default_termios = {
	.c_cc = {
		ASCII_CTRL('D'),	// VEOF
		'\0',	// VEOL (\n and \r already handled)
		'\b',	// VERASE
		ASCII_CTRL('C'),	// VINTR
		ASCII_CTRL('U'),	// VKILL
		1,		// VMIN
		ASCII_CTRL('\\'),	// VQUIT
		'\0',	// VSTART
		'\0',	// VSTOP
		ASCII_CTRL('Z'),	// VSUSP
		0,		// VTIME
	},

	.c_iflag = ICRNL | IGNBRK | IGNPAR,
	.c_oflag = 0,
	.c_cflag = CLOCAL | CS8 | B9600,
	.c_lflag = ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG | ECHOCTL
};


int tty_ioctl(struct tty *tty, int cmd, void *data) {
	switch(cmd) {
		case TIOCGWINSZ:
			return tty_getwinsize(tty, data);
		case TIOCSWINSZ:
			return tty_setwinsize(tty, data);
		case TIOCSCTTY:
			return tty_setctty(tty, (int)data);
		case TIOCNOTTY:
			return tty_noctty(tty);
		case TIOCGPGRP:
			return tty_getpgrp(tty, data);
		case TIOCSPGRP:
			return tty_setpgrp(tty, data);
		case TIOCGSID:
			return tty_getsid(tty, data);
		case TCGETS:
			*(struct termios*)data = tty->termios;
			return 0;
		case TCSETS:
			return tty_set_termios(tty, data);
	}

	// this point is reached if cmd is not recognized
	if(tty->ops->ioctl != NULL)
		return tty->ops->ioctl(tty, cmd, data);
	return -EINVAL;
}


int tty_default_init(struct tty *tty) {
	tty->fifo.buffer = tty->fifo_buf;
	tty->fifo.max_size = TTY_INPUT_BUFFER;
	tty->fifo.size = 0;
	tty->fifo.top = 0;

	tty->line_pos = 0;

	INIT_WAIT_QUEUE(& tty->wqueue);

	tty->controler = 0;
	tty->fpgid = 0;

	// use an acceptable default for termios
	tty->termios = _tty_default_termios;

	return 0;
}


static void tty_echo_char(struct tty *tty, char c) {
	if(tty->termios.c_lflag & ECHOCTL && ASCII_IS_CTRL(c))
	{
		// display as ^<char>, like ^C
		tty->ops->putchar(tty, '^');

		if(c < 0x20)
			c += 0x40;
		else
			c -= 0x40;
	}
	
	tty->ops->putchar(tty, c);
}


// erase a character previsously echoed
static inline void tty_echo_erase(struct tty *tty, char c) {
	// erase with BS, SPACE and BS
	tty->ops->tty_write(tty, "\b \b", 3);

	// twice if it was an escaped ASCII char
	if(tty->termios.c_lflag & ECHOCTL && ASCII_IS_CTRL(c))
		tty->ops->tty_write(tty, "\b \b", 3);
}


int tty_input_char(struct tty *tty, char c) {
	int discard=0;
	const struct termios *ios = & tty->termios;

	// do not check character '\0', which is used to disable a control char
	if(c != '\0') {
		// two main cases where we have to check the added character
		if(ios->c_lflag & ISIG) {
			// check for generating signals
			if(c == ios->c_cc[VINTR]) {
				// SIGINT
				if(tty->fpgid != 0)
					signal_pgid_raise(tty->fpgid, SIGINT);

				discard = 1;
				tty_echo_char(tty, c);
			}
			else if(c == ios->c_cc[VQUIT]) {
				// SIGQUIT TODO
				discard = 1;
				tty_echo_char(tty, c);
			}
			else if(c == ios->c_cc[VSUSP]) {
				// SIGTSTP (FIXME instead of SIGSTOP?)
				if(tty->fpgid != 0)
					signal_pgid_raise(tty->fpgid, SIGSTOP);

				discard = 1;
				tty_echo_char(tty, c);
			}
		}

		if(ios->c_lflag & ICANON) {
			// check for canonical processing
			if(c == ios->c_cc[VERASE]) {
				// remove a single char
				discard = 1;

				if(tty->line_pos > 0) {
					tty->line_pos--;

					if(ios->c_lflag & ECHOE)
						tty_echo_erase(tty, tty->line_buf[tty->line_pos]);
				}
			}
			else if(c == ios->c_cc[VKILL]) {
				// remove the whole line
				discard = 1;
				
				// under linux, this is ECHOKE behavior (but POSIX-compliant)
				if(ios->c_lflag & ECHOK) {
					while(tty->line_pos > 0) {
						tty->line_pos--;
						tty_echo_erase(tty, tty->line_buf[tty->line_pos]);
					}
				}
			}
			else if(c == ios->c_cc[VEOF]) {
				// end the line and force return 0-count value if needed!
				// TODO
				discard = 1;
			}
			else if(c == '\n' || c == ios->c_cc[VEOL]
					|| (c == '\r'/*&& TODO check ICRNL set and IGNCR isn't*/) ) {
				// end the line (done after)
				c = '\n';
			}
			// VSTART and VSTOP not checked (useless?)
		}

		// if not discared, add the character to the line buffer
		if(!discard) {
			if(tty->line_pos + 1 < TTY_LINE_BUFFER) {
				int flush_line = 0;

				tty->line_buf[tty->line_pos] = c;
				tty->line_pos++;

				if(ios->c_lflag & ECHO)
					tty_echo_char(tty, c);

				// determine if line buffer contains a valid input
				if(ios->c_lflag & ICANON) {
					// after a newline
					if(c == '\n') {
						flush_line = 1;
						// NL should be echoed if ECHONL is set
						if(!(ios->c_lflag & ECHO) && ios->c_lflag & ECHONL)
							tty_echo_char(tty, c);
					}
				}
				else {
					// should check conditions on VMIN and VTIME
					// FIXME VTIME not handled at all!
					if(tty->line_pos >= ios->c_cc[VMIN])
						flush_line = 1;
				}

				if(flush_line == 1) {
					// add the line buffer to the input one, wakeup waiters
					cfifo_push(& tty->fifo, tty->line_buf, tty->line_pos);
					tty->line_pos = 0;
					wqueue_wakeup(& tty->wqueue);
				}
			}
		}
	}

	return 0;
}


int tty_read(struct tty *tty, char *dest, size_t len) {
	// TODO atomic fifo access
	int curlen = 0;
	volatile size_t *fifo_size = &(tty->fifo.size);

	// FIXME check behavior : should return the first line available
	//while(curlen < len) {
	size_t readlen;

	wait_until_condition(& tty->wqueue, (readlen=*fifo_size) > 0);

	// at least 1 byte available in the FIFO
	readlen = readlen + curlen > len ? len - curlen : readlen;
	cfifo_pop(& tty->fifo, dest, readlen);
	curlen += readlen;
	//}

	return curlen;
}
