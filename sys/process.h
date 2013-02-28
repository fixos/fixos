#ifndef _SYS_PROCESS_H
#define _SYS_PROCESS_H

/**
 * Process managing and difinitions.
 */

#include "types.h"
#include <arch/sh/virtual_memory.h>

// maximum ASID number at one time
#define MAX_ASID	32

// tmp TODO real implementation of virtual memory
struct _virtual_mem;


// process status
#define PROCESS_IDLE	1
#define PROCESS_RUN		2
// used at process creation :
#define PROCESS_STOPED	3


struct _process_info {
	pid_t pid;
	asid_t asid;

	// virtual memory managing data :
	vm_table_t vm;
	
	// information for saving/restoring context : 
	void *stack_addr;
	void *pc_addr;

	void *base_stack;

	int state;
};

typedef struct _process_info process_t;


/**
 * Get the process informations from its ASID
 */
process_t *process_from_asid(unsigned char asid);

#endif //_SYS_PROCESS_H
