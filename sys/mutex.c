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

#include "mutex.h"
#include <sys/interrupt.h>
#include <sys/scheduler.h>


void mutex_init_unlocked(struct mutex *m) {
	m->locked = 0;
}


int mutex_trylock(struct mutex *m) {
	int atomic_state;
	int ret;

	interrupt_atomic_save(&atomic_state);
	ret = m->locked;
	if(!ret) {
		m->locked = 1;
	}
	interrupt_atomic_restore(atomic_state);

	return ret;
}


void mutex_lock(struct mutex *m) {
	int atomic_state;
	int ret;
	volatile int *locked = &(m->locked);

	ret = 0;
	do {
		interrupt_atomic_save(&atomic_state);
		if(*locked == 0) {
			*locked = 1;
			ret = 1;
		}
		interrupt_atomic_restore(atomic_state);

		// if mutex not locked, give the CPU to another task
		if(!ret) {
			sched_schedule();
		}
	} while(!ret);
}


void mutex_unlock(struct mutex *m) {
	m->locked = 0;
}
