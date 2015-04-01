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

#ifndef _SYS_STIMER_H
#define _SYS_STIMER_H

/**
 * Soft timers, used to delay some work or to implement periodic job.
 * These timers aren't really accurate, and are built on top of time subsystem.
 * The time unit is clock ticks, so the real time depends on configuration
 * and plateform used.
 *
 * A soft timer calls the associated stimer_func_t in a not-reliable context
 * (even if stimer_add() was called in a user context of a specific process,
 * the timer function will be called in any process context, and can't be
 * preempted).
 * Therefore, a timer function should never try to sleep, schedule another
 * task, lock any ressource...
 */

#include <utils/types.h>

typedef void (*stimer_func_t)(void *data);

struct stimer_job;


/**
 * Init the soft-timer subsystem.
 */
void stimer_init();


/**
 * Add a soft timer, calling func(data) when timed out, after duration ticks.
 * Return a pointer to the added timer, or NULL if an error occurs. 
 * When the timer expires, its data structure is removed from the current
 * working timers, so you should not use the returned value after that.
 */
struct stimer_job * stimer_add(stimer_func_t func, void *data, clock_t duration);


/**
 * Remove a previously added soft timer, that didn't expire for now.
 * TODO seems not reliable for now (race condition if timer expires at the same
 * time...)
 */
int stimer_remove(struct stimer_job *timer);


/**
 * Called by the time subsystem when a tick occurs.
 */
int stimer_do_tick();



#endif //_SYS_STIMER_H
