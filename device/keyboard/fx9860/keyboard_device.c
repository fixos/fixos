#include "keyboard_device.h"
#include <utils/cyclic_fifo.h>
#include <fs/file_operations.h>
#include <interface/fixos/fcntl.h>
#include <interface/fixos/errno.h>


const struct device fxkeyboard_device = {
	.name = "fxkdb",
	.init = &fxkdb_init,
	.open = &fxkdb_open
};


static const struct file_operations _fxkdb_fop = {
	.release = &fxkdb_release,
	.read = &fxkdb_read,
};


static struct fxkey_event _evt_buffer[FXKDB_MAX_KEYS];

static struct cyclic_fifo _evt_fifo = {
	.buffer = (char*)_evt_buffer,
	.max_size = sizeof(_evt_buffer),
	.size = 0,
	.top = 0
};

// number of potential listener, if 0 no key insertion is done
static int _listener = 0;



void fxkdb_init() {
	_listener = 0;
}


int fxkdb_insert_event(const struct fxkey_event *event) {
	if(_listener > 0 && _evt_fifo.size + sizeof(*event) <= _evt_fifo.max_size) {
		cfifo_push(&_evt_fifo, (char*)event, sizeof(*event));	
		return 0;
	}
	return -1;
}


int fxkdb_open(uint16 minor, struct file *filep) {
	if(minor == FXKDB_DIRECT_MINOR) {
		filep->op = &_fxkdb_fop;
		_listener++;
		return 0;
	}

	return -ENXIO;
}


int fxkdb_release(struct file *filep) {
	_listener--;
	return 0;
}


ssize_t fxkdb_read(struct file *filep, void *dest, size_t len) {
	// only allow a size multiple of fxkey_event size
	if(len % sizeof(struct fxkey_event) == 0) {
		// TODO atomic fifo access
		int curlen = 0;
		volatile size_t *fifo_size = &(_evt_fifo.size);
		int nonblock = filep->flags & O_NONBLOCK;

		do {
			size_t readlen;

			readlen =  *fifo_size;
			if(readlen > 0) {
				readlen = readlen + curlen > len ? len - curlen : readlen;
				readlen -= readlen % sizeof(struct fxkey_event);
				cfifo_pop(&_evt_fifo, dest, readlen);
				curlen += readlen;
			}
		} while(curlen < len && !nonblock);
		
		if(nonblock && curlen==0)
			return -EAGAIN;
		return curlen;
	}
	return -EINVAL;
}

