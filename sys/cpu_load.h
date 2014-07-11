#ifndef _SYS_CPU_LOAD_H
#define _SYS_CPU_LOAD_H

/**
 * CPU load calculation stuff, both per-process and global average load.
 * TODO global load alculation (see BSD way to do that, with fixed point
 * arithmetic on exponential decay, etc...)
 */

#include <sys/process.h>
#include <sys/time.h>

/**
 * Upadate all load-related informations in the given process, with cur_ticks
 * the monotonic ticks elapsed since boot (as a parameter to avoid to
 * call monotnick_ticks() if not needed).
 * Should be called before any usage of the load data.
 */
extern inline void load_proc_update(process_t *p, clock_t cur_ticks) {
	clock_t deltasample;

	deltasample = cur_ticks/PROC_LOAD_SAMPMAX - p->load_last/PROC_LOAD_SAMPMAX;
	switch(deltasample) {
		case 0:
			break;
		case 1:
			// just replace the corresponding sample
			p->load_samples[(p->load_last/PROC_LOAD_SAMPMAX) % PROC_LOAD_SAMPLES]
				= p->load_cursamp;
			p->load_cursamp = 0;
			break;
		default:
		{
			// at least two samples to remove
			int first;
			int nb;
			int i;

			first = (p->load_last/PROC_LOAD_SAMPMAX) % PROC_LOAD_SAMPLES;
			nb = deltasample > PROC_LOAD_SAMPLES ? PROC_LOAD_SAMPLES : deltasample;
			for(i=1; i<nb ; i++) {
				p->load_samples[(first + i) % PROC_LOAD_SAMPLES] = 0;
			}
			p->load_samples[first] = p->load_cursamp;
			p->load_cursamp = 0;
		}
	}


	// last updated jiffie
	p->load_last = cur_ticks;
}


extern inline void load_proc_addtick(process_t *p) {
	load_proc_update(p, time_monotonic_ticks());
	p->load_cursamp++;
}


/**
 * Get the average CPU time for the given process.
 * The returned value is 100 times the percent of CPU usage.
 * TODO fixed point!
 */
extern inline int load_proc_average(process_t *p) {
	int i;
	int ret;

	load_proc_update(p, time_monotonic_ticks());
	ret = 0;
	for(i=0; i<PROC_LOAD_SAMPLES; i++)
		ret += p->load_samples[i];	

	return (ret * 100 * 100) / (PROC_LOAD_SAMPMAX * PROC_LOAD_SAMPLES);
}

#endif //_SYS_CPU_LOAD_H
