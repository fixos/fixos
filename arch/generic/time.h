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

#ifndef _ARCH_GENERIC_TIME_H
#define _ARCH_GENERIC_TIME_H

/**
 * Should define TICK_HZ, the base clock frequency!
 */
#include <arch/time.h>

struct timespec;


/**
 * Initialize machine time-related stuff, and set the "tick" interrupt.
 */
void arch_time_init();

/**
 * Try to read real time information from an external real time clock
 * if present, and fill the given struct with the current time.
 * Returns 0 in success case, negative value else (no RTC or error)
 */
int arch_time_get_hw(struct timespec *t);

int arch_time_set_hw(const struct timespec *t);


#endif //_ARCH_GENERIC_TIME_H
