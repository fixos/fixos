#ifndef _DEVICE_USB_CDC_ACM
#define _DEVICE_USB_CDC_ACM

/**
 * Primitives for an implementation of CDC Abstract Communication Model USB class.
 * These primitives allow direct usage of this USB device class, or can be
 * used wrapped into a device.
 */


#include <utils/types.h>


struct usb_setup;


/**
 * Initialize CDC ACM module (find good endpoints, prepare setup handler...)
 */
void cdc_acm_init();


/**
 * Get the CDC/ACM connection status.
 * Returns 1 if USB connection is configured and host seems ready,
 * 0 in all other cases (for exemple USB not connected at all).
 */
int cdc_acm_is_ready();


/**
 * Send data using cdc_acm, and return number of bytes writen.
 */
size_t cdc_acm_send(const char *data, size_t size);

/**
 * Receive data using cdc_acm, return number of bytes read.
 */
size_t cdc_acm_receive(char *dest, size_t size);

/**
 * USB Setup handler (see usb_setup_handler_t and usb_set_setup_handler())
 */
int cdc_acm_setup_handler(const struct usb_setup *setup);


#endif //_DEVICE_USB_CDC_ACM
