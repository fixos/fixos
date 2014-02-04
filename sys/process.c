#include "process.h"
#include <arch/sh/mmu.h>
#include <sys/memory.h>
#include <arch/sh/interrupt.h>
#include <utils/strutils.h>
#include "scheduler.h"
#include <utils/log.h>

#include <loader/elfloader/loader.h>
#include <fs/vfs_file.h>
#include <fs/vfs.h>
#include <fs/vfs_op.h>

// temp stuff
#include <device/keyboard/fx9860/keymatrix.h>
#include <device/keyboard/fx9860/matrix_codes.h>

// entry in free linked list
union proc_entry {
	process_t p;
	union proc_entry *next;
};

#define PROCESS_PER_PAGE (PM_PAGE_BYTES/sizeof(union proc_entry))

static void *_proc_page;

static union proc_entry *_first_free;

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
	.state = PROCESS_STATE_RUN,
	.acnt = NULL,
	.kernel_stack = NULL
};

// this variable must be maintained by the scheduler as the
// current running process at *ANY* time
process_t *_proc_current = &mock_process;



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
	//return process_from_asid(mmu_getasid());
	return _proc_current;
}


void process_contextjmp(process_t *proc) {
	
	interrupt_inhibit_all(1);

	// if ASID is not valid (first contextjmp, or process was remove from 'active'
	// process, we need to set it's ASID before to run it
	if(proc->asid == ASID_INVALID)
		process_set_asid(proc);

	_proc_current = proc;

	// just before context jump, set MMU current ASID to process' ASID
	mmu_setasid(proc->asid);
	//printk("asid=%d [%d], pid=%d\n", proc->asid, mmu_getasid(), proc->pid);

	/*printk("r15=%p, r0=%p\npc=%p, sr=%p\n", (void*)(proc->acnt.reg[15]),
			(void*)(proc->acnt.reg[0]), (void*)(proc->acnt.pc),
			(void*)(proc->acnt.sr));
*/
	/*static int magic = 0;
	if(magic == 1)
		while(1);
	magic = 1;*/

	proc->state = PROCESS_STATE_RUN;

	arch_kernel_contextjmp(proc->acnt, &(proc->acnt));
}


pid_t sys_fork() {
	process_t *cur;
	process_t *newproc;
	int i;


	cur = process_get_current();
	// do fork only if this is the first context-switch of the process
	// (this should be the case here, but it's useful for debug)
	if(cur->acnt->previous != NULL) {
		printk("fork: multiple context... aborted\n");
		return -1;
	}
	

	//arch_int_weak_atomic_block(1);
	// we need to block any exception, fork operation should be
	// atomic
	interrupt_inhibit_all(1);

	printk("fork start\n");

	// alloc a new process, and copy everything
	newproc = process_alloc();

	newproc->ppid = cur->pid;
	for(i=0; i<PROCESS_MAX_FILE; i++) {
		// TODO real COPY of each opened file!
		newproc->files[i] = cur->files[i];
	}

	// copy each memory page with same virtual addresses
	// TODO copy-on-write system!
	for(i=0; i<3; i++) {
		/*printk("page: [%s] p[%d] v[%d]\n", cur->vm.direct[i].valid ? "V" : "!v",
				cur->vm.direct[i].ppn, cur->vm.direct[i].vpn);*/
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

	kstack = mem_pm_get_free_page(MEM_PM_CACHED) + PM_PAGE_BYTES;
	newproc->kernel_stack = kstack;

	// black magic : we know acnt is on the stack and acnt->previous is NULL
	newproc->acnt = (kstack - PM_PAGE_BYTES) 
			+ ( ((unsigned int)cur->acnt) % PM_PAGE_BYTES);

	// compute the position in stack
	asm volatile ("mov r15, %0" : "=r"(cur_stack));
	// WARNING : only works if *ONE* page is used for kernel stack!
	kstack -= PM_PAGE_BYTES - ((unsigned int)(cur_stack)  % PM_PAGE_BYTES);

	// copy page content (do not forget, stack is lower address on top)
	memcpy(kstack, cur_stack,
			PM_PAGE_BYTES - ((unsigned int)(kstack) % PM_PAGE_BYTES));

	// we modify the context-saved r0 in child, so it will return from fork()
	// with the given value
	newproc->acnt->reg[0] = 0;

	sched_add_task(newproc);
	//arch_int_weak_atomic_block(0);
	interrupt_inhibit_all(0);

	return newproc->pid;

	/*printk("preempt_fork returned %d\n", val);
	while(!hwkbd_real_keydown(K_EXE));
	while(hwkbd_real_keydown(K_EXE));*/
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




int sys_execve(const char *filename, char *const argv[], char *const envp[]) {
	inode_t *elf_inode;
	struct file *elf_file;

	// first, check if we can open and execute filename

	elf_inode = vfs_resolve(filename);
	if(elf_inode == NULL || (elf_file = vfs_open(elf_inode)) == NULL ) {
		printk("execve: failed to open '%s'\n", filename);
	}
	else {
		process_t *cur;
		int i;

		cur = process_get_current();

		// close/free all ressources not 'shared' through exec
		// TODO files with CLOSE_ON_EXEC
		// TODO atomic code!!!

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


		// TODO do not release kernel stack, ELF loader should not set it?
		void *old_kstack = cur->kernel_stack;
		
		cur->state = PROCESS_STATE_STOP;
		if(elfloader_load(elf_file , cur) != 0) {
			printk("execve: unable to load ELF file\n");
			// 'kill' process TODO proper way to do that
			cur->state = PROCESS_STATE_STOP;
			sched_next_task(cur);

			printk("execve: re-executed dead process!\n");
		}
		else {
			// the image is load, we can't simply return from syscall because
			// stack is now 'corrupted' (old_kstack is the real stack used, but
			// it's not the current proc kernel stack...)

			// TODO set argv[] and envp[]!!!

			 asm volatile (
					"mov %0, r15;"
					"mov %1, r0;"
					"mov %2, r4;"
					"jsr @r0;"
					"nop;"
					"mov %3, r4;"
					"mov %4, r0;"
					"jmp @r0;"
					"nop;" : : "r"(cur->kernel_stack), "r"(&mem_pm_release_page),
							"r"(old_kstack), "r"(cur), "r"(&process_contextjmp)
							: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7" );

			printk("execve: this should not happen!\n");
		}
	}

	return -1;
}


