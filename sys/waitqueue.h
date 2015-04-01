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

#ifndef _SYS_WAITQUEUE_H
#define _SYS_WAITQUEUE_H

/**
 * Wait queue for process, allowing to sleep until a given event (the wait
 * queue itself is the "event").
 */

#include <utils/list.h>

struct process;

struct wait_queue {
	struct dlist_head towake;
};

struct wait_item {
	struct dlist_head list;
	struct process *proc;
};

#define WAIT_QUEUE_INIT(name) \
	{ .towake = LIST_HEAD_INIT((name).towake) }

#define INIT_WAIT_QUEUE(ptr) \
	INIT_DLIST_HEAD(& (ptr)->towake)

/**
 * Wait until the given condition become true.
 * The condition may be evaluated many times, side effect should be avoided.
 * TODO saffer implementation, to avoid race conditions!
 */
#define wait_until_condition(wqueue, cond) \
	while(!(cond)) { \
		wqueue_wait_event(wqueue); \
	}


/**
 * Calling process is put in PROCESS_STATE_UNINTERRUPTIBLE sleeping state
 * until the given waitqueue is used to wake it up.
 * wait_until_condition() should be used in most cases
 */
void wqueue_wait_event(struct wait_queue *queue);

/**
 * Called when the corresponding event happens, wake up all the waiting
 * processes if any.
 */
void wqueue_wakeup(struct wait_queue *queue);

#endif //_SYS_WAITQUEUE_H
