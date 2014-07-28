#include "process.h"
#include <arch/sh/mmu.h>
#include <sys/memory.h>
#include <sys/interrupt.h>
#include <utils/strutils.h>
#include <utils/pool_alloc.h>
#include "scheduler.h"
#include <utils/log.h>
#include <utils/bitfield.h>
#include <interface/fcntl.h>

#include <loader/elfloader/loader.h>
#include <fs/vfs_file.h>
#include <fs/vfs.h>
#include <fs/vfs_op.h>

#include <sys/sysctl.h>
#include <sys/cpu_load.h>

// temp stuff
#include <device/keyboard/fx9860/keymatrix.h>
#include <device/keyboard/fx9860/matrix_codes.h>

// pool allocation data
static struct pool_alloc _proc_pool = POOL_INIT(process_t);

// array of process ptr for corresponding ASIDs
// that allow ASID -> PID translation in O(1)
static process_t * _asid_proc_array[MAX_ASID];

// contain the pid used for the next process creation
static pid_t _pid_next;

static BITFIELD_STATIC(_pid_used, CONFIG_PID_MAX);


struct list_head _process_list = LIST_HEAD_INIT(_process_list);

// number of processes in the system (not counting idle process)
static int _process_number = 0;

/**
 * this variable must be maintained by the scheduler as the
 * current running process at *ANY* time
 * TODO using idle task before scheduler start is working here because of
 * obscure implementation details, but its simpler than using NULL.
 * A good idea should be to make all that clean and use NULL at initialization?
 */
process_t *_proc_current = &_arch_idle_task;



void process_init()
{
	int i;

	// init ASID -> process table
	for(i=0; i<MAX_ASID; i++) {
		_asid_proc_array[i] = NULL;
	}

	// clear pid usage bitfield, and set pid 0 as used (idle task)
	bitfield_all_clear(_pid_used, CONFIG_PID_MAX);
	bitfield_set(_pid_used, 0);
	_pid_next = 1;

	printk("process: proc/page=%d\n", _proc_pool.perpage);
}



process_t *process_from_asid(asid_t asid)
{
	if(asid == 0xFF)
		return &_arch_idle_task;
	else if(asid < MAX_ASID)
		return _asid_proc_array[asid];
	return NULL;
}


// FIXME this is a temporary hack to have a pid -> process working, need to
// be changed soon
process_t *process_from_pid(pid_t pid)
{
	// should not return anything for PID 0 ?
	if(pid == 0)
		return &_arch_idle_task;
	else return sched_find_pid(pid);
}


process_t *process_alloc() {
	if(_process_number < CONFIG_PROC_MAX) {
		process_t *proc;

		proc = pool_alloc(&_proc_pool);
		if(proc != NULL) {
			int i;

			for(i=0; i<PROCESS_MAX_FILE; i++) {
				proc->files[i] = NULL;
				proc->fdflags[i] = 0;
			}
			proc->pid = process_get_pid();
			proc->ppid = 0;
			proc->state = PROCESS_STATE_CREATE;
			proc->asid = ASID_INVALID;

			proc->dir_list = NULL;

			sigemptyset(& proc->sig_blocked);
			sigemptyset(& proc->sig_pending);
			for(i=0; i<SIGNAL_INDEX_MAX; i++) {
				proc->sig_array[i].sa_handler = SIG_DFL;
				proc->sig_array[i].sa_flags = 0;
			}

			proc->uticks = 0;
			proc->kticks = 0;

			proc->initial_brk = NULL;
			proc->current_brk = NULL;

#ifdef CONFIG_ELF_SHARED
			proc->shared.file = NULL;
#endif //CONFIG_ELF_SHARED

			list_push_front(&_process_list, & proc->list);
			_process_number++;
		}
		return proc;
	}
	
	printk("process_alloc: no more allowed process\n");
	return NULL;
}


void process_free(process_t *proc) {
	list_remove(&_process_list, & proc->list);
	process_release_pid(proc->pid);
	_process_number--;

	pool_free(&_proc_pool, proc);
	printk("free proc %p\n", proc);
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
	
	int dummy;
	interrupt_atomic_save(&dummy);

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

// the scheduler must check for state
	proc->state = PROCESS_STATE_RUNNING;

	// if the process to "restore" was in user mode, check for pending signals
	if(arch_process_mode(proc) == 1) {
		signal_deliver_pending();
	}

	arch_kernel_contextjmp(proc->acnt, &(proc->acnt));
}



void process_terminate(process_t *proc, int status) {
	struct page_dir *curdir;
	struct page_dir *nextdir;
	int i;

	// close openned files
	for(i=0; i<PROCESS_MAX_FILE; i++) {
		if(proc->files[i] != NULL) {
			vfs_close(proc->files[i]);
			proc->files[i] = NULL;
		}
	}
	
	// remove all virtual pages from the TLB (invalidate them)
	// TODO do not invalidate ALL entries, select only this ASID
	// in addition, free each allocated physical pages
	mmu_tlbflush();
	for(curdir = proc->dir_list; curdir != NULL; curdir = nextdir) {
		nextdir = curdir->next;
		mem_release_dir(curdir);
	}
	proc->dir_list = NULL;
	
	// VM is not used after, but the process need to have an ASID until
	// it will be remove from Zombie list.

	proc->exit_status = status;
	proc->state = PROCESS_STATE_ZOMBIE;
	// TODO call scheduler to remove it from waiting queue...

	// do not free the kernel stack before wait() is called to be sure
	// we can still execute code in case of re-execution of zombie process
	while(1) {
		sched_schedule();
		printk("exit: exited process executed!\n");
	}

}



pid_t sys_fork() {
	process_t *cur;
	process_t *newproc;
	int i;
	int atomicsaved;
	struct page_dir *dir;	


	cur = process_get_current();
	// do fork only if this is the first context-switch of the process
	// (this should be the case here, but it's useful for debug)
	if(cur->acnt->previous != NULL) {
		printk("fork: multiple context... aborted\n");
		return -1;
	}
	

	// we need to block any exception, fork operation should be atomic
	interrupt_atomic_save(&atomicsaved);

	printk("fork start\n");

	// alloc a new process, and copy everything
	newproc = process_alloc();

	newproc->ppid = cur->pid;
	for(i=0; i<PROCESS_MAX_FILE; i++) {
		// for each valid file descriptor, increment usage counter
		newproc->files[i] = cur->files[i];
		newproc->fdflags[i] = cur->fdflags[i];
		if(newproc->files[i] != NULL)
			newproc->files[i]->count++;
	}

	// copy each memory page with same virtual addresses
	// TODO copy-on-write system!
	for(dir = cur->dir_list; dir != NULL; dir = dir->next) {
		// stupid implementation : copy each page if valid
		for(i=0; i<MEM_DIRECTORY_PAGES; i++) {
			if((dir->pages[i].private.flags & MEM_PAGE_PRIVATE)
					&& (dir->pages[i].private.flags & MEM_PAGE_VALID) )
			{
				mem_copy_page(&dir->pages[i], &newproc->dir_list,
						MEM_PAGE_ADDRESS(dir->dir_id, i));
			}
			// TODO shared pages
		}
	}

	// copy signal info
	sigemptyset(& newproc->sig_pending);
	newproc->sig_blocked = cur->sig_blocked;
	for(i=0; i<SIGNAL_INDEX_MAX; i++) {
		newproc->sig_array[i] = cur->sig_array[i];
	}
	
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
	interrupt_atomic_restore(atomicsaved);

	return newproc->pid;

	/*printk("preempt_fork returned %d\n", val);
	while(!hwkbd_real_keydown(K_EXE));
	while(hwkbd_real_keydown(K_EXE));*/
}




void sys_exit(int status) {
	process_terminate(process_get_current(), _WSTATUS_EXITS(status));
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
	if(elf_inode == NULL || (elf_file = vfs_open(elf_inode, O_RDONLY)) == NULL ) {
		printk("execve: failed to open '%s'\n", filename);
	}
	else {
		process_t *cur;
		int i;

		// we need to copy argv[] and env[] somewhere before to destroy process
		// virtual memory...
		void *args_page;
		size_t args_pos;

		// values to be used as main() arguments
		int arg_argc;
		void *arg_argv;

		union pm_page page;

		sched_preempt_block();
		cur = process_get_current();


		args_page = mem_pm_get_free_page(MEM_PM_CACHED);
		if(args_page == NULL) {
			printk("execve: not enought memory\n");
			// TODO abort
			while(1);
		}
		page.private.ppn = PM_PHYSICAL_PAGE(args_page);
		page.private.flags = MEM_PAGE_PRIVATE | MEM_PAGE_VALID | MEM_PAGE_CACHED;
		// do not map the page now, the process old address space must be cleaned

		args_pos = 0;
		// copy argv
		if(argv != NULL) {
			// compute number of arguments
			int nbargs;
			char **args_array = args_page;
			for(nbargs=0; argv[nbargs] != NULL; nbargs++);

			// space for argument pointer array
			args_pos = nbargs * sizeof(char*);

			for(i=0 ; i<nbargs; i++) {
				char *copied_arg = args_page + args_pos;
				size_t curarg_size;
				for(curarg_size = 0; argv[i][curarg_size] != '\0'; curarg_size++);
				curarg_size++;

				// TODO check max size
				strcpy(copied_arg, argv[i]);
				// we want to store the VM address (not physical one) :
				args_array[i] = (void*)(((unsigned int)copied_arg % PM_PAGE_BYTES)
						+ ((unsigned int)ARCH_UNEWPROC_DEFAULT_ARGS)) ;

				args_pos += curarg_size;
			}

			arg_argc = nbargs;
			arg_argv = (void*)(((unsigned int)args_array % PM_PAGE_BYTES)
				+ ((unsigned int)ARCH_UNEWPROC_DEFAULT_ARGS)) ;
		}
		else {
			arg_argc = 0;
			arg_argv = NULL;
		}

		// close/free all ressources not 'shared' through exec
		for(i=0; i<PROCESS_MAX_FILE; i++) {
			if(cur->files[i] != NULL && (cur->fdflags[i] & FD_CLOEXEC))
					vfs_close(cur->files[i]);
		}
		
		// signal handler are reseted to SIG_DFL, except in case of SIG_IGN
		for(i=0; i<SIGNAL_INDEX_MAX; i++) {
			if(cur->sig_array[i].sa_handler != SIG_IGN)
				cur->sig_array[i].sa_handler = SIG_DFL;
		}

		// remove all virtual pages from the TLB (invalidate them)
		// TODO do not invalidate ALL entries, select only this ASID
		// in addition, free each allocated physical pages
		mmu_tlbflush();

		struct page_dir *curdir;
		struct page_dir *nextdir;
		for(curdir = cur->dir_list; curdir != NULL; curdir = nextdir) {
			nextdir = curdir->next;
			mem_release_dir(curdir);
		}
		cur->dir_list = NULL;

		// TODO do not release kernel stack, ELF loader should not set it?
		void *old_kstack = cur->kernel_stack;
		
		cur->state = PROCESS_STATE_CREATE;
		if(elfloader_load(elf_file , cur) != 0) {
			printk("execve: unable to load ELF file\n");
			// 'kill' process TODO proper way to do that
			cur->state = PROCESS_STATE_CREATE;

			// does sched_schedule() should reset preempt count?
			sched_preempt_unblock();
			sched_schedule(cur);

			printk("execve: re-executed dead process!\n");
		}
		else {
			// the image is load, we can't simply return from syscall because
			// stack is now 'corrupted' (old_kstack is the real stack used, but
			// it's not the current proc kernel stack...)
			int dummy;

			// add virtual memory page for args
			mem_insert_page(& cur->dir_list , &page, (void*)ARCH_UNEWPROC_DEFAULT_ARGS);
			
			cur->acnt->reg[4] = arg_argc;
			cur->acnt->reg[5] = (uint32)arg_argv;

			// interrupt context is expected to be reseted by process_contextjmp() ?
			interrupt_atomic_save(&dummy);
			sched_preempt_unblock();

			printk("exec: ready, r15=%p\n", (void*)(cur->acnt->reg[15]));
			// this job is done using inline assembly to avoid GCC stack usage
			 asm volatile (
					"mov %0, r15;"
					"mov %1, r0;"
					"mov %2, r4;"
					"jsr @r0;"
					"nop;"
					"mov %3, r4;"
					"mov %4, r0;"
					"jmp @r0;"
					"nop;" : : "r"(cur->kernel_stack - sizeof(*(cur->acnt))), "r"(&mem_pm_release_page),
							"r"(old_kstack-1), "r"(cur), "r"(&process_contextjmp)
							: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7" );

			printk("execve: this should not happen!\n");
		}
	}

	return -1;
}


void *sys_sbrk(int incr) {
	// current implementation is pretty simple (no check for stack/shared area)
	process_t *cur;

	cur = process_get_current();
	if(cur->initial_brk != NULL) {
		void *ret;

		if(cur->current_brk == NULL)
			cur->current_brk = cur->initial_brk;
		ret = cur->current_brk;

		printk("sbrk: incr=%d\n", incr);

		// add or remove the given size
		if(incr > 0) {
			// add pages to process if needed
			unsigned int curalign;
			void *lastpos;

			lastpos = cur->current_brk - 1;
			curalign = (((unsigned int) lastpos)) % PM_PAGE_BYTES;
			if(curalign + incr >= PM_PAGE_BYTES) {
				union pm_page page;
				void *curvm;
				int relincr;

				relincr = incr - (PM_PAGE_BYTES - curalign);
				curvm = lastpos + (PM_PAGE_BYTES - curalign);

				page.private.flags = MEM_PAGE_PRIVATE | MEM_PAGE_VALID | MEM_PAGE_CACHED;
				while(relincr >= 0) {
					void *pageaddr;

					printk("sbrk: add page @%p\n", curvm);
					pageaddr = mem_pm_get_free_page(MEM_PM_CACHED);
					if(pageaddr == NULL) {
						// FIXME clean before return
						return (void*)-1;
					}

					page.private.ppn = PM_PHYSICAL_PAGE(pageaddr);
					mem_insert_page(& cur->dir_list , &page, curvm);

					relincr -= PM_PAGE_BYTES;
					curvm += PM_PAGE_BYTES;
				}
			}
		}

		else if(incr < 0) {
			// remove pages if possible
			int curalign;

			// impossible to reduce the size beyond the original heap begin addr
			incr = cur->current_brk + incr <= cur->initial_brk ?
				cur->initial_brk - cur->current_brk : incr ;

			curalign = (((unsigned int) cur->current_brk) - 1) % PM_PAGE_BYTES;
			if(curalign + incr < 0 ) {
				union pm_page *page;
				void *curvm;
				int nbpages;

				nbpages = (-incr - ((unsigned int)(cur->current_brk) % PM_PAGE_BYTES)
						-1) / PM_PAGE_BYTES + 1;
				curvm = cur->current_brk - ((unsigned int)(cur->current_brk)
						% PM_PAGE_BYTES);

				while(nbpages > 0) {
					printk("sbrk: remove page @%p\n", curvm);

					page = mem_find_page(cur->dir_list, curvm);
					if(page != NULL) {
						mem_release_page(page);
					}

					nbpages--;
					curvm -= PM_PAGE_BYTES;
				}
			}
		}

		cur->current_brk += incr;
		return ret;
	}
	return (void*)-1;
}



pid_t process_get_pid() {
	pid_t ret;

	// get the next PID from _pid_next, and check for the first free PID by
	// wrapping around CONFIG_PID_MAX maximum value
	
	// TODO issue if CONFIG_PROC_MAX >= CONFIG_PID_MAX
	for(ret = _pid_next; bitfield_get(_pid_used, ret);
			ret = (ret+1) % CONFIG_PID_MAX);
	
	_pid_next = (ret+1) % CONFIG_PID_MAX;
	bitfield_set(_pid_used, ret);
	return ret;
}


void process_release_pid(pid_t pid) {
	if(pid >= 0 && pid < CONFIG_PID_MAX)
		bitfield_clear(_pid_used, pid);
}


/**
 * Process-related sysctls
 */

static void copy_proc_user(process_t *proc, void *userbuf) {
	struct proc_uinfo *uinfo = (struct proc_uinfo*)userbuf;

	uinfo->cpu_usage = load_proc_average(proc);
	uinfo->kticks = proc->kticks;
	uinfo->uticks = proc->uticks;

	uinfo->pid = proc->pid;
	uinfo->ppid = proc->ppid;

	uinfo->state = proc->state;
	uinfo->exit_status = proc->exit_status;
}

static SYSCTL_OBJECT(ctl__proc) = {
	.parent = &ctl__kern,
	.name = "proc",
	.id = KERN_PROC,
	.type = CTL_TYPE_NODE
};

static int access_proc_pid(void *oldbuf, size_t *oldlen, const void *newbuf,
		size_t newlen, int index)
{
	if(oldlen != NULL) {
		process_t *proc;
		proc = process_from_pid((pid_t)index);

		if(proc != NULL && *oldlen >= sizeof(struct proc_uinfo) ) {
			*oldlen = sizeof(struct proc_uinfo);
			if(oldbuf != NULL) {
				copy_proc_user(proc, oldbuf);
				return 0;
			}
		}
		*oldlen = sizeof(struct proc_uinfo);
	}
	return -1;
}


static SYSCTL_OBJECT(ctl_proc_pid) = {
	.parent = &ctl__proc,
	.name = "pid",
	.id = KERN_PROC_PID,
	.type = CTL_TYPE_OPAQUE_NDX,
	.access.indexed = &access_proc_pid
};



static int access_proc_all(void *oldbuf, size_t *oldlen, const void *newbuf,
		size_t newlen)
{
	if(oldlen != NULL) {
		size_t size;

		// TODO lock processes list and _process_number during this
		size = _process_number * sizeof(struct proc_uinfo);
		if(*oldlen >= size) {
			*oldlen = size;
			if(oldbuf != NULL) {
				struct proc_uinfo *cur_uinfo;
				struct list_head *cur;

				cur_uinfo = (struct proc_uinfo*)oldbuf;
				list_for_each(cur, &_process_list) {
					copy_proc_user(container_of(cur, process_t, list), cur_uinfo);
					cur_uinfo++;
					if((void*)cur_uinfo > (oldbuf+size))
						break;
				}
				return 0;
			}
		}
		// if maximum size is too small, lie about the needed size to ensure
		// the next call have small chances to fail if new processes are added
		*oldlen = size + sizeof(struct proc_uinfo);
	}
	return -1;
}


static SYSCTL_OBJECT(ctl_proc_all) = {
	.parent = &ctl__proc,
	.name = "all",
	.id = KERN_PROC_ALL,
	.type = CTL_TYPE_OPAQUE,
	.access.data = &access_proc_all
};

