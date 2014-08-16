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
