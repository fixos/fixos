#ifndef _SYS_CONSOLE_H
#define _SYS_CONSOLE_H

/**
 * Stuff related to the kernel console.
 * This console must be a tty-like device (virtual terminal, serial TTY,
 * USB emulating serial).
 */

#include <utils/types.h>


/**
 * Switch kernel output to the console used (either the default, tty1, or the
 * one specified by 'console=...' boot time argument).
 * This function should be called only when all possible devices are ready.
 * TODO better design, to allow using each device as soon as possible without
 * waiting the last one to be ready.
 */
void console_make_active();

/**
 * Return major/minor numbers representing the device used as the kernel console.
 */
uint32 console_get_device();

#endif //_SYS_CONSOLE_H
