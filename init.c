//#include "arch/sh/7705_Casio.h"
#include "display/T6K11/T6K11.h"
#include "sys/terminal.h"
#include "keyboard/keyboard.h"
#include "arch/sh/interrupt.h"
#include "arch/sh/virtual_memory.h"
#include "arch/sh/mmu.h"
#include "sys/process.h"
#include "arch/sh/physical_memory.h"
#include "utils/log.h"

extern void * fixos_vbr;  // see fixos.ld

void test();

// Real entry point of the OS :
void init() {
	unsigned char vram[1024];
	int j;

	// init exceptions/interruptions handling
	interrupt_set_vbr(&fixos_vbr); // don't forget we want the ADDRESS of linked vbr 
	// tmp stuff
	 INTERRUPT_PRIORITY_IRQ0 = 0;
	 INTERRUPT_PRIORITY_IRQ1 = 0;
	 INTERRUPT_PRIORITY_IRQ2 = 0;
	 INTERRUPT_PRIORITY_IRQ3 = 0;
	 INTERRUPT_PRIORITY_IRQ4 = 0;
	 INTERRUPT_PRIORITY_IRQ5 = 0;
	 INTERRUPT_PRIORITY_PINT0_7 = 0;
	 INTERRUPT_PRIORITY_PINT8_15 = 0;
	 INTERRUPT_PRIORITY_DMAC = 0;
	 INTERRUPT_PRIORITY_SCIF0 = 0;
	 INTERRUPT_PRIORITY_SCIF2 = 0;
	 INTERRUPT_PRIORITY_ADC = 0;
	 INTERRUPT_PRIORITY_USB	= 0;
	 INTERRUPT_PRIORITY_TPU0 = 0;
	 INTERRUPT_PRIORITY_TPU1 = 0;
	 INTERRUPT_PRIORITY_TPU2 = 0;
	 INTERRUPT_PRIORITY_TPU3 = 0;
	 INTERRUPT_PRIORITY_TMU0 = 0;
	 INTERRUPT_PRIORITY_TMU1 = 0;
	 INTERRUPT_PRIORITY_TMU2 = 0;
	 INTERRUPT_PRIORITY_RTC = 0;
	 INTERRUPT_PRIORITY_WDT = 0;
	 INTERRUPT_PRIORITY_REF = 0;

	terminal_set_vram(vram);
	terminal_clear();

	set_kernel_print(&terminal_write);

	terminal_write("Boostrap... OK!\n");
	terminal_set_colors(TCOLOR_WHITE, TCOLOR_BLACK);
	terminal_write("WARNING : ");
	terminal_set_colors(TCOLOR_BLACK, TCOLOR_WHITE);
	terminal_write("it's working!\n  It's really strange...\n");

	mmu_init();
	pm_init_pages();

//	INTERRUPT_PRIORITY_PINT0_7 = 0xF;
//	INTERRUPT_PRIORITY_PINT8_15 = 0xF;

	interrupt_set_callback(INT_PINT_0_7, &test);
	interrupt_set_callback(INT_PINT_8_15, &test);
	
	while(is_key_down(K_EXE));
	while(!is_key_down(K_EXE));

	process_t *mock = process_from_asid(0xFF);

	vm_page_t page;
	int magic_integer = 589;
	int magic_offset;
	page.cache = 0;
	page.size = VM_DEFAULT_SIZE;
	page.valid = 1;
	page.vpn = (0x01000000 >> 10); // a translatable address
	page.ppn = ((((int)&magic_integer) & 0x1FFFFFFF) >> 10); // address of a stack page ;)

	magic_offset = ((int)(&magic_integer)) & 0x000003FF;

	vm_init_table(&(mock->vm));
	vm_add_entry(&(mock->vm), &page);

	while(is_key_down(K_EXE));
	while(!is_key_down(K_EXE));

	interrupt_inhibit_all(0);


	asm volatile ("trapa #50");

	int *surprise = (int*)(0x01000000 + magic_offset);

	printk("Magic? %d\n", *surprise);
	*surprise = 986;

	unsigned int ppn1;
	if(! pm_get_free_page(&ppn1)) {
		printk("Page number : %d\n", ppn1);
	}

	// just for fun
	/*asm volatile ("mov #3, r0");
	asm volatile ("mov.l @r0, r1");
*/


	PFC.PBCR.WORD = 0x5555;
	PB.DR.BYTE = 0b00000000;
	PFC.PACR.WORD = 0xAAAA;

	INTX.PINTER.WORD = 0b0000000011111111;

	j=0;
	while(1) {
		printk("Test a la con : %d\n", j);
		j++;
		//while(is_key_down(K_EXE));
		//while(!is_key_down(K_EXE));
		while(1);
	}

}

void test() {
	static int i = 0;

	printk("Ohohoh! Surprise^%d!\n", i);
	i++;
}

