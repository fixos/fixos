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


/**
 * Place for arch-specific time related functions.
 */

#include <utils/types.h>
#include <sys/time.h>
#include <arch/sh/rtc.h>
#include <arch/generic/time.h>


// interrupt handler for RTC interrupt, update all the time subsystem
//static void periodic_rtc_interrupt();

void arch_time_init() {
		rtc_set_interrupt(&time_do_tick, RTC_PERIOD_256_HZ);
}



