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

#ifndef _SYS_MUTEX_H
#define _SYS_MUTEX_H

/**
 * Mutual exclusion implementation, allowing to protect ressources inside the
 * kernel.
 * Be careful if used in interrupt context, or any other context where no sleep
 * is allowed : mutex_trylock() should be used instead of mutex_lock() and
 * the caller must handle case where mutex can't be acquired!
 */


/**
 * Mutex data structure (probably changed in a near future).
 */
struct mutex {
	int locked;
};

// static initializer to define an unlocked mutex
#define INIT_MUTEX_UNLOCKED		{ .locked = 0 }


/**
 * Initialize a mutex in a not-locked state.
 */
void mutex_init_unlocked(struct mutex *m);


/**
 * Try to lock the given mutex, without blocking if already acquired.
 * Return 0 if the mutex was not locked, 1 otherwise.
 */
int mutex_trylock(struct mutex *m);


/**
 * Lock the given mutex, sleeping until it is unlocked (in this case the
 * scheduler is called to schedule an other task).
 */
void mutex_lock(struct mutex *m);


/**
 * Unlock the given mutex.
 */
void mutex_unlock(struct mutex *m);


#endif //_SYS_MUTEX_H
