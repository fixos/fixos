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

#ifndef INTERRUPT_CODES_H
#define INTERRUPT_CODES_H

/**
 * This file contain exception and interruption codes (INTEVT and EXPEVT registers)
 */

// interrupts :  INTEVT2 codes 

#define INT_CODE_NMI			0x1C0

#define INT_CODE_TMU0_TUNI0		0x400
#define INT_CODE_TMU1_TUNI1		0x420
#define INT_CODE_TMU2_TUNI2		0x440
#define INT_CODE_TMU2_TICPI2	0x460
#define INT_CODE_RTC_ATI		0x480
#define INT_CODE_RTC_PRI		0x4A0
#define INT_CODE_RTC_CUI		0x4C0

#define INT_CODE_WTD_ITI		0x560
#define INT_CODE_REF_RCMI		0x580

#define INT_CODE_UDI			0x5E0
#define INT_CODE_IRQ_IRQ0		0x600
#define INT_CODE_IRQ_IRQ1		0x620
#define INT_CODE_IRQ_IRQ2		0x640
#define INT_CODE_IRQ_IRQ3		0x660
#define INT_CODE_IRQ_IRQ4		0x680
#define INT_CODE_IRQ_IRQ5		0x6A0

#define INT_CODE_PINT_0_7		0x700
#define INT_CODE_PINT_8_15		0x720

#define INT_CODE_DMAC_DIE0		0x800
#define INT_CODE_DMAC_DIE1		0x820
#define INT_CODE_DMAC_DIE2		0x840
#define INT_CODE_DMAC_DIE3		0x860			
#define INT_CODE_SCIF0_ERI0		0x880
#define INT_CODE_SCIF0_RXI0		0x8A0

#define INT_CODE_SCIF0_TXI0		0x8E0
#define INT_CODE_SCIF2_ERI2		0x900
#define INT_CODE_SCIF3_RXI2		0x920

#define INT_CODE_SCIF2_TXI2		0x960
#define INT_CODE_ADC_ADI		0x980

#define INT_CODE_USB_USI0		0xA20
#define INT_CODE_USB_USI1		0xA40

#define INT_CODE_TPU0_TPI0		0xC00
#define INT_CODE_TPU1_TPI1		0xC20

#define INT_CODE_TPU2_TPI2		0xC80
#define INT_CODE_TPU3_TPI3		0xCA0


// only for models with SDHI :
#define INT_CODE_SDHI_SDI		0xE80


// exceotions : EXPEVT codes
#define EXP_CODE_RESET_POWERON	0x000
#define EXP_CODE_RESET_MANUAL	0x020

// these two codes may occurs at VBR+0x100 (TLB invalid)
// and at VBR+0x400 (TLB miss special handler)
#define EXP_CODE_TLB_READ		0x040
#define EXP_CODE_TLB_WRITE		0x060

#define EXP_CODE_TLB_INITWRITE	0x080

#define EXP_CODE_TLB_PROTECT_R	0x0A0
#define EXP_CODE_TLB_PROTECT_W	0x0C0

#define EXP_CODE_ACCESS_READ	0x0E0
#define EXP_CODE_ACCESS_WRITE	0x100

#define EXP_CODE_TRAPA			0x160

#define EXP_CODE_BAD_INSTR		0x180
#define EXP_CODE_BAD_SLOTINSTR	0x1A0

#define EXP_CODE_USER_BREAK		0x1E0

#define EXP_CODE_DMA_ERROR		0x5C0

#endif //INTERRUPT_CODES_H
