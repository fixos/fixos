#include "virtual_term.h"
#include <utils/cyclic_fifo.h>
#include <fs/file_operations.h>
#include <sys/waitqueue.h>
#include "text_display.h"

#include <device/terminal/fx9860/text_display.h>


#define VT_INPUT_BUFFER		256
#define VT_LINE_BUFFER		128


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
};


static struct vt_instance _vts[VT_MAX_TERMINALS];
static int _vt_current;

const struct device virtual_term_device = {
	.name = "v-term",
	.init = &vt_init,
	.open = &vt_open
};


static const struct file_operations _vt_fop = {
	.release = &vt_release,
	.write = &vt_write,
	.read = &vt_read,
	.ioctl = &vt_ioctl
};


static const struct text_display *_tdisp = &fx9860_text_display;


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
	}
}


// function used to print characters
static void vt_term_print(struct vt_instance *term, void *source, size_t len) {
	//TODO future extensions to support more VT100-like escape codes
	
	int i;
	unsigned char *str = source;

	for(i=0; i<len; i++) {
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

	// print cursor at current position
	_tdisp->print_char(& term->disp, term->posx, term->posy, VT_CURSOR_CHAR);

	// TODO flush only if active!
	//disp_mono_copy_to_dd(_term_vram);
}



// callback function called when a key is stroke
void vt_key_stroke(int code) {

	// all keyboard input are 'redirected' to the current active terminal, if any
	if(_vt_current >= 0 && _vt_current <VT_MAX_TERMINALS) {
		struct vt_instance *term;

		term = & _vts[_vt_current];

		if(code == '\x08') {
			// backspace, remove last buffered char and echo space instead
			if(term->line_pos > 0)  {
				term->line_pos--;

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
		}
		else {
			// check the line buffer, and add the char to it if possible
			if(term->line_pos < VT_LINE_BUFFER) {
				if(code < 0x80) {
					char ccode = (char)code;

					term->line_buf[term->line_pos] = ccode;
					term->line_pos++;

					// basic echo, need to be improved (do not copy_to_dd() each time...)
					vt_term_print(term, &ccode, 1);
					_tdisp->flush(& term->disp);

					// copy and flush the line buffer if end of line is reached
					if(code == '\n') {
						cfifo_push(& term->fifo, term->line_buf, term->line_pos);
						term->line_pos = 0;
						wqueue_wakeup(& term->wqueue);
					}
				}
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

	return -1;
}


size_t vt_write(struct file *filep, void *source, size_t len) {
	int term;
	
	term = (int)(filep->private_data);
	if(term >= 0 && term <VT_MAX_TERMINALS) {
		vt_term_print(& _vts[term], source, len);
		
		// screen should be flushed only if it's the current active
		if(term == _vt_current)
			_tdisp->flush(& _vts[term].disp);

		return len;
	}
	return -1;
}


size_t vt_read(struct file *filep, void *dest, size_t len) {
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

	return -1;
}


int vt_release(struct file *filep) {
	return 0;
}



int vt_ioctl(struct file *filep, int cmd, void *data) {
	return -1;
}
