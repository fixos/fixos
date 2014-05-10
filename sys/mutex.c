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

	while(!ret) {
		interrupt_atomic_save(&atomic_state);
		ret = *locked;
		if(!ret) {
			*locked = 1;
		}
		interrupt_atomic_restore(atomic_state);

		// if mutex not locked, give the CPU to another task
		// TODO real implementation
		//sched_schedule();
	}
}


void mutex_unlock(struct mutex *m) {
	m->locked = 0;
}
