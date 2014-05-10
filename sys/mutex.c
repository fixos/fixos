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
