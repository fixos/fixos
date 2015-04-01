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

#ifndef _ARCH_SH_TIMER_H
#define _ARCH_SH_TIMER_H

/**
 * Functions and definitions for hardware TMU.
 * Based on the 7705 TMUs (TMU0, TMU1 and TMU2, the Casio-specific TMU3 is not
 * implemented yet)
 */

#include "interrupt.h"

// Timer Prescaler Values :
#define TIMER_PRESCALER_4	0b000
#define TIMER_PRESCALER_16	0b001
#define TIMER_PRESCALER_64	0b010
#define TIMER_PRESCALER_256	0b011



/**
 * timer_init_tmuX() are used to initialize (but *not* to start) the given TMU.
 * time is the initial number of the down-counter, and prescaler is the divider
 * used on the peripheral clock of the CPU.
 * callback is called only once the associated TMU is started with interruption
 * enabled and down-counter is underflowing.
 */
void timer_init_tmu0(unsigned int time, int prescaler, interrupt_callback_t callback);

void timer_init_tmu1(unsigned int time, int prescaler, interrupt_callback_t callback);


/**
 * Start the given TMU counter. If interrupting == 1, underflow on this TMU will
 * set an interrupt (TUNIx).
 */
void timer_start_tmu0(int interrupting);

void timer_start_tmu1(int interrupting);

/**
 * Stop given TMU counter.
 */
void timer_stop_tmu0();

void timer_stop_tmu1();

#endif //_ARCH_SH_TIMER_H
