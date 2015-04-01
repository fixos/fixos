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

#ifndef _SYS_INTERRUPT_H
#define _SYS_INTERRUPT_H

/**
 * Interrupt-related functions and definitions.
 * This part is no arch-specific, be careful if you need functionalities
 * provided by any arch-specific header (maybe it's a better idea to 
 * write a more generic interface in this file?).
 */


/**
 * Save the current interrupt state (arch-specific value) and disable
 * all interrupts.
 * Use it with interrupt_atomic_restore() to define atomic sections.
 */
void interrupt_atomic_save(int *state);


/**
 * Restore interrupt handling as it was when the corresponding
 * interrupt_atomic_save() was called.
 */
void interrupt_atomic_restore(int state);


/**
 * Return 1 if called inside an atomic block, 0 if interrupts are allowed.
 */
int interrupt_in_atomic();


#endif //_SYS_INTERRUPT_H
