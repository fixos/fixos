#ifndef _DEVICE_USB_CDC_ACM
#define _DEVICE_USB_CDC_ACM

/**
 * Primitives for an implementation of CDC Abstract Communication Model USB class.
 * These primitives allow direct usage of this USB device class, or can be
 * used wrapped into a device.
 */

struct usb_setup;


/**
 * Initialize CDC ACM module (find good endpoints, prepare setup handler...)
 */
void cdc_acm_init();


/**
 * USB Setup handler (see usb_setup_handler_t and usb_set_setup_handler())
 */
int cdc_acm_setup_handler(const struct usb_setup *setup);


#endif //_DEVICE_USB_CDC_ACM
