#ifndef _ARCH_SH_KDELAY
#define _ARCH_SH_KDELAY

/*
 * Some kernel-level delay/sleep functions.
 * Be careful, very low level sleep...
 */


/*
 * kernel delay, using watchdog
 * time is in 140micro-seconds unit aproximatively on 9860G not "overclocked"
 * (the peripheral clock seems to be at more or less 11MHz)
 * TODO get better estimation
 *
 * time must be between 1 and 40
 */
void kdelay(int time);


/*
 * More convenient function than kdelay, as time is in micro-seconds
 * This function is not really precise, the real sleep time depends on
 * various things, and will always be greater than the given time.
 *
 * For small numbers (less than 500micro-seconds), the relative difference may
 * be important...
 */
void kusleep(int time);

#endif // _ARCH_SH_KDELAY
