#include "process.h"
#include <arch/sh/mmu.h>
#include <sys/memory.h>
#include <arch/sh/interrupt.h>
#include <utils/strutils.h>
#include "scheduler.h"
#include <utils/log.h>

#include <device/keyboard/keyboard.h>

// entry in free linked list
union proc_entry {
	process_t p;
	union proc_entry *next;
};

#define PROCESS_PER_PAGE (PM_PAGE_BYTES/sizeof(union proc_entry))

static void *_proc_page;

static union proc_entry *_first_free;

// TODO move that to an arch-specific part
void * g_process_current_kstack = NULL;

// array of process ptr for corresponding ASIDs
// that allow ASID -> PID translation in O(1)
static process_t * _asid_proc_array[MAX_ASID];

// contain the pid used for the next process creation
static pid_t _pid_next_value = 1;

// Test purpose, like a kernel process
static process_t mock_process = 
{
	.pid = 0,
	.asid = 0xFF,
	.vm = {
		.direct = {{0}, {0}, {0}},
		.indir1 = (void*)0
	},
	.state = PROCESS_STATE_RUN
};


void process_init()
{
	int i;
	union proc_entry *cur;

	// init ASID -> process table
	for(i=0; i<MAX_ASID; i++) {
		_asid_proc_array[i] = NULL;
	}

	printk("process: proc/page=%d\n", PROCESS_PER_PAGE);

	_proc_page = mem_pm_get_free_page(MEM_PM_CACHED);
	cur = _first_free = _proc_page;

	// init free linked list
	for(i=0; i<PROCESS_PER_PAGE; i++) {
		cur->next = cur+1;
		cur++;
	}
	
	(cur-1)->next = NULL;
}



process_t *process_from_asid(asid_t asid)
{
	if(asid == 0xFF)
		return &mock_process;
	else if(asid < MAX_ASID)
		return _asid_proc_array[asid];
	return NULL;
}


// TODO
process_t *process_from_pid(pid_t pid)
{
	if(pid == 0)
		return &mock_process;
	else return NULL;
}


process_t *process_alloc() {
	if(_first_free != NULL) {
		process_t *proc;
		union proc_entry *next;
		int i;

		next = _first_free->next;
		proc = &(_first_free->p);

		for(i=0; i<PROCESS_MAX_FILE; i++)
			proc->files[i] = NULL;
		proc->pid = _pid_next_value++;
		proc->ppid = 0;
		proc->state = PROCESS_STATE_CREATE;
		proc->asid = ASID_INVALID;
		vm_init_table(&(proc->vm));

		// set first free to next one
		_first_free = next;
		return proc;
	}
	return NULL;
}


void process_free(process_t *proc) {
	union proc_entry *entry;

	entry = (void*)proc;
	entry->next = _first_free;
	_first_free = entry;
	printk("free proc %p\n", entry);
}


int process_set_asid(process_t *proc)
{
	// stupid-but-functionnal algorithm, again
	int i;
	int found = 0;

	for(i=0; i<MAX_ASID && !found; i++)
		found = (_asid_proc_array[i] == NULL);

	if(found) {
		_asid_proc_array[i-1] = proc;
		proc->asid = i-1;
		return 0;
	}
	else {
		// TODO if all ASID are currently used, erase one of them (complex because
		// of VM freeing and other things...)
		return -1;
	}
}


void process_release_asid(process_t *proc) {
	
	// no more virtual memory usage, we can release its ASID
	if(proc->asid != ASID_INVALID) {
		_asid_proc_array[proc->asid] = NULL;
		proc->asid = ASID_INVALID;
	}

}



process_t *process_get_current() {
	return process_from_asid(mmu_getasid());
}


extern int* _bank0_context;

void process_contextjmp(process_t *proc) {
	
	interrupt_inhibit_all(1);

	// black magic to save old BANK0 context
	process_get_current()->acnt.bank0_saved = _bank0_context;
	_bank0_context = proc->acnt.bank0_saved;
	

	// if ASID is not valid (first contextjmp, or process was remove from 'active'
	// process, we need to set it's ASID before to run it
	if(proc->asid == ASID_INVALID)
		process_set_asid(proc);

	//

	// just before context jump, set MMU current ASID to process' ASID
	mmu_setasid(proc->asid);
	printk("asid=%d [%d], pid=%d\n", proc->asid, mmu_getasid(), proc->pid);

	/*printk("r15=%p, r0=%p\npc=%p, sr=%p\n", (void*)(proc->acnt.reg[15]),
			(void*)(proc->acnt.reg[0]), (void*)(proc->acnt.pc),
			(void*)(proc->acnt.sr));
*/
	/*static int magic = 0;
	if(magic == 1)
		while(1);
	magic = 1;*/

	proc->state = PROCESS_STATE_RUN;

	arch_kernel_contextjmp(&(proc->acnt));
}


pid_t sys_fork() {
	process_t *cur;
	process_t *newproc;
	int i;

	arch_int_weak_atomic_block(1);

	printk("fork start\n");

	// alloc a new process, and copy everything
	cur = process_get_current();
	newproc = process_alloc();

	newproc->ppid = cur->pid;
	for(i=0; i<PROCESS_MAX_FILE; i++) {
		// TODO real COPY of each opened file!
		newproc->files[i] = cur->files[i];
	}

	// copy each memory page with same virtual addresses
	// TODO copy-on-write system!
	for(i=0; i<3; i++) {
		printk("page: [%s] p[%d] v[%d]\n", cur->vm.direct[i].valid ? "V" : "!v",
				cur->vm.direct[i].ppn, cur->vm.direct[i].vpn);
		if(cur->vm.direct[i].valid) {
			mem_vm_copy_page(&(cur->vm.direct[i]), &(newproc->vm.direct[i]),
					MEM_VM_COPY_ONWRITE);
		}
	}
	// TODO indirect pages
	newproc->vm.indir1 = NULL;
	
	// get the new kernel stack
	void *kstack;
	void *cur_stack;
	void *new_bank0_context;

	kstack = mem_pm_get_free_page(MEM_PM_CACHED) + PM_PAGE_BYTES;
	newproc->acnt.kernel_stack = kstack;

	// black magic : we now _bank0_context is on the kernel stack...
	new_bank0_context = (kstack - PM_PAGE_BYTES) 
			+ ( ((unsigned int)_bank0_context) % PM_PAGE_BYTES);

	// compute the position in stack
	asm volatile ("mov r15, %0" : "=r"(cur_stack));
	// WARNING : only works if *ONE* page is used for kernel stack!
	kstack -= PM_PAGE_BYTES - ((unsigned int)(cur_stack)  % PM_PAGE_BYTES);

	printk("stack : %p->%p\nbank0 : %p->%p\n", cur_stack, kstack, _bank0_context, new_bank0_context);
	
	// copy page content (do not forget, stack is lower address on top)
	memcpy(kstack, cur_stack,
			PM_PAGE_BYTES - ((unsigned int)(kstack) % PM_PAGE_BYTES));

	// do the pseudo-fork by saving context on child, and check the
	// return value
	int val = arch_sched_preempt_fork(newproc, kstack); 
	printk("preempt_fork returned %d\n", val);
	while(!is_key_down(K_EXE));
	while(is_key_down(K_EXE));

	if(val == 0) {
		// we are in the parent process (the one which realy returned)
		//printk("fork: parent code\n");
		sched_add_task(newproc);
		arch_int_weak_atomic_block(0);

		return cur->pid;
	}
	else {
		// we use some black magic to return from the TRAPA exception
		// into the child process
		
		// exceptions/interrupt are inhibited in the return context!
		_bank0_context = new_bank0_context;

		//printk("fork: child code\n");
		return 0;
	}
}




void sys_exit(int status) {
	process_t *cur;
	int i;

	cur = process_get_current();
	
	// TODO close files?
	
	// remove all virtual pages from the TLB (invalidate them)
	// TODO do not invalidate ALL entries, select only this ASID
	// in addition, free each allocated physical pages
	mmu_tlbflush();
	for(i=0; i<3; i++) {
		if(cur->vm.direct[i].valid) {
			mem_pm_release_page((void*)(PM_PHYSICAL_ADDR(cur->vm.direct[i].ppn)));
		}
	}
	// TODO indirect pages
	
	// VM is not used after, but the process need to have an ASID until
	// it will be remove from Zombie list.

	cur->exit_status = status;
	cur->state = PROCESS_STATE_ZOMBIE;

	// do not free the kernel stack before wait() is called to be sure
	// we can still execute code in case of re-execution of zombie process
	while(1) {
		sched_next_task(cur);
		printk("exit: exited process executed!\n");
	}
}



pid_t sys_getpid() {
	return process_get_current()->pid;
}

pid_t sys_getppid() {
	return process_get_current()->ppid;
}

