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

#include "time.h"
#include "process.h"
#include <sys/scheduler.h>
#include <sys/cpu_load.h>
#include <arch/generic/time.h>
#include <arch/generic/process.h>


extern int stimer_do_tick();


// clock counter, incremented at each time_do_tick
static clock_t _monotonic_ticks = 0;

// real time clock (from startup moment, not from epoch)
static struct timespec _monotonic_time;

// real time at time_init() call, relative to epoch
// so real time at a any moment is obtained by adding _monotonic_time to it.
static struct timespec _init_epoch_time;



// arch-specific constant, representing the time ellapsed during one clock tick
extern const struct timespec g_arch_tick_time;

// TODO move it to arch-spec file
const struct timespec g_arch_tick_time = {
	.tv_sec = 0,
	.tv_nsec = 3906250  // 256 Hz
};




void time_init() {
	arch_time_init();

	_monotonic_ticks = 0;
	_monotonic_time.tv_sec = 0;
	_monotonic_time.tv_nsec = 0;
	arch_time_get_hw(&_init_epoch_time);
}



clock_t time_monotonic_ticks() {
	return _monotonic_ticks;
}



void time_monotonic_time(struct timespec *t) {
	t->tv_sec = _monotonic_time.tv_sec;
	t->tv_nsec = _monotonic_time.tv_nsec;
}


void time_real_time(struct timespec *t) {
	time_add_hr(&_monotonic_time, &_init_epoch_time, t);
}


void time_do_tick() {
	struct process *cur;

	_monotonic_ticks++;
	time_add_hr(&_monotonic_time, &g_arch_tick_time, &_monotonic_time);

	cur = process_get_current();
	if(cur != NULL) {
		// check if process is in user or kernel mode
		if(arch_process_mode(cur) == 1) {
			cur->uticks++;
		}
		else {
			cur->kticks++;
		}

		// add a tick for average CPU usage count
		load_proc_addtick(cur);
	}

	stimer_do_tick();

	// scheduler tick ellapsed
	sched_check();

}

int sys_gettimeofday(struct timespec *tv, struct timezone *tz) {
	(void)tz;

	if(tv != NULL) {
		struct timespec temp_tv; // for later user-space secured copy

		time_real_time(&temp_tv);
		tv->tv_sec = temp_tv.tv_sec;
		tv->tv_nsec = temp_tv.tv_nsec;
	}

	return 0;
}


clock_t sys_times(struct tms *buf) {
	if(buf != NULL) {
		struct process *cur;

		cur = process_get_current();
		buf->tms_utime = cur->uticks;
		buf->tms_stime = cur->kticks;
		buf->tms_cutime = 0;
		buf->tms_cstime = 0;
	}
	return _monotonic_ticks;
}
