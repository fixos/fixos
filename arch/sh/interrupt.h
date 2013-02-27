#ifndef _SH_INTERRUPT_H
#define _SH_INTERRUPT_H

#include "7705.h"


typedef void(*interrupt_callback_t)();

typedef unsigned int interrupt_priorities_t[8];

// Interruptions IDs :
#define INT_TMU0		0x00
#define INT_TMU1		0x01
#define INT_TMU2		0x02

#define INT_PINT_0_7	0x03
#define INT_PINT_8_15	0x04

#define INT__NUMBER	0x05   // interruptions number

// Inhibit or dishinibit all interruptions/exceptions
// if mode == 0, interrupts will be inhibited, else they will be handled
extern void interrupt_inhibit_all(int mode);

void interrupt_set_vbr(void *vbr);

void* interrupt_get_vbr(void);

// Store the interrupt context to restore it later.
// This function must be call before all other.
// (VBR and IPRx registers are saved) 
void interrupt_store_context(interrupt_priorities_t ipr_storage);

// Restore the stored context.
void interrupt_restore_context(const interrupt_priorities_t ipr_storage);

// Set a callback called for a specific interrupt.
// Many interrupt may have the same callback function.
void interrupt_set_callback(unsigned int interruptID, interrupt_callback_t address);

// Get the callback for an interrupt, usefull to check if an interrupt
// has already a correspondng callback.
interrupt_callback_t interrupt_get_callback(unsigned int interruptID);

// macros to access to the interrupts priority (read/write)
#define INTERRUPT_PRIORITY_IRQ0		(INTX.IPRC.BIT._IRQ0)
#define INTERRUPT_PRIORITY_IRQ1		(INTX.IPRC.BIT._IRQ1)
#define INTERRUPT_PRIORITY_IRQ2		(INTX.IPRC.BIT._IRQ2)
#define INTERRUPT_PRIORITY_IRQ3		(INTX.IPRC.BIT._IRQ3)
#define INTERRUPT_PRIORITY_IRQ4		(INTX.IPRD.BIT._IRQ4)
#define INTERRUPT_PRIORITY_IRQ5		(INTX.IPRD.BIT._IRQ5)
#define INTERRUPT_PRIORITY_PINT0_7	(INTX.IPRD.BIT._PINT0_7)
#define INTERRUPT_PRIORITY_PINT8_15	(INTX.IPRD.BIT._PINT8_15)
#define INTERRUPT_PRIORITY_DMAC		(INTX.IPRE.BIT._DMAC)
#define INTERRUPT_PRIORITY_SCIF0	(INTX.IPRE.BIT._SCIF0)
#define INTERRUPT_PRIORITY_SCIF2	(INTX.IPRE.BIT._SCIF2)
#define INTERRUPT_PRIORITY_ADC		(INTX.IPRE.BIT._ADC)
#define INTERRUPT_PRIORITY_USB		(INTX.IPRF.BIT._USB)
#define INTERRUPT_PRIORITY_TPU0		(INTX.IPRG.BIT._TPU0)
#define INTERRUPT_PRIORITY_TPU1		(INTX.IPRG.BIT._TPU1)
#define INTERRUPT_PRIORITY_TPU2		(INTX.IPRH.BIT._TPU2)
#define INTERRUPT_PRIORITY_TPU3		(INTX.IPRH.BIT._TPU3)
#define INTERRUPT_PRIORITY_TMU0		(INTC.IPRA.BIT._TMU0)
#define INTERRUPT_PRIORITY_TMU1		(INTC.IPRA.BIT._TMU1)
#define INTERRUPT_PRIORITY_TMU2		(INTC.IPRA.BIT._TMU2)
#define INTERRUPT_PRIORITY_RTC		(INTC.IPRA.BIT._RTC)
#define INTERRUPT_PRIORITY_WDT		(INTC.IPRB.BIT._WDT)
#define INTERRUPT_PRIORITY_REF		(INTC.IPRB.BIT._REF)


#endif //_SH_INTERRUPT_H
