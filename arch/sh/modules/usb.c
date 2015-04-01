/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <device/usb/usb_device_protocol.h>
#include <arch/sh/7705_Casio.h>
#include <arch/sh/interrupt.h>
#include <utils/strutils.h>
#include <utils/cyclic_fifo.h>
#include <arch/sh/kdelay.h>

#include <utils/log.h>

// uncomment to enable verbose debug mode for USB module
//#define DEBUG_USB

/**
 * This file contains the implementation of usb_device_protocol.h interface
 * for SH3 7705 and similar architecture (device-side USB using integrated
 * USB 2.0 controler).
 */

// buffer for received data
#define USB_EP0o_BUFFER_SIZE	128
static char _usb_buffer_ep0o[USB_EP0o_BUFFER_SIZE];
static struct cyclic_fifo _usb_fifo_ep0o = {
	USB_EP0o_BUFFER_SIZE, 0, 0, _usb_buffer_ep0o
};

#define USB_EP1o_BUFFER_SIZE	512
static char _usb_buffer_ep1o[USB_EP1o_BUFFER_SIZE];
static struct cyclic_fifo _usb_fifo_ep1o = {
	USB_EP1o_BUFFER_SIZE, 0, 0, _usb_buffer_ep1o
};


// setup callback
usb_setup_callback_t _usb_setup_callback = NULL;

// ep1 receive callback
usb_receive_callback_t _usb_ep1_receive_callback = NULL;


// debug stuff
extern int _magic_lock;


static void usb_sh3_interrupt_handler();

static int usb_send_sync_ep0(const char *data, int max_size);

static int usb_send_sync_ep2(const char *data, int max_size);

// call this function when some bytes from endpoint 1 out buffer are read
// this allow to re-enable EP1FULL interrupt when buffer was previously full
static void usb_sh3_ep1_read(int nbread);



void usb_init() {
	// USB test code (probably a lot of useless code here, TODO this is not External controler!)
	PFC.PNCR.WORD = 0x4000;
	PFC.PNCR2.BYTE = 0x7F;
	PFC.PMCR.BIT.PM6MD = 0;
	STBCR3.BIT._USB = 0;
	printk(LOG_DEBUG, "USB: 0x%x 0x%x (0x%x) [0x%x]\nUnknown Port = 0x%x\nClock = 0x%x\n", PFC.PNCR.WORD, PFC.PNCR2.BYTE, PFC.PMCR.WORD, STBCR3.BYTE, UKNPORT.BYTE, USB.UCLKCR.BYTE);
	interrupt_set_callback(INT_USB, &usb_sh3_interrupt_handler);
	USB.IER0.BYTE = 0xEF;
	USB.IER1.BYTE = 0x07;

	USB.IFR0.BYTE = 0x00;
	USB.IFR1.BYTE = 0x00;

	USB.ISR0.BYTE = 0x00;
	USB.ISR1.BYTE = 0x00;

	USB.EPSTL.BYTE = 0x00;
	USB.XVERCR.BIT.XVEROFF = 0;

	// enable pull-up on D+ pin
	UKNPORT.BIT.u2 = 1;

	USB.FCLR.BIT.EP1CLR = 1;
	USB.TRG.BIT.EP1RDFN = 1;

	// for now, some interrupts are disabled by default
	USB.IER0.BIT.EP2EMPTY = 0;
	USB.IER0.BIT.EP2TR = 0;
	USB.IER1.BIT.EP3TR = 0;
	USB.IER1.BIT.EP3TS = 0;

	// TODO reset global variable to allow using of usb_init()
	// to reset the USB subsystem?

	// enable USB clock (WARNING, writing magic 0xA5 as high byte because of
	// special register protection!)
	*(unsigned short *)&(USB.UCLKCR.BYTE) = 0xA5E0;
	INTERRUPT_PRIORITY_USB	= INTERRUPT_PVALUE_HIGH;

}



int usb_send(int endpoint, const char *data, size_t size) {
	switch(endpoint) {
	case USB_EP_ADDR_EP2IN:
		return usb_send_sync_ep2(data, size);
		break;

	case USB_EP_ADDR_EP0IN:
		return usb_send_sync_ep0(data, size);
		break;
	//case USB_EP_ADDR_EP3IN:
	
	default:
		return -1;
	}
}


volatile int chose1;

int usb_receive(int endpoint, char *data, size_t size, int flags) {
	// variables used to get data from any endpoint
	struct cyclic_fifo *fifo;
	ssize_t ret = -1;
	int tried_once = 0;

	switch(endpoint) {
	case USB_EP_ADDR_EP0OUT:
		fifo = &_usb_fifo_ep0o;
		break;
	
	case USB_EP_ADDR_EP1OUT:
		fifo = &_usb_fifo_ep1o;
		break;

	default:
		return -1;
	}


	// real job depends of flags
	ret = 0;

	//printk(LOG_DEBUG, "usb_recv: [0x%x] {%p, %p}\n", endpoint, bufsize, buffer);

	while(ret>=0 && ret<size && !((flags & USB_RECV_PARTIAL) && ret!=0)
			&& !((flags & USB_RECV_NONBLOCK) && tried_once) )
	{
		int nbbytes;
		int bufsize;

		// avoid to be interrupted, exceptialy by USB interrupt!
		arch_int_weak_atomic_block(1);

		bufsize = (volatile int)(fifo->size);

		nbbytes = bufsize + ret;
		nbbytes = nbbytes > size ? size - ret : nbbytes - ret;
		if(nbbytes > 0) {
			cfifo_pop(fifo, data + ret, nbbytes);
			ret += nbbytes;

			if(endpoint == USB_EP_ADDR_EP1OUT)
				usb_sh3_ep1_read(nbbytes);

		//	printk(LOG_DEBUG, "recv: [%d] %d/%d -> %d\n", nbbytes, fifo->size, fifo->max_size, ret);
		}

		// re-enable interrupts
		arch_int_weak_atomic_block(0);

		tried_once = 1;
	}

	//printk(LOG_DEBUG, "recv: ret(%d)", ret);
	return ret;
}


void usb_set_receive_callback(int endpoint, usb_receive_callback_t callback) {
	if(endpoint == USB_EP_EP1)
		_usb_ep1_receive_callback = callback;
}


static void usb_sh3_ep1_read(int nbread) {
	(void)nbread; // not used for now

	// if enough space is present in buffer
	if(_usb_fifo_ep1o.max_size - _usb_fifo_ep1o.size > USB.EPSZ1)
		USB.IER0.BIT.EP1FULL = 1;
}


int usb_find_endpoint_config(int ep_number, struct usb_endpoint_ability *eps) {
	// for this hardware, find a configuration is not difficult : 
	// EP1 for bulk out, EP2 for bulk in, and EP3 for interrupt.
	
	int i;
	int ret;
	int ep1used = 0, ep2used = 0, ep3used = 0;

	for(i=0, ret=0; i<ep_number && ret==0; i++) {
		if(eps[i].attributes == USB_EP_ATTR_BULK 
				&& eps[i].max_size <= 64)
		{
			if(eps[i].endpoint == USB_EP_DIR_OUT && !ep1used) {
				eps[i].endpoint = USB_EP_ADDR_EP1OUT;
				eps[i].max_size = 64;
				ep1used = 1;
			}
			else if(eps[i].endpoint == USB_EP_DIR_IN && !ep2used) {
				eps[i].endpoint = USB_EP_ADDR_EP2IN;
				eps[i].max_size = 64;
				ep2used = 1;
			}
			else
				ret = -1;
		}
		else if(eps[i].attributes == USB_EP_ATTR_INTER
				&& eps[i].max_size <= 8
				&& eps[i].endpoint == USB_EP_DIR_IN
				&& !ep3used)
		{
			eps[i].max_size = 8;
			eps[i].endpoint = USB_EP_ADDR_EP3IN;
			ep3used = 1;
		}
		else
			ret = -1;
	}

	return ret;
}



void usb_set_setup_callback(usb_setup_callback_t callback) {
	_usb_setup_callback = callback;
}




// interrupt handler function called for USB interrupts

void usb_sh3_interrupt_handler() {
	int jobdone = 0;
	//printk(LOG_DEBUG, "USB interrupt:\nISRs = 0x%x 0x%x\n", USB.ISR0.BYTE, USB.ISR1.BYTE);
	// printk(LOG_DEBUG, "# IFRs: 0x%x 0x%x [0x%x]\n", USB.IFR0.BYTE, USB.IFR1.BYTE, USB.EPSTL.BYTE);
	//printk(LOG_DEBUG, "# IERs: 0x%x 0x%x [0x%x]\n", USB.IER0.BYTE, USB.IER1.BYTE, USB.EPSZ1);
	
	// if it's a VBUS interrupt
	if(USB.IER1.BIT.VBUS == 1 && USB.IFR1.BIT.VBUS == 1) {
#ifdef DEBUG_USB
		printk(LOG_DEBUG, "# VBUS interrupt (%s)\n", USB.IFR1.BIT.VBUSMN ? "plugged" : "unplugged");
#endif
		USB.IFR1.BIT.VBUS = 0;
		jobdone = 1;
	}
	
	// if it's a BRST (Bus ReSeT)
	if(USB.IER0.BIT.BRST == 1 && USB.IFR0.BIT.BRST == 1) {
#ifdef DEBUG_USB
		printk(LOG_DEBUG, "# USB Bus Reset handled!\n");
#endif
		USB.IFR0.BIT.BRST = 0;

		// clear every FIFO :
		USB.FCLR.BYTE = 0x73;
		jobdone = 1;
	}

	// if it's a SETUPTS
	if(USB.IER0.BIT.SETUPTS == 1 && USB.IFR0.BIT.SETUPTS == 1) {
		struct usb_setup setup;
		int i;

		// clear SETUPTS flag and EP0 i/o FIFOs
		USB.IFR0.BIT.SETUPTS = 0;
		USB.FCLR.BIT.EP0iCLR = 1;
		USB.FCLR.BIT.EP0oCLR = 1;

		for(i=0; i<8; i++)
			((char*) &setup)[i] = USB.EPDR0s;

		// set Setup EP0 is read
		USB.TRG.BIT.EP0sRDFN = 1;

#ifdef DEBUG_USB
		printk(LOG_DEBUG, "# SETUPTS: (%d,%d,%d,%d,%d)\n", setup.bm_request_type.BYTE, setup.b_request,
				USB_WORD_FROM(setup.w_index), USB_WORD_FROM(setup.w_value), USB_WORD_FROM(setup.w_length));
#endif

		if(_usb_setup_callback != NULL) {
			if(_usb_setup_callback(&setup) != 0) {
				// if setup was not handled, stall EP0
				// TODO
			}
		}
		else {
			printk(LOG_DEBUG, "usb: SETUP received but no callback!\n");
		}

		jobdone = 1;
	}

	if(USB.IER0.BIT.EP0iTR == 1 && USB.IFR0.BIT.EP0iTR == 1) {
		// TODO async send
#ifdef DEBUG_USB
		printk(LOG_DEBUG, "# EP0 INreq\n");
#endif
		/* (remain %d/%d B)\n", _usb_ep0i_length - _usb_ep0i_pos, _usb_ep0i_length);

		
		if(_usb_ep0i_pos <  _usb_ep0i_length) {
			int i;
			// copy at most 8 bytes and fill with padding
			for(i=0; _usb_ep0i_pos < _usb_ep0i_length && i<8; i++, _usb_ep0i_pos++)
				USB.EPDR0i = _usb_ep0i_buffer[_usb_ep0i_pos];

			// data is now ready :
			USB.TRG.BIT.EP0iPKTE = 1;
		}*/

		USB.IFR0.BIT.EP0iTR = 0;
		jobdone = 1;
	}

	if(USB.IER0.BIT.EP0iTS == 1 && USB.IFR0.BIT.EP0iTS == 1) {
#ifdef DEBUG_USB
		printk(LOG_DEBUG, "# EP0 In transmited.\n");
#endif

		USB.IFR0.BIT.EP0iTS = 0;
		jobdone = 1;
	}

	if(USB.IER0.BIT.EP0oTS == 1 && USB.IFR0.BIT.EP0oTS == 1) {
		int nbreceived = USB.EPSZ0o;
#ifdef DEBUG_USB
		printk(LOG_DEBUG, "# EP0 Out received (%dB)\n", nbreceived);
#endif
		// TODO other than 0-length data receceived!

		USB.IFR0.BIT.EP0oTS = 0;

		// try to copy received data to ep1 buffer
		if(nbreceived > 0 && _usb_fifo_ep0o.size + nbreceived <= _usb_fifo_ep0o.max_size) {
			int i;

			// copy in cyclic FIFO need some additionnal things :
			int bufpos = (_usb_fifo_ep0o.top + _usb_fifo_ep0o.size) % _usb_fifo_ep0o.max_size;
			for(i=0; i<nbreceived; i++, bufpos = bufpos >= _usb_fifo_ep0o.max_size - 1 ? 0 : bufpos+1 ) {
				_usb_fifo_ep0o.buffer[bufpos] = USB.EPDR0o;
			}
			_usb_fifo_ep0o.size += nbreceived;
		}

		USB.TRG.BIT.EP0oRDFN = 1;
		jobdone = 1;
	}

	if(USB.IER0.BIT.EP1FULL == 1 && USB.IFR0.BIT.EP1FULL == 1) {
		int i;
		int nbreceived = USB.EPSZ1;

		// call the async receive callback?
		if(_usb_ep1_receive_callback != NULL) {
			for(i=0; i<nbreceived; i++)
				_usb_ep1_receive_callback(USB.EPDR1);
			USB.TRG.BIT.EP1RDFN = 1;
		}
		else {
			// try to copy received data to ep1 buffer
			if(_usb_fifo_ep1o.size + nbreceived <= _usb_fifo_ep1o.max_size) {

				// copy in cyclic FIFO need some additionnal things :
				int bufpos = (_usb_fifo_ep1o.top + _usb_fifo_ep1o.size) % _usb_fifo_ep1o.max_size;
				for(i=0; i<nbreceived; i++, bufpos = bufpos >= _usb_fifo_ep1o.max_size - 1 ? 0 : bufpos+1 ) {
					_usb_fifo_ep1o.buffer[bufpos] = USB.EPDR1;
				}
				_usb_fifo_ep1o.size += nbreceived;

				USB.TRG.BIT.EP1RDFN = 1;
			}
			else {
				//printk(LOG_DEBUG, "usb: no enougth space for ep1o\n");
				// in addition, EP1FULL interrupt is disabled
				// the function usb_sh3_ep1_read() is used to signal some part of
				// the buffer was read, and maybe EP1FULL can be re-enabled
				USB.IER0.BIT.EP1FULL = 0;
			}
		}

		USB.IFR0.BIT.EP1FULL = 0;
		jobdone = 1;
	}

	if(USB.IER1.BIT.EP3TR == 1 && USB.IFR1.BIT.EP3TR == 1) {
		// nothing to do for now
#ifdef DEBUG_USB
		printk(LOG_DEBUG, "*");
#endif

		USB.IFR1.BIT.EP3TR = 0;
		jobdone = 1;
	}

	if(USB.IER1.BIT.EP3TS == 1 && USB.IFR1.BIT.EP3TS == 1) {
		// nothing to do for now
#ifdef DEBUG_USB
		printk(LOG_DEBUG, "+");
#endif

		USB.IFR1.BIT.EP3TS = 0;
		jobdone = 1;
	}

	if(USB.IER0.BIT.EP2TR == 1 && USB.IFR0.BIT.EP2TR == 1) {
#ifdef DEBUG_USB
		printk(LOG_DEBUG, "# EP2 Transmit Request\n");
#endif

	//	USB.TRG.BIT.EP2PKTE = 1;
		printk(LOG_DEBUG, "### EP2 stalled!\n");
		USB.EPSTL.BIT.EP2STL = 1;

		USB.IFR0.BIT.EP2TR = 0;
		jobdone = 1;
	}


	if(!jobdone) {
		printk(LOG_WARNING, "USB: Do not know what to do!\n");
		printk(LOG_WARNING, "# IFRs: 0x%x 0x%x [0x%x]\n", USB.IFR0.BYTE, USB.IFR1.BYTE, USB.EPSTL.BYTE);
		USB.IFR0.BYTE = 0x00;
		USB.IFR1.BYTE = 0x00;
	}
}




// send synchronously EP0 data, returns the written bytes number
// return when everything is written or when not expected flag is set
int usb_send_sync_ep0(const char *data, int max_size)
{
	int nb_written;
	int error;
	int i;


	error = 0;
	for(nb_written=0, error=0; nb_written<max_size && !error; ) {
		// copy at most 8 bytes each time
		for(i=0; i+nb_written < max_size && i<8; i++)
			USB.EPDR0i = data[nb_written+i];

		// data is now ready :
		USB.IFR0.BIT.EP0iTS = 0;
		USB.IFR0.BIT.EP0iTR = 0;
		USB.TRG.BIT.EP0iPKTE = 1;
		nb_written += i;

#ifdef DEBUG_USB
		printk(LOG_DEBUG, "=");
#endif
		// FIXME why this delay is needed?
		kdelay(1);

		// wait before an other packet
		while(USB.IFR0.BIT.EP0iTS == 0 && (error = (USB.IFR0.BIT.BRST == 0 || USB.IFR1.BIT.VBUSMN == 1)) == 0);
		USB.IFR0.BIT.EP0iTS = 0;
		USB.IFR0.BIT.EP0iTR = 0;
	}

	// send a 0-length data if max_size is multiple of 8
	// (this include max_size == 0, 0-length explicit data)
	if(!error && max_size%8 == 0) {
		USB.IFR0.BIT.EP0iTS = 0;
		USB.IFR0.BIT.EP0iTR = 0;

		USB.TRG.BIT.EP0iPKTE = 1;
		while(USB.IFR0.BIT.EP0iTS == 0 && (error = (USB.IFR0.BIT.BRST == 0 || USB.IFR1.BIT.VBUSMN == 1)) == 0);

		USB.IFR0.BIT.EP0iTS = 0;
		USB.IFR0.BIT.EP0iTR = 0;
	}

#ifdef DEBUG_USB
	printk(LOG_DEBUG, "SyncWrite %d/%d [%d]\n", nb_written, max_size, error);
#endif
	return error ? -1 : nb_written;
}


/*
#define USB_EP0i_BUFFER_SIZE	128
static char _usb_ep0i_buffer[128];
static int _usb_ep0i_length = 0;
static int _usb_ep0i_pos = 0;

// copy given data in an internal buffer, and then try to send them asynchronously
// using USB interrupts (the function return before everything is sent)
static int usb_send_async_ep0(const char *data, int max_size)
{
	int nb_written = max_size > USB_EP0i_BUFFER_SIZE ? USB_EP0i_BUFFER_SIZE : max_size;

	memcpy(_usb_ep0i_buffer, data, nb_written);
	_usb_ep0i_length = nb_written;
	_usb_ep0i_pos = 0;

	return nb_written;
}
*/


// for synchronous EP2 sending :
int usb_send_sync_ep2(const char *data, int max_size)
{
	int i, total;

	// echo'ed on EP2
	for(total=0; total<max_size; total += 64) {
		// wait for EP2 transmited :
		while(USB.IFR0.BIT.EP2EMPTY != 1);

		for(i=0; i<max_size && i<64; i++)
			USB.EPDR2 = data[total+i];
		USB.TRG.BIT.EP2PKTE = 1;
	}

	// if less than 64Bytes of data, EP2EMPTY is still set because 2nd buffer is
	// not fill, so send a second EP2PKTE=1
	if(USB.IFR0.BIT.EP2EMPTY == 1)
		USB.TRG.BIT.EP2PKTE = 1;
	return max_size;
}

