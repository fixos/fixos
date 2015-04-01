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
