#include <utils/log.h>
#include <sys/process.h>
#include <sys/memory.h>
#include <arch/generic/process.h>
#include "mmu.h"

#include <sys/kdebug.h>


// in fixos.ld
extern uint16 kernel_text_begin;
extern uint16 kernel_text_end;
#define IS_KERNEL_TEXT(addr)	((uint16*)(addr) >= &kernel_text_begin \
								&& (uint16*)(addr) <= &kernel_text_end)

#define MAX_STACK_FRAMES	15


#ifdef CONFIG_DEBUG_SYMBOL_NAMES
extern struct symbol_entry symbols_begin;
extern struct symbol_entry symbols_end;


const struct symbol_entry *kdebug_nearest_symbol(void *addr) {
	const struct symbol_entry *ret = NULL;
	const struct symbol_entry *cur;

	for(cur = &symbols_begin; cur < &symbols_end; cur++) {
		if(cur->val > (uint32)addr && cur != &symbols_begin) {
			ret = cur-1;
			break;
		}
	}
	return ret;
}

#else
const struct symbol_entry *kdebug_nearest_symbol(void *addr) {
	(void)addr;
	return NULL;
}
#endif //CONFIG_DEBUG_SYMBOL_NAMES




static void arch_print_trace(uint32 *stack, uint32 *bottom) {
	int i;

	/*
	asm volatile ("mov r14, %0;"
				"mov r15, %1"
				: "=r"(cur), "=r"(stack) : : );
				*/

	printk("Stack=%p\nCall trace:\n", stack);
	i = 0;
	while(stack < bottom && i < MAX_STACK_FRAMES) {
		if(IS_KERNEL_TEXT(*stack)) {
			printk("@%p: ", stack);
			kdebug_print_symbol((void*)*stack);
			printk("\n");
			i++;
		}
		stack++;
	}
	printk("(End of call trace)\n\n");
}



void kdebug_print_trace() {
	process_t *proc;
	void *stack;

	proc = process_get_current();

	asm volatile ("mov r15, %0" : "=r"(stack) : : );
	arch_print_trace(stack, (uint32*)(proc->acnt));//proc->kernel_stack);
}



void kdebug_oops(const char *errstr) {
	process_t *proc;
	struct _context_info *cont;
	

	printk("Fatal kernel oops!\n");
	if(errstr != NULL)
		printk("(%s)\n", errstr);
	printk("Following informations may help :\n");

	proc = process_get_current();
	if(proc != NULL) {
		printk("Running pid %d (asid=%d)\n", proc->pid, mmu_getasid());
	}

	kdebug_print_vmspace(proc);

	// print each kernel context information
	kdebug_print_trace();

	cont = proc->acnt;
	while(cont != NULL && cont->previous != NULL) {
		printk("---- Previous Context ----\n       PC: ");
		kdebug_print_symbol((void*)(cont->pc));
		printk("\n       PR: ");
		kdebug_print_symbol((void*)(cont->pr));
		printk("\n");

		arch_print_trace((uint32*)(cont->reg[15]), (uint32*)(cont->previous) );
		cont = cont->previous;
	}

	while(1);
}
