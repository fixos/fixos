#include "virtual_term.h"
#include <utils/cyclic_fifo.h>
#include <fs/file_operations.h>
#include <sys/waitqueue.h>
#include <interface/fixos/errno.h>
#include <sys/tty.h>
#include <sys/process.h>
#include "text_display.h"

#include <device/terminal/fx9860/text_display.h>

#include <utils/log.h>


#define VT_INPUT_BUFFER		256
#define VT_LINE_BUFFER		128

#define VT_100_NO_ESCAPE_CODE 0 
#define VT_100_ESCAPE_CHARACTER_FOUND 1 
#define VT_100_PARSING_ESCAPE_CODE 2 

// define character that must be written as "^<char>" escaped form, like ^C
#define IS_ESC_CTRL(c) \
	((c)>=0 && (c)<0x20 && (c)!='\n')

struct vt_instance {
	struct tdisp_data disp;
	int posx;
	int posy;

	// for input control :
	char fifo_buf[VT_INPUT_BUFFER];
	struct cyclic_fifo fifo;

	char line_buf[VT_LINE_BUFFER];
	int line_pos;

	struct wait_queue wqueue;

	struct tty tty;
};


static struct vt_instance _vts[VT_MAX_TERMINALS];
static int _vt_current;

static struct tty *vt_get_tty(uint16 minor);

const struct device virtual_term_device = {
	.name = "v-term",
	.init = &vt_init,
	.open = &vt_open,
	.get_tty = &vt_get_tty
};


static const struct file_operations _vt_fop = {
	.release = &vt_release,
	.write = &vt_write,
	.read = &vt_read,
	.ioctl = &vt_ioctl
};


static const struct text_display *_tdisp = &fx9860_text_display;


static int vt_ioctl_getwinsize(struct tty *tty, struct winsize *size);

static int vt_ioctl_setwinsize(struct tty *tty, const struct winsize *size);

static int vt_tty_is_ready(struct tty *tty) {
	(void)tty;
	return 1;
}

static int vt_tty_write(struct tty *tty, const char *data, size_t len);

static const struct tty_ops _vt_tty_ops = {
	.ioctl_setwinsize = &vt_ioctl_setwinsize,
	.ioctl_getwinsize = &vt_ioctl_getwinsize,
	.is_ready = &vt_tty_is_ready,
	.tty_write = &vt_tty_write
};


void vt_init() {
	int i;

	_vt_current = -1;

	for(i=0; i<VT_MAX_TERMINALS; i++) {
		_tdisp->init_disp(& _vts[i].disp);

		_vts[i].fifo.buffer = _vts[i].fifo_buf;
		_vts[i].fifo.max_size = VT_INPUT_BUFFER;
		_vts[i].fifo.size = 0;
		_vts[i].fifo.top = 0;

		_vts[i].line_pos = 0;

		_vts[i].posx = 0;
		_vts[i].posy = 0;

		INIT_WAIT_QUEUE(& _vts[i].wqueue);

		_vts[i].tty.controler = 0;
		_vts[i].tty.fpgid = 0;
		_vts[i].tty.private = & _vts[i];
		_vts[i].tty.ops = &_vt_tty_ops;
	}
}

// TODO export these static variables to the terminal structure
static unsigned char vt100_discovery_state = VT_100_NO_ESCAPE_CODE;
static unsigned char buffer[8] = {0};
static unsigned char buffer_index = 0;

static unsigned char using_arguments = 0;
static uint32 arguments[2] = {0};
static uint32 arguments_index = 0;

static void vt_clear_escape_code(struct vt_instance *term) {
	int i;
	vt100_discovery_state = VT_100_NO_ESCAPE_CODE;
	for(i = 0; i < 8; i++)
		// TODO print char
		buffer[i] = 0;

	buffer_index = 0;

	using_arguments = 0;
	for(i = 0; i < 2; i++)
		arguments[i] = 0;
	arguments_index = 0;

}

static int vt_parse_simple_escape_code(struct vt_instance *term, char str_char) {
	switch(str_char) {
		// Simple escape codes
		case 'c':
		// TODO reset terminal setup
		break;
		case '(':
		// TODO Set default font
		break;
		case ')':
		// TODO Set alternative font
		break;
		case '7':
		// TODO Save position and attributes(?). NO arguments
		break;
		case '8':
		// TODO Restores position and attributes(?). NO arguments
		break;
		case 'D':
		// TODO Scroll down display one line
		break;
		case 'M':
		// TODO Scroll up display one line
		break;
		case 'H':
		// TODO Set tab at this position (?)
		break;
		case '[':
			buffer[buffer_index] = str_char;
			buffer_index ++;
			return 0;
		break;
	}
	return 1;
}

static void vt_parse_escape_code(struct vt_instance *term, char str_char) {
	switch(str_char) {
		case 'n':
		// TODO Device status related
		break;
		case 'h':
		// TODO Line Wrap. Beware, the first argument is always a 7
		break;
		case 'H':
		case 'f':
		// TODO Cursor Home. IF both arguments are given, go to position or go to cursor home
		break;
		case 'A':
		// TODO Cursor up. THe argument is optionnal (default : 1)			
		break;
		case 'B':
		// TODO Cursor down. THe argument is optionnal (default : 1)			
		break;
		case 'C':
		// TODO Cursor right. THe argument is optionnal (default : 1)			
		break;
		case 'D':
		// TODO Cursor left. THe argument is optionnal (default : 1)			
		break;
		case 's':
		// TODO Save position. NO arguments
		break;
		case 'u':
		// TODO Restores position. NO arguments
		break;
		case 'r':
		// TODO Enable scrolling for the whole screen. If two arguments are given (start;end), enable scrolling from {start} to {end}
		break;
		case 'g':
		// TODO Clear tab at this position. If an argument is given and == 3, clear all tabs
		break;
		case 'K':
		// TODO Clear the line from the position to the end. If an argument is given and == 1, clear to the begin instead.
		// If an argument is given and == 1, clear the whole line instead.			
		break;
		case 'J':
		// TODO Erase the screen down to the bottom from the current line. If an argument is given and == 1, erase up to the top instead.
		// If an argument is given and == 2, clear the whole screen with the background color and go to cursor home.			
		break;
		case 'p':
		// TODO Bind a string to a keyboard key
		break;
		case 'm':
		// TODO COLORS ANd ATTRIBUTES
		break;
	}	
}

static void vt_read_escape_code(struct vt_instance *term, char str_char) {
	if(vt100_discovery_state == VT_100_ESCAPE_CHARACTER_FOUND) {
		// Avoid parsing <ESC>
		vt100_discovery_state = VT_100_PARSING_ESCAPE_CODE;
		return;
	}
	if(buffer_index == 1 && vt_parse_simple_escape_code(term, str_char) == 1) {
			vt_clear_escape_code(term);
			return;
	}
	// We're on the bigger escapes codes
	if((str_char >= '0' && str_char <= '9') || str_char == ';') {
		// We're adding the arguments
		buffer[buffer_index] = str_char;

		using_arguments = 1;

		if(str_char == ';') {
			arguments_index++;
			if(arguments_index == VT_100_PARSING_ESCAPE_CODE) {
				vt_clear_escape_code(term);
				return;
			}
		} else {
			// Setting the digit to arguments
			arguments[arguments_index]*=10;
			arguments[arguments_index] += str_char - '0';
		}
	}
	else if((str_char >= 'a' && str_char <= 'z') || (str_char >= 'A' && str_char <= 'Z')) {
		// If we have reached the end of an escape code
		vt_parse_escape_code(term, str_char);
		vt_clear_escape_code(term);
	}
	if(++buffer_index >= 8);
		vt_clear_escape_code(term);
}

// function used to print characters
static void vt_term_print(struct vt_instance *term, const void *source, size_t len) {
	// TODO future extensions to support more VT100-like escape codes
	

	int i;
	const unsigned char *str = source;

	for(i=0; i<len; i++) {
		if(str[i] == 0x1B) {
			if(vt100_discovery_state == 0) {
				vt100_discovery_state = VT_100_ESCAPE_CHARACTER_FOUND;
				buffer[0] = str[i];
				buffer_index = 1;
			}
		}
		else if(str[i] == '\n') {
			// remove the current cursor display before line feed
			_tdisp->print_char(& term->disp, term->posx, term->posy, ' ');
			term->posx = 0;
			term->posy++;
		}
		else if(str[i] == '\r') term->posx=0;
		else {
			_tdisp->print_char(& term->disp, term->posx, term->posy, str[i]);
			term->posx++;
		}
		if(vt100_discovery_state == VT_100_NO_ESCAPE_CODE) {
			// We aren't in a vt100 escape code
			if(term->posx >= _tdisp->cwidth) {
				term->posx = 0;
				term->posy++;
			}

			
			if(term->posy >= _tdisp->cheight) {
				_tdisp->scroll(&term->disp);
				term->posy = _tdisp->cheight - 1;
			}			
		}
		else {
			vt_read_escape_code(term, str[i]);
		}
	}

	// print cursor at current position
	_tdisp->print_char(& term->disp, term->posx, term->posy, VT_CURSOR_CHAR);

	// TODO flush only if active!
	//disp_mono_copy_to_dd(_term_vram);
}



// remove the previous echoed character from the display
static void vt_unwind_echo_char(struct vt_instance *term) {
	_tdisp->print_char(& term->disp, term->posx, term->posy, ' ');

	term->posx--;
	if(term->posx < 0) {
		term->posx = _tdisp->cwidth - 1;
		term->posy--;
	}

	// print cursor at current position
	_tdisp->print_char(& term->disp, term->posx, term->posy,
			VT_CURSOR_CHAR);

	_tdisp->flush(& term->disp);
}


// add the given character to line buffer and echo it if needed
static void vt_add_character(struct vt_instance *term, char c) {
	term->line_buf[term->line_pos] = c;
	term->line_pos++;

	// basic echo, need to be improved (do not copy_to_dd() each time...)
	if(IS_ESC_CTRL(c)) {
		char esc[2] = {'^', ASCII_UNCTRL(c)};
		vt_term_print(term, esc, 2);
	}
	else {
		vt_term_print(term, &c, 1);
	}
	_tdisp->flush(& term->disp);
}


// do special action for character < 0x20 (special ASCII chars)
static void vt_do_special(struct vt_instance *term, char spe) {
	/*
	char str[3] = {'^', ' ', '\0'};
	str[1] = ASCII_UNCTRL(spe);
	printk("tty: received %s, pgid=%d\n", str, term->tty.fpgid);
	*/
	char spestr[2] = {'^', ASCII_UNCTRL(spe)};

	switch(spe) {
		case ASCII_CTRL('C'):
			// kill group
			if(term->tty.fpgid != 0)
				signal_pgid_raise(term->tty.fpgid, SIGINT);
			vt_term_print(term, spestr, 2);
			_tdisp->flush(& term->disp);
			break;

		case ASCII_CTRL('Z'):
			// stop foreground
			if(term->tty.fpgid != 0)
				signal_pgid_raise(term->tty.fpgid, SIGSTOP);
			vt_term_print(term, spestr, 2);
			_tdisp->flush(& term->disp);
			break;

		case '\n':
			vt_add_character(term, '\n');
			cfifo_push(& term->fifo, term->line_buf, term->line_pos);
			term->line_pos = 0;
			wqueue_wakeup(& term->wqueue);
			break;

		case ASCII_CTRL('H'):
			// backspace, remove last buffered char and echo space instead
			if(term->line_pos > 0)  {
				char removed = term->line_buf[term->line_pos-1];

				// removed 2 characters if it was displayed as "^<char>"
				if(IS_ESC_CTRL(removed))
					vt_unwind_echo_char(term);

				vt_unwind_echo_char(term);
				term->line_pos--;
			}
			break;

		default:
			vt_add_character(term, spe);
	}
}


// callback function called when a key is stroke
void vt_key_stroke(int code) {

	// all keyboard input are 'redirected' to the current active terminal, if any
	if(_vt_current >= 0 && _vt_current <VT_MAX_TERMINALS) {
		struct vt_instance *term;

		term = & _vts[_vt_current];

		// check the line buffer, and add the char to it if possible
		if(term->line_pos < VT_LINE_BUFFER || code == 0x08) {
			if(code < 0x20) {
				// special character (should be improved)
				vt_do_special(term, (char)code);	
			}
			else if(code < 0x80) {
				vt_add_character(term, (char)code);
			}
		}
	}
}


void vt_set_active(int term) {
	if(term >= -1 && term < VT_MAX_TERMINALS) {
		if(term != _vt_current) {
			_tdisp->set_active(& _vts[_vt_current].disp, 0);
			if(term != -1) {
				_tdisp->set_active(& _vts[term].disp, 1);
			}
			_vt_current = term;
		}
	}
}



int vt_open(uint16 minor, struct file *filep) {
	if(minor < VT_MAX_TERMINALS) {
		filep->op = &_vt_fop;
		filep->private_data = (void*)((int)minor);
		return 0;
	}

	return -ENXIO;
}


static struct tty *vt_get_tty(uint16 minor) {
	if(minor < VT_MAX_TERMINALS) {
		return &_vts[minor].tty;
	}
	return NULL;
}


static ssize_t vt_prim_write(struct vt_instance *term, const void *source, size_t len) {
	vt_term_print(term, source, len);

	// screen should be flushed only if it's the current active
	if(term == &_vts[_vt_current])
		_tdisp->flush(&term->disp);

	return len;
}


static int vt_tty_write(struct tty *tty, const char *data, size_t len) {
	return vt_prim_write(tty->private, data, len);
}


ssize_t vt_write(struct file *filep, void *source, size_t len) {
	int term;
	
	term = (int)(filep->private_data);
	if(term >= 0 && term <VT_MAX_TERMINALS) {
		return vt_prim_write(&_vts[term], source, len);
	}
	return -EINVAL;
}


ssize_t vt_read(struct file *filep, void *dest, size_t len) {
	int term;
	
	term = (int)(filep->private_data);
	if(term >= 0 && term <VT_MAX_TERMINALS) {
		// TODO atomic fifo access
		int curlen = 0;
		volatile size_t *fifo_size = &(_vts[term].fifo.size);

		while(curlen < len) {
			size_t readlen;

			wait_until_condition(& _vts[term].wqueue, (readlen=*fifo_size) > 0);

			// at least 1 byte available in the FIFO
			readlen = readlen + curlen > len ? len - curlen : readlen;
			cfifo_pop(& _vts[term].fifo, dest, readlen);
			curlen += readlen;
		}
		
		return curlen;
	}

	return -EINVAL;
}


int vt_release(struct file *filep) {
	return 0;
}


static int vt_ioctl_getwinsize(struct tty *tty, struct winsize *size) {
	(void)tty;
	if(size == NULL)
		return -EINVAL;
	size->ws_col = _tdisp->cwidth;
	size->ws_row = _tdisp->cheight;
	return 0;
}


static int vt_ioctl_setwinsize(struct tty *tty, const struct winsize *size) {
	(void)tty;
	(void)size;
	return -EINVAL;
}


int vt_ioctl(struct file *filep, int cmd, void *data) {
	int term;

	term = (int)(filep->private_data);
	if(term >= 0 && term <VT_MAX_TERMINALS) {
		int ret;
		ret = tty_ioctl(&_vts[term].tty, cmd, data);
		if(ret == -EFAULT) {
			// device-level ioctl command, if any
		}
		else {
			return ret;
		}
	}
	return -EINVAL;
}

