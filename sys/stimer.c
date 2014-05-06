#include "stimer.h"



struct stimer_job {
	stimer_func_t func;
	void *data;
	clock_t remaining_ticks;
};


// TODO real implementation (slice allocation? linked list? faster way...)
// For now, timers are a simple vector of struct stimer_job, and an unused
// item is marked by a NULL 'func' field.
#define STIMER_MAX_JOBS 10

static struct stimer_job _jobs[STIMER_MAX_JOBS];



void stimer_init() {
	int i;
	for(i=0; i<STIMER_MAX_JOBS; i++) {
		_jobs[i].func = NULL;
	}
}


struct stimer_job * stimer_add(stimer_func_t func, void *data, clock_t duration) {
	struct stimer_job *ret = NULL;
	int i;

	for(i=0; i<STIMER_MAX_JOBS && ret == NULL; i++) {
		if(_jobs[i].func == NULL)
			ret = &(_jobs[i]);
	}

	if(ret != NULL) {
		ret->data = data;
		ret->func = func;
		ret->remaining_ticks = duration;
	}

	return ret;
}



int stimer_remove(struct stimer_job *timer) {
	if(timer != NULL) {
		timer->func = NULL;
		return 0;
	}
	return 1;
}



int stimer_do_tick() {
	// TODO for now, when a timer expire, the function is called directly
	// in the current context.
	// Maybe it should be better to call these in a more appropriate context
	// ( at schedule time?)
	
	int i;
	
	for(i=0; i<STIMER_MAX_JOBS; i++) {
		if(_jobs[i].func != NULL) {
			_jobs[i].remaining_ticks--;
			if(_jobs[i].remaining_ticks <= 0) {
				_jobs[i].func(_jobs[i].data);
				_jobs[i].func = NULL;
			}
		}
	}

	return 0;
}

