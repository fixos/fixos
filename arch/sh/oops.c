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
	__asm__ volatile ("mov r14, %0;"
				"mov r15, %1"
				: "=r"(cur), "=r"(stack) : : );
				*/

	printk(LOG_EMERG, "Stack=%p\nCall trace:\n", stack);
	i = 0;
	while(stack < bottom && i < MAX_STACK_FRAMES) {
		if(IS_KERNEL_TEXT(*stack)) {
			printk(LOG_EMERG, "@%p: ", stack);
			kdebug_print_symbol((void*)*stack);
			printk(LOG_EMERG, "\n");
			i++;
		}
		stack++;
	}
	printk(LOG_EMERG, "(End of call trace)\n\n");
}



void kdebug_print_trace() {
	struct process *proc;
	void *stack;

	proc = process_get_current();

	__asm__ volatile ("mov r15, %0" : "=r"(stack) : : );
	arch_print_trace(stack, (uint32*)(proc->acnt));//proc->kernel_stack);
}



void kdebug_oops(const char *errstr) {
	struct process *proc;
	struct _context_info *cont;
	

	printk(LOG_EMERG, "Fatal kernel oops!\n");
	if(errstr != NULL)
		printk(LOG_EMERG, "(%s)\n", errstr);
	printk(LOG_EMERG, "Following informations may help :\n");

	proc = process_get_current();
	if(proc != NULL) {
		printk(LOG_EMERG, "Running pid %d (asid=%d)\n", proc->pid, mmu_getasid());
	}

	kdebug_print_vmspace(proc);

	printk_force_flush();

	// print each kernel context information
	kdebug_print_trace();

	cont = proc->acnt;
	while(cont != NULL && cont->previous != NULL) {
		printk(LOG_EMERG, "---- Previous Context ----\n       PC: ");
		kdebug_print_symbol((void*)(cont->pc));
		printk(LOG_EMERG, "\n       PR: ");
		kdebug_print_symbol((void*)(cont->pr));
		printk(LOG_EMERG, "\n");

		arch_print_trace((uint32*)(cont->reg[15]), (uint32*)(cont->previous) );
		cont = cont->previous;
	}

	printk_force_flush();

	while(1);
}
