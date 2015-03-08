#include "virtual_term.h"
#include <fs/file_operations.h>
#include <interface/fixos/errno.h>
#include <sys/tty.h>
#include <sys/process.h>
#include "text_display.h"
#include <sys/stimer.h>
#include <sys/time.h>

#include <device/tty.h>
#include <device/terminal/fx9860/text_display.h>

#include <utils/log.h>


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

	struct tty tty;

	struct vt100_esc_state esc_state;
};


static struct vt_instance _vts[VT_MAX_TERMINALS];
static int _vt_current;


static const struct text_display *_tdisp = &fx9860_text_display;

static int vt_ioctl_getwinsize(struct tty *tty, struct winsize *size);

static int vt_ioctl_setwinsize(struct tty *tty, const struct winsize *size);

static int vt_tty_is_ready(struct tty *tty) {
	(void)tty;
	return 1;
}

static int vt_tty_putchar(struct tty *tty, char c);

static int vt_tty_write(struct tty *tty, const char *data, size_t len);

static int vt_tty_force_flush(struct tty *tty);

static const struct tty_ops _vt_tty_ops = {
	.ioctl_setwinsize = &vt_ioctl_setwinsize,
	.ioctl_getwinsize = &vt_ioctl_getwinsize,
	.is_ready = &vt_tty_is_ready,
	.tty_write = &vt_tty_write,
	.putchar = &vt_tty_putchar,
	.force_flush = &vt_tty_force_flush
};


// soft timer used to flush the display (VRAM to display)
static void vt_flush_display(void *data);

static int _vt_flush_delayed = 0;



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

		_vts[i].posx = 0;
		_vts[i].posy = 0;
		_vts[i].saved_posx = 0;
		_vts[i].saved_posy = 0;

		vt_clear_escape_code(_vts + i);

		// set TTY default settings, and add it to TTY device
		tty_default_init(&_vts[i].tty);
		_vts[i].tty.private = & _vts[i];
		_vts[i].tty.ops = &_vt_tty_ops;

		ttydev_set_minor(VT_MINOR_BASE + i, &_vts[i].tty);
	}
}


/**
 * Soft timer used to display the screen only once per tick if needed.
 * This allow minimal drawing without catastrophic counterpart.
 */
static void vt_flush_display(void *data) {
	(void)data;
	if(_vt_current >= 0 && _vt_current < VT_MAX_TERMINALS) {
		_tdisp->flush(& _vts[_vt_current].disp);
	}
	_vt_flush_delayed = 0;
}


// use a delay of more or less 20 ms after the first call
static void vt_delay_flush() {
	if(!_vt_flush_delayed) {
		_vt_flush_delayed = 1;
		stimer_add(&vt_flush_display, NULL, TICKS_MSEC_NOTNULL(20));
	}
}


// direct flush without waiting delayed flushing
static int vt_tty_force_flush(struct tty *tty) {
	_tdisp->flush(& ((struct vt_instance*)tty->private)->disp);
	return 0;
}


/**
 * Move the cursor from its current position to (newx, newy), after checking
 * screen boundaries.
 */
static void vt_move_cursor(struct vt_instance *term, int newx, int newy) {
	term->posx = newx > _tdisp->cwidth-1 ? _tdisp->cwidth-1
		: (newx < 0 ? 0 : newx);
	term->posy = newy > _tdisp->cheight-1 ? _tdisp->cheight-1
		: (newy < 0 ? 0 : newy);

	// print cursor at current position
	_tdisp->set_cursor_pos(& term->disp, term->posx, term->posy);

	vt_delay_flush();
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
		if(term->esc_state.arguments_index >= 0 
				&& term->esc_state.arguments[0] == 2)
		{
			// If an argument is given and == 2, clear the whole screen with the
			// background color and go to cursor home.
			_tdisp->clear(& term->disp);
			vt_move_cursor(term, 0, 0);
			vt_delay_flush();
		}
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

	if(newstate == VT100_STATE_NONE)
		vt_clear_escape_code(term);
	term->esc_state.discovery_state = newstate;
}


/**
 * Echo a character to the screen, without flushing to display driver
 */
static void vt_echo_char(struct vt_instance *term, char c) {
	if((term->esc_state.discovery_state == VT100_STATE_NONE && c != '\x1B')) {
		// We aren't in a vt100 escape code
		if(c == '\b') {
			// backspace, should only go back (non destructive)
			term->posx--;
			if(term->posx < 0) {
				// this behavior is not POSIX, but (very?) useful
				term->posx = _tdisp->cwidth - 1;
				term->posy--;
			}
		}
		else if(c == '\n') {
			// remove the current cursor display before line feed
			term->posx = 0;
			term->posy++;
		}
		else if(c == '\r') term->posx=0;
		else {
			// any other character should just be displayed
			_tdisp->print_char(& term->disp, term->posx, term->posy, c);
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

		_tdisp->set_cursor_pos(& term->disp, term->posx, term->posy);
	}
	else {
		// parse the character as a part of a VT100-like escape sequence
		vt_read_escape_code(term, c);
	}
}

/**
 * Function used to print characters (as well written to term and echoed input
 * from keyboard).
 */
static void vt_term_print(struct vt_instance *term, const void *source,
		size_t len)
{
	int i;
	const unsigned char *str = source;

	for(i=0; i<len; i++) {
		vt_echo_char(term, str[i]);
	}
}



// called by TTY driver to display a single character, be kind and flush disp
static int vt_tty_putchar(struct tty *tty, char c) {
	struct vt_instance *term = tty->private;
	vt_echo_char(term, c);

	vt_delay_flush();
	return 0;
}


// callback function called when a key is stroke
void vt_key_stroke(int code) {

	// all keyboard input are 'redirected' to the current active terminal, if any
	if(_vt_current >= 0 && _vt_current <VT_MAX_TERMINALS) {
		struct vt_instance *term;

		term = & _vts[_vt_current];
		tty_input_char(& term->tty, (char)code);
	}
}


// used to blink the cursor
static int _vt_blink_scheduled = 0;

static void vt_toggle_cursor();

static void vt_cursor_schedule_blink() {
	if(!_vt_blink_scheduled) {
		stimer_add(&vt_toggle_cursor, NULL, TICKS_MSEC_NOTNULL(750));
		_vt_blink_scheduled = 1;
	}
}

static void vt_toggle_cursor() {
	if(_vt_current >= 0 && _vt_current <VT_MAX_TERMINALS) {
		// TODO handle case where cusor isn't displayed, etc...
		if(_vts[_vt_current].disp.cursor == TEXT_CURSOR_NORMAL)
			_vts[_vt_current].disp.cursor = TEXT_CURSOR_DISABLE;
		else
			_vts[_vt_current].disp.cursor = TEXT_CURSOR_NORMAL;

		vt_delay_flush();

		_vt_blink_scheduled = 0;
		vt_cursor_schedule_blink();
	}
	else
		_vt_blink_scheduled = 0;
}


void vt_set_active(int term) {
	if(term >= -1 && term < VT_MAX_TERMINALS) {
		if(term != _vt_current) {
			_tdisp->set_active(& _vts[_vt_current].disp, 0);
			if(term != -1) {
				_tdisp->set_active(& _vts[term].disp, 1);
				vt_cursor_schedule_blink();
			}
			_vt_current = term;
		}
	}
}


static int vt_tty_write(struct tty *tty, const char *data, size_t len) {
	struct vt_instance *term = tty->private;

	vt_term_print(term, data, len);

	// screen should be flushed only if it's the current active
	if(term == &_vts[_vt_current])
		vt_delay_flush();
	return len;
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

