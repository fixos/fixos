#include "time.h"
#include "process.h"
#include <sys/scheduler.h>


extern int stimer_do_tick();


// clock counter, incremented at each time_do_tick
static clock_t _monotonic_ticks = 0;

// real time clock (from startup moment, not from epoch)
static struct hr_time _monotonic_time;

// real time at time_init() call, relative to epoch
// so real time at a any moment is obtained by adding _monotonic_time to it.
static struct hr_time _init_epoch_time;



// arch-specific constant, representing the time ellapsed during one clock tick
extern const struct hr_time g_arch_tick_time;

// TODO move it to arch-spec file
const struct hr_time g_arch_tick_time = {
	.sec = 0,
	.nano = 3906250  // 256 Hz
};



extern void arch_time_init();

void time_init() {
	_monotonic_ticks = 0;
	_monotonic_time.sec = 0;
	_monotonic_time.nano = 0;
	time_get_hw(&_init_epoch_time);

	arch_time_init();
}



clock_t time_monotonic_ticks() {
	return _monotonic_ticks;
}



void time_monotonic_time(struct hr_time *t) {
	t->sec = _monotonic_time.sec;
	t->nano = _monotonic_time.nano;
}


void time_real_time(struct hr_time *t) {
	time_add_hr(&_monotonic_time, &_init_epoch_time, t);
}


void time_do_tick() {
	process_t *cur;

	_monotonic_ticks++;
	time_add_hr(&_monotonic_time, &g_arch_tick_time, &_monotonic_time);

	cur = process_get_current();
	if(cur != NULL) {
		// TODO check if process is in user or kernel mode
		if(1) { //process_mode(cur) == USER_MODE
			cur->uticks++;
		}
		else {
			cur->kticks++;
		}
	}

	stimer_do_tick();

	// scheduler tick ellapsed
	sched_check();

}

