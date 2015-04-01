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

#ifndef _UTILS_LOG_H
#define _UTILS_LOG_H

#include <utils/types.h>

// callack used to print a part of a printk() message
typedef void(*print_callback_t)(const char*);

struct tty;

/**
 * Message level are using the same names than the POSIX syslog() priorities.
 * This allow to avoid printing some minor messages to the console when needed,
 * and also to remove debug messages at compile time with some tricks.
 */
#define LOG_EMERG	7
#define LOG_ALERT	6
#define LOG_CRIT	5
#define LOG_ERR		4
#define LOG_WARNING	3
#define LOG_NOTICE	2
#define LOG_INFO	1
#define LOG_DEBUG	0


/**
 * printf-like function to log misc kernel messages
 * Currently, only some modifier are allowed : %s, %x, %p, %d
 * The print callback will be called at least once, but it may be called many time.
 *
 * We use a little preprocessor magic to remove calls of printk with a level
 * lower than CONFIG_PRINTK_STATIC_LEVEL.
 * An implementation based on inline functions is not possible because of
 * variable arguments, so we have to use preprocessor to "inline" this code and
 * let the compiler remove useless calls.
 * As level is usualy a compile-time constant, the print_internal() call should
 * be optimized by any decent compiler as an unreachable path.
 */

// should return an integer to be used with a boolean operator...
void printk_internal(int level, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

// define it as an expression, not a statement, so it will seem like a real
// function...
#define printk(level, format, ...) \
	( (level) >= CONFIG_PRINTK_STATIC_LEVEL ? \
		 printk_internal(level, format, ##__VA_ARGS__)  : (void)0)


/**
 * Helper for printing raw memory dump (from addr to addr+len)...
 */
void print_memory(int level, void *addr, size_t len);


/**
 * Set dynamic log level. All messages with level lower than the current
 * log level value will not be displayed on the console.
 */
void printk_set_level(int level);


/**
 * Set the printk function used for print a string chunk.
 */
void set_kernel_print(print_callback_t func);


/**
 * Set printk callback function to tty_write(logfile, msg, strlen(msg)).
 * This function override the callback defined by set_kernel_print().
 */
void printk_set_console_tty(struct tty *tty);


/**
 * Should force the previous printk() content to be displayed to the
 * underlying console if possible.
 */
void printk_force_flush();

#endif //_UTILS_LOG_H
