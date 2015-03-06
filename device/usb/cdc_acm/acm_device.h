#ifndef _DEVICE_USB_CDC_ACM_ACM_DEVICE_H
#define _DEVICE_USB_CDC_ACM_ACM_DEVICE_H

/**
 * Device for USB communication using CDC/ACM class.
 * The only implemented MINOR is 0, which is the ACM 'pipe'.
 * TODO : support ioctls
 */

#include <device/device.h>
#include <fs/file.h>


#define ACM_USB_TTY_MINOR		6

void acm_usb_init();

#endif //_DEVICE_USB_CDC_ACM_ACM_DEVICE_H
