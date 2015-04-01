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

