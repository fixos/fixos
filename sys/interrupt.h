#ifndef _SYS_INTERRUPT_H
#define _SYS_INTERRUPT_H

/**
 * Interrupt-related functions and definitions.
 * This part is no arch-specific, be careful if you need functionalities
 * provided by any arch-specific header (maybe it's a better idea to 
 * write a more generic interface in this file?).
 */


/**
 * Save the current interrupt state (arch-specific value) and disable
 * all interrupts.
 * Use it with interrupt_atomic_restore() to define atomic sections.
 */
void interrupt_atomic_save(int *state);


/**
 * Restore interrupt handling as it was when the corresponding
 * interrupt_atomic_save() was called.
 */
void interrupt_atomic_restore(int state);


/**
 * Return 1 if called inside an atomic block, 0 if interrupts are allowed.
 */
int interrupt_in_atomic();


#endif //_SYS_INTERRUPT_H
