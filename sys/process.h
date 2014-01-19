#ifndef _SYS_PROCESS_H
#define _SYS_PROCESS_H

/**
 * Process generic manipulation.
 */

#include <utils/types.h>
#include <arch/sh/virtual_memory.h>
#include <arch/sh/process.h>

// maximum ASID number at one time
#define MAX_ASID	32

// ASID for 'not valid ASID'
#define ASID_INVALID	0xFE

// tmp TODO real implementation of virtual memory
struct _virtual_mem;


// process status
#define PROCESS_STATE_IDLE		1
#define PROCESS_STATE_RUN		2
// used at process creation :
#define PROCESS_STATE_CREATE	3
#define PROCESS_STATE_STOP		4


// maximum files opened at a time by a process
#define PROCESS_MAX_FILE		5

struct file;

struct _process_info {
	// information for saving/restoring context
	// WARNING : this structure is arch-dependant, and MUST be the *first* field
	// of process struct (see arch/sh/scheduler.S for example)
	struct _context_info acnt;

	pid_t pid;
	asid_t asid;

	// virtual memory managing data :
	vm_table_t vm;

	// files opened by process, index is file descriptor
	struct file *files[PROCESS_MAX_FILE];
	

	int state;
};

typedef struct _process_info process_t;

/**
 * Used for mode switching (user -> kernel)
 * The exception handler check if previous context was in kernel mode (SSR.MD = 1)
 * If it was in user mode, g_process_current_kstack is used as new stack.
 * (nothing happen and r15 is keeped if previous context was kernel mode)
 */
extern void * g_process_current_kstack;


// Init process manager
void process_init();

/**
 * Get the process informations from its ASID (NULL if not found)
 */
process_t *process_from_asid(asid_t asid);

/**
 * Get the process from its PID (NULL if corresponding process doesn't exist)
 */
process_t *process_from_pid(pid_t pid);


/**
 * Alloc a new process in the process list, returns NULL if an error occurs.
 * In success case, the returned process_t is not fully initialized (pid is an
 * auto-incremented value, asid is an invalid ASID, vm table is zeroed (but *not*
 * freed), and the status is set to PROCESS_STATE_CREATE)
 */
process_t *process_alloc();


/**
 * Get a unused ASID and set it to the asid field of given proc.
 * Return a negative value if error occurs.
 */
int process_set_asid(process_t *proc);


/**
 * Return the running process at the time this function is called.
 */
process_t *process_get_current();


/**
 * Run given process, switching context from current kernel context to
 * process context_info.
 * ASID and other things are changed before the real context jump is done.
 */
void process_contextjmp(process_t *proc);

#endif //_SYS_PROCESS_H
