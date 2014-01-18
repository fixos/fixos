#include <device/usb/usb_device_protocol.h>
#include <arch/sh/7705_Casio.h>
#include <arch/sh/interrupt.h>
#include <utils/strutils.h>
#include <utils/cyclic_fifo.h>

#include <utils/log.h>

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
	printk("USB: 0x%x 0x%x (0x%x) [0x%x]\nUnknown Port = 0x%x\nClock = 0x%x\n", PFC.PNCR.WORD, PFC.PNCR2.BYTE, PFC.PMCR.WORD, STBCR3.BYTE, UKNPORT.BYTE, USB.UCLKCR.BYTE);
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
	INTERRUPT_PRIORITY_USB	= 0xA;
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

	//printk("usb_recv: [0x%x] {%p, %p}\n", endpoint, bufsize, buffer);

	while(ret>=0 && ret<size && !((flags & USB_RECV_PARTIAL) && ret!=0)
			&& !((flags & USB_RECV_NONBLOCK) && tried_once) )
	{
		unsigned char prio = INTERRUPT_PRIORITY_USB;
		int nbbytes;
		int bufsize;

		INTERRUPT_PRIORITY_USB = 0x00; // stop interrupt for USB!
		bufsize = (volatile int)(fifo->size);

		nbbytes = bufsize + ret;
		nbbytes = nbbytes > size ? size - ret : nbbytes - ret;
		if(nbbytes > 0) {
			cfifo_pop(fifo, data + ret, nbbytes);
			ret += nbbytes;

			if(endpoint == USB_EP_ADDR_EP1OUT)
				usb_sh3_ep1_read(nbbytes);

			//printk("recv: [%d] %d/%d\n", nbbytes, fifo->size, fifo->max_size);
		}

		// re-enable interrupts
		INTERRUPT_PRIORITY_USB = prio;

		tried_once = 1;
	}

	return ret;
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
	//printk("USB interrupt:\nISRs = 0x%x 0x%x\n", USB.ISR0.BYTE, USB.ISR1.BYTE);
	// printk("# IFRs: 0x%x 0x%x [0x%x]\n", USB.IFR0.BYTE, USB.IFR1.BYTE, USB.EPSTL.BYTE);
	//printk("# IERs: 0x%x 0x%x [0x%x]\n", USB.IER0.BYTE, USB.IER1.BYTE, USB.EPSZ1);
	
	// if it's a VBUS interrupt
	if(USB.IER1.BIT.VBUS == 1 && USB.IFR1.BIT.VBUS == 1) {
		printk("# VBUS interrupt (%s)\n", USB.IFR1.BIT.VBUSMN ? "plugged" : "unplugged");
		USB.IFR1.BIT.VBUS = 0;
		jobdone = 1;
	}
	
	// if it's a BRST (Bus ReSeT)
	if(USB.IER0.BIT.BRST == 1 && USB.IFR0.BIT.BRST == 1) {
		printk("# USB Bus Reset handled!\n");

		USB.IFR0.BIT.BRST = 0;

		// clear every FIFO :
		USB.FCLR.BYTE = 0x73;
		jobdone = 1;
	}

	// if it's a SETUPTS
	if(USB.IER0.BIT.SETUPTS == 1 && USB.IFR0.BIT.SETUPTS == 1) {
		struct usb_setup setup;
		int i;

		// clear SETUPTS flag
		USB.IFR0.BIT.SETUPTS = 0;

		for(i=0; i<8; i++)
			((char*) &setup)[i] = USB.EPDR0s;

		// set Setup EP0 is read
		USB.TRG.BIT.EP0sRDFN = 1;

		printk("# SETUPTS: (%d,%d,%d,%d,%d)\n", setup.bm_request_type.BYTE, setup.b_request,
				USB_WORD_FROM(setup.w_index), USB_WORD_FROM(setup.w_value), USB_WORD_FROM(setup.w_length));

		if(_usb_setup_callback != NULL) {
			if(_usb_setup_callback(&setup) != 0) {
				// if setup was not handled, stall EP0
				// TODO
			}
		}
		else {
			printk("usb: SETUP received but no callback!\n");
		}

		jobdone = 1;
	}

	if(USB.IER0.BIT.EP0iTR == 1 && USB.IFR0.BIT.EP0iTR == 1) {
		// TODO async send
		printk("# EP0 INreq");/* (remain %d/%d B)\n", _usb_ep0i_length - _usb_ep0i_pos, _usb_ep0i_length);

		
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
		printk("# EndPoint0 transmited.\n");

		USB.IFR0.BIT.EP0iTS = 0;
		jobdone = 1;
	}

	if(USB.IER0.BIT.EP0oTS == 1 && USB.IFR0.BIT.EP0oTS == 1) {
		printk("# EP0 Out received\n");
		// TODO other than 0-length data receceived!

		USB.TRG.BIT.EP0oRDFN = 1;
		USB.IFR0.BIT.EP0oTS = 0;
		jobdone = 1;
	}

	if(USB.IER0.BIT.EP1FULL == 1 && USB.IFR0.BIT.EP1FULL == 1) {
		int i;
		int nbreceived = USB.EPSZ1;

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
			//printk("usb: no enougth space for ep1o\n");
			// in addition, EP1FULL interrupt is disabled
			// the function usb_sh3_ep1_read() is used to signal some part of
			// the buffer was read, and maybe EP1FULL can be re-enabled
			USB.IER0.BIT.EP1FULL = 0;
		}

/*		// wait for EP2 transmited :
		while(USB.IFR0.BIT.EP2EMPTY != 1);

		// echo'ed on EP2
		for(i=0; i<nbreceived && i<64; i++)
			USB.EPDR2 = inbuf[i];
		USB.TRG.BIT.EP2PKTE = 1;
*/
		// remove magic lock to allow USB printk tests ;)
		_magic_lock = 1;

		USB.IFR0.BIT.EP1FULL = 0;
		jobdone = 1;
	}

	if(USB.IER1.BIT.EP3TR == 1 && USB.IFR1.BIT.EP3TR == 1) {
		// nothing to do for now
		printk("*");

		USB.IFR1.BIT.EP3TR = 0;
		jobdone = 1;
	}

	if(USB.IER1.BIT.EP3TS == 1 && USB.IFR1.BIT.EP3TS == 1) {
		// nothing to do for now
		printk("+");

		USB.IFR1.BIT.EP3TS = 0;
		jobdone = 1;
	}

	if(USB.IER0.BIT.EP2TR == 1 && USB.IFR0.BIT.EP2TR == 1) {
		printk("# EP2 Transmit Request\n");

	//	USB.TRG.BIT.EP2PKTE = 1;
		printk("### EP2 stalled!\n");
		USB.EPSTL.BIT.EP2STL = 1;

		USB.IFR0.BIT.EP2TR = 0;
		jobdone = 1;
	}


	if(!jobdone) {
		printk("USB: Do not know what to do!\n");
		printk("# IFRs: 0x%x 0x%x [0x%x]\n", USB.IFR0.BYTE, USB.IFR1.BYTE, USB.EPSTL.BYTE);
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


	for(nb_written=0, error=0; nb_written<max_size && !error; ) {
		// copy at most 8 bytes each time
		for(i=0; i+nb_written < max_size && i<8; i++)
			USB.EPDR0i = data[nb_written+i];

		// data is now ready :
		USB.IFR0.BIT.EP0iTS = 0;
		USB.IFR0.BIT.EP0iTR = 0;
		USB.TRG.BIT.EP0iPKTE = 1;
		nb_written += i;

		printk("=");
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

	printk("SyncWrite %d/%d [%d]\n", nb_written, max_size, error);
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
	return max_size;
}

