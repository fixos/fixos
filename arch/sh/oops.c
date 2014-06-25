#include <utils/log.h>
#include <sys/process.h>
#include "process.h"
#include "mmu.h"


// in fixos.ld
extern uint16 kernel_text_begin;
extern uint16 kernel_text_end;
#define IS_KERNEL_TEXT(addr)	((uint16*)(addr) >= &kernel_text_begin \
								&& (uint16*)(addr) <= &kernel_text_end)

#define MAX_STACK_FRAMES	15


struct symbol_entry {
	uint32 val;
	const char *name;
};

#ifdef CONFIG_DEBUG_SYMBOL_NAMES
extern struct symbol_entry symbols_begin;
extern struct symbol_entry symbols_end;


static const struct symbol_entry *get_nearest_symbol(void *addr) {
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
static inline const struct symbol_entry *get_nearest_symbol(void *addr) {
	(void)addr;
	return NULL;
}
#endif //CONFIG_DEBUG_SYMBOL_NAMES


void arch_print_trace(uint32 *stack, uint32 *bottom) {
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
			const struct symbol_entry *symbol;

			// try to get symbol name and offset if registered
			symbol = get_nearest_symbol((void*)*stack);
			
			if(symbol != NULL) {
				printk("@%p: <%s + %d> (%p)\n", stack, symbol->name,
						*stack - symbol->val, (void*)*stack);
			}
			else {
				printk("@%p: <%p>\n", stack, (void*)(*stack));
			}
			i++;
		}
		stack++;
	}
	printk("(End of call trace)\n\n");
}


void kernel_oops() {
	process_t *proc;
	void *stack;

	printk("Fatal kernel oops!\n"
			"Following informations may help :\n");

	proc = process_get_current();
	if(proc != NULL) {
		printk("Running pid %d (asid=%d)\n", proc->pid, mmu_getasid());
	}

	asm volatile ("mov r15, %0" : "=r"(stack) : : );
	arch_print_trace(stack, (uint32*)(proc->acnt));//proc->kernel_stack);
	while(1);
}
