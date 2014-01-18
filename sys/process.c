#include "process.h"
#include <arch/sh/mmu.h>
#include <sys/memory.h>

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
		proc->state = PROCESS_STATE_CREATE;
		proc->asid = ASID_INVALID;
		vm_init_table(&(proc->vm));

		// set first free to next one
		_first_free = next;
		return proc;
	}
	return NULL;
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


process_t *process_get_current() {
	return process_from_asid(mmu_getasid());
}



void process_contextjmp(process_t *proc) {
	// if ASID is not valid (first contextjmp, or process was remove from 'active'
	// process, we need to set it's ASID before to run it
	if(proc->asid == ASID_INVALID)
		process_set_asid(proc);

	// just before context jump, set MMU current ASID to process' ASID
	mmu_setasid(proc->asid);
	printk("[D] ASID = %d\n", mmu_getasid());
	printk("asid=%d, pid=%d\n", proc->asid, proc->pid);

	arch_kernel_contextjmp(&(proc->acnt));
}
