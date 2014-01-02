#include "process.h"
#include <arch/sh/mmu.h>


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

// test for user process...
static process_t _test_proc;
static int _test_proc_used = 0;


void process_init()
{
	int i;
	for(i=0; i<MAX_ASID; i++) {
		_asid_proc_array[i] = NULL;
	}
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
	if(!_test_proc_used) {
		int i;

		for(i=0; i<PROCESS_MAX_FILE; i++)
			_test_proc.files[i] = NULL;
		_test_proc.pid = _pid_next_value++;
		_test_proc.state = PROCESS_STATE_CREATE;
		_test_proc_used = 1;
		return &_test_proc;
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
