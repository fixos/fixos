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

#include "waitqueue.h"
#include <sys/scheduler.h>
#include <sys/process.h>


void wqueue_wait_event(struct wait_queue *queue) {
	// use a local variable to store the wait item (on kernel stack)
	struct wait_item item;
	struct process *proc;
	
	proc = process_get_current();
	item.proc = proc;
	dlist_push_back(& queue->towake, & item.list);

	// now, sleep in uninterruptible state...
	proc->state = PROCESS_STATE_UNINTERRUPTIBLE;
	sched_schedule();
}


void wqueue_wakeup(struct wait_queue *queue) {
	struct dlist_head *cur;

	// wake up each waiting process
	// TODO lock the list
	dlist_for_each(cur, & queue->towake) {
		struct wait_item *item;

		item = container_of(cur, struct wait_item, list);
		sched_wake_up(item->proc);
	}

	// clean the list
	INIT_WAIT_QUEUE(queue);
}
