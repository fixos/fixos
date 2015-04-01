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

#ifndef _ARCH_SH_FREQ_H
#define _ARCH_SH_FREQ_H

/**
 * SH3 CPU and peripheral frequency control.
 */


// CKIO frequency multiplier
#define FREQ_STC_4		3
#define FREQ_STC_3		2
#define FREQ_STC_2		1
#define FREQ_STC_1		0
#define FREQ_STC_SAME	-1


// Internal and Peripheral frequency dividers (4 for 1/4e multiplier)
#define FREQ_DIV_4		3
#define FREQ_DIV_3		2
#define FREQ_DIV_2		1
#define FREQ_DIV_1		0
#define FREQ_DIV_SAME	-1


/**
 * Change global frequency multiplier, or Internal/Peripheral clock divider.
 */
int freq_change(int ckio_mul, int ifc, int pfc);



/**
 * Calibrate CKIO frequency, allowing to know the internal and peripheral clock
 * frequency (useful to know the real frequency used internaly, and for timers).
 * Because of usage of RTC and TMU0 interrupt, RTC must be initialized and no
 * other module should use RTC interrupt before.
 */
int freq_time_calibrate();


/**
 * Get CKIO frequency (after a freq_time_calibrate() call).
 */
unsigned int freq_get_ckio_hz();

/**
 * Get Peripheral clock frequency (after a freq_time_calibrate() call).
 */
unsigned int freq_get_peripheral_hz();

/**
 * Get Internal clock frequency (after a freq_time_calibrate() call).
 */
unsigned int freq_get_internal_hz();


#endif //_ARCH_SH_FREQ_H
