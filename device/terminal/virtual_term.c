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


// for VT100 escape code handling :

// not in an escape sequence
#define VT100_STATE_NONE				0
// in a simple escape (after a "^[" )
#define VT100_STATE_SIMPLE_ESCAPE		1
// in a CSI escape sequence ( "^[ [ ..." )
#define VT100_STATE_CSI_ESCAPE			2
// in a CSI with question mark sequence ( "^[ [ ? ...")
#define VT100_STATE_CSI_QM_ESCAPE		3

// maximum of integer arguments acceptable in VT100 parser
#define VT100_MAX_INTARGS				4


// define character that must be written as "^<char>" escaped form, like ^C
#define IS_ESC_CTRL(c) \
	((c)>=0 && (c)<0x20 && (c)!='\n')


struct vt100_esc_state {
	// keep current parser state
	unsigned char discovery_state;

	// integer arguments, using short but it seems a byte should be enough?
	unsigned short arguments[VT100_MAX_INTARGS];
	// current integer argument, using -1 if no argument is used for now
	short arguments_index;
};

struct vt_instance {
	struct tdisp_data disp;
	int posx;
	int posy;
	int saved_posx;
	int saved_posy;

	// for input control :
	char fifo_buf[VT_INPUT_BUFFER];
	struct cyclic_fifo fifo;

	char line_buf[VT_LINE_BUFFER];
	int line_pos;

	struct wait_queue wqueue;

	struct tty tty;

	struct vt100_esc_state esc_state;
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


/**
 * Clear the given VT escape parser data to get a clean state again.
 */
static void vt_clear_escape_code(struct vt_instance *term) {
	int i;
	term->esc_state.discovery_state = VT100_STATE_NONE;

	for(i = 0; i < VT100_MAX_INTARGS; i++)
		term->esc_state.arguments[i] = 0;
	term->esc_state.arguments_index = -1;
}


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
		_vts[i].saved_posx = 0;
		_vts[i].saved_posy = 0;

		INIT_WAIT_QUEUE(& _vts[i].wqueue);

		_vts[i].tty.controler = 0;
		_vts[i].tty.fpgid = 0;
		_vts[i].tty.private = & _vts[i];
		_vts[i].tty.ops = &_vt_tty_ops;

		vt_clear_escape_code(_vts + i);

	}
}



/**
 * Move the cursor from its current position to (newx, newy), after checking
 * screen boundaries.
 * TODO the cursor should be implemented in terminal_disp directly to avoid
 * overwriting characters when moving it using VT100 sequences!
 */
static void vt_move_cursor(struct vt_instance *term, int newx, int newy) {
	_tdisp->print_char(& term->disp, term->posx, term->posy, ' ');

	term->posx = newx > _tdisp->cwidth-1 ? _tdisp->cwidth-1
		: (newx < 0 ? 0 : newx);
	term->posy = newy > _tdisp->cheight-1 ? _tdisp->cheight-1
		: (newy < 0 ? 0 : newy);

	// print cursor at current position
	_tdisp->print_char(& term->disp, term->posx, term->posy,
			VT_CURSOR_CHAR);

	_tdisp->flush(& term->disp);
}

/**
 * Check for the 1-character ANSI/VT100 escape codes (like "^[ c"), and return
 * the next state of the parser.
 */
static int vt_parse_simple_escape_code(struct vt_instance *term, char str_char) {
	int ret = VT100_STATE_NONE;

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
		term->saved_posx = term->posx;
		term->saved_posy = term->posy;
		break;
	case '8':
		// TODO Restores position and attributes(?). NO arguments
		vt_move_cursor(term, term->saved_posx, term->saved_posy);
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
		ret = VT100_STATE_CSI_ESCAPE;
		break;
	default:
		// parsing error, get back into the not-escape state
		ret = VT100_STATE_NONE;
	}

	return ret;
}


/**
 * Parse all subsequent characters of an escape code, one by one, and return
 * each time the next state of the VT100 escape sequences parser.
 */
static int vt_parse_escape_code(struct vt_instance *term, char str_char) {
	int ret = VT100_STATE_NONE;
	int tmp;

	switch(str_char) {
	case 'n':
		// TODO Device status related
		break;

	case 'h':
		// TODO Line Wrap. Beware, the first argument is always a 7
		if(term->esc_state.arguments_index != -1 && term->esc_state.arguments[0] == 7) {
			// here
		}
		break;

	case 'H':
	case 'f':
		// in ANSI escape sequences, position 1;1 is the top-left corner
		if(term->esc_state.arguments_index >= 0) {
			vt_move_cursor(term, term->esc_state.arguments[0]-1,
					term->esc_state.arguments[1]-1);
		}
		else {
			vt_move_cursor(term, 0, 0);
		}
		break;

	case 'A':
		// Cursor up
		tmp = term->esc_state.arguments_index >= 0 ? term->esc_state.arguments[0] : 1;
		vt_move_cursor(term, term->posx, term->posy - tmp);
		break;

	case 'B':
		// Cursor down
		tmp = term->esc_state.arguments_index >= 0 ? term->esc_state.arguments[0] : 1;
		vt_move_cursor(term, term->posx, term->posy + tmp);
		break;

	case 'C':
		// Cursor left
		tmp = term->esc_state.arguments_index >= 0 ? term->esc_state.arguments[0] : 1;
		vt_move_cursor(term, term->posx - tmp, term->posy);
		break;

	case 'D':
		// Cursor right
		tmp = term->esc_state.arguments_index >= 0 ? term->esc_state.arguments[0] : 1;
		vt_move_cursor(term, term->posx + tmp, term->posy);
		break;

	case 's':
		// Save cursor position.
		term->saved_posx = term->posx;
		term->saved_posy = term->posy;
		break;

	case 'u':
		// Restores cursor position.
		vt_move_cursor(term, term->saved_posx, term->saved_posy);
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
		// TODO Colors and attributes
		break;
	
	default:
		// bad escape sequence
		ret = VT100_STATE_NONE;
	}

	return ret;
}



static void vt_read_escape_code(struct vt_instance *term, char str_char) {
	int newstate = VT100_STATE_NONE;

	switch(term->esc_state.discovery_state) {
	case VT100_STATE_NONE:
		if(str_char == '\x1B') 
			newstate = VT100_STATE_SIMPLE_ESCAPE;
		break;

	case VT100_STATE_SIMPLE_ESCAPE:
		newstate = vt_parse_simple_escape_code(term, str_char);
		break;
	
	case VT100_STATE_CSI_ESCAPE:
		// parsing a "^[ [ ..." escape code
		if((str_char >= '0' && str_char <= '9') || str_char == ';') {
			newstate = VT100_STATE_CSI_ESCAPE;
			// We're adding the arguments
			if(term->esc_state.arguments_index < 0)
				term->esc_state.arguments_index = 0;

			if(str_char == ';') {
				term->esc_state.arguments_index++;
				// check if there are too much arguments for our simple parser
				if(term->esc_state.arguments_index == VT100_MAX_INTARGS) {
					vt_clear_escape_code(term);
					newstate = VT100_STATE_NONE;
				}
			} else {
				// Setting the digit to arguments
				term->esc_state.arguments[term->esc_state.arguments_index] *= 10;
				term->esc_state.arguments[term->esc_state.arguments_index] += str_char - '0';
			}
		}
		else {
			// maybe we reached the end of an escape code
			newstate = vt_parse_escape_code(term, str_char);
		}
		break;

	case VT100_STATE_CSI_QM_ESCAPE:
		// TODO (mainly the sequences "^[ [ ? 2 5 l" for hiding cursor)
		break;
	}

	term->esc_state.discovery_state = newstate;
}

/**
 * Function used to print characters (as well written to term and echoed input from
 * keyboard).
 * If mayesc is not-zero, try to check VT100-like escape sequences from source data.
 */
static void vt_term_print(struct vt_instance *term, const void *source, size_t len,
		int mayesc)
{
	int i;
	const unsigned char *str = source;

	for(i=0; i<len; i++) {
		if(!mayesc || (term->esc_state.discovery_state == VT100_STATE_NONE && str[i] != '\x1B')) {
			// We aren't in a vt100 escape code
			if(str[i] == '\n') {
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
			// parse the character as a part of a VT100-like escape sequence
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
		vt_term_print(term, esc, 2, 0);
	}
	else {
		vt_term_print(term, &c, 1, 0);
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
			vt_term_print(term, spestr, 2, 0);
			_tdisp->flush(& term->disp);
			break;

		case ASCII_CTRL('Z'):
			// stop foreground
			if(term->tty.fpgid != 0)
				signal_pgid_raise(term->tty.fpgid, SIGSTOP);
			vt_term_print(term, spestr, 2, 0);
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
	vt_term_print(term, source, len, 1);

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

