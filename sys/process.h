/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SYS_PROCESS_H
#define _SYS_PROCESS_H

/**
 * Process generic manipulation.
 */

#include <utils/types.h>
#include <sys/signal.h>
#include <interface/fixos/process.h>
#include <sys/memory.h>
#include <utils/list.h>
#include <fs/inode.h>

#include <arch/process.h>
#include <arch/memory.h>


// maximum files opened at a time by a process
#define PROCESS_MAX_FILE		12

struct file;

// information for saving/restoring context
// WARNING : this structure is arch-dependant, and MUST be the *first* field
// of process struct (see arch/sh/scheduler.S for example)
struct _context_info;

#ifdef CONFIG_ELF_SHARED
// loaded library associated to a process
struct elf_shared_lib {
	struct file *file;
	void *offset;
};
#endif //CONFIG_ELF_SHARED

// for per-process CPU load calculation (should be power of 2 for optimization!)
#define PROC_LOAD_SAMPLES	4
#define PROC_LOAD_SAMPMAX	64

// various definitions related to processes :
// default *maximum* user stack size (should be allocated when needed)
#define PROCESS_DEFAULT_STACK_SIZE		(64*1024)

// maximum pages allowed for execve's argv and envp data...
#define PROCESS_ARG_MAX_PAGES	4


struct tty;
struct mem_area;

struct process {
	// address on the kernel stack of the process, or NULL if running out of any
	// interrupt...
	struct _context_info *acnt;

	void *kernel_stack;

	pid_t pid;
	pid_t pgid;

	// parent, or NULL (no other process than swaper and init should use NULL)
	struct process *parent;

	// virtual memory managing data :
	struct addr_space addr_space;
	struct page_dir *dir_list;
	// address space areas (see sys/mem_area.h)
	struct list_head mem_areas;

	// files opened by process, index is file descriptor
	struct file *files[PROCESS_MAX_FILE];
	// corresponding file descriptor flags (currently only FD_CLOEXEC is used)
	char fdflags[PROCESS_MAX_FILE];

	// current working directory
	struct inode *cwd;
	
	// signal related stuff
	sigset_t sig_blocked;
	sigset_t sig_pending;
	struct sigaction sig_array[SIGNAL_INDEX_MAX];

	int state;
	int exit_status; // only valid when state is PROCESS_STATE_ZOMBIE

	// clock ticks ellapsed in User mode and in Kernel mode
	clock_t uticks;
	clock_t kticks;

	// shared libraries loaded
	// TODO better design
#ifdef CONFIG_ELF_SHARED
	struct elf_shared_lib shared;
#endif //CONFIG_ELF_SHARED

	// initial data break address (end of static .data/.bss)
	void *initial_brk;
	// this is the pointer to the main heap memory area (manipulated by sbrk())
	// be careful : this area *should* exists in process mem_areas list!
	struct mem_area *heap_area;

	// for process cpu load (should not be used directly, see cpu_load.h)
	uint8 load_cursamp;
	uint8 load_samples[PROC_LOAD_SAMPLES];
	clock_t load_last;

	// linked list used to represent every processes
	struct list_head list;

	// controlling terminal if any
	struct tty *ctty;
};

// list of all processes
extern struct list_head _process_list;

extern struct process *_proc_current;

/**
 * Process used as "idle task", executed once scheduler is stared, when no other
 * task should be done.
 * As the scheduler is initialized after all the early init job, the stack
 * used may be the kernel init stack.
 */
extern struct process _proc_idle_task;

// iterate for each process in _process_list
#define for_each_process(cur) \
	for(cur = container_of(&_process_list, struct process, list); \
	   	(cur = container_of(cur->list.next, struct process, list)) \
			!= container_of(& _process_list, struct process, list) ; )

// Init process manager
void process_init();

/**
 * Get the process from its PID (NULL if corresponding process doesn't exist)
 */
struct process *process_from_pid(pid_t pid);


/**
 * Get the next free PID, and mark it as "used".
 */
pid_t process_get_pid();

/**
 * Release a PID for a future usage.
 */
void process_release_pid(pid_t pid);


/**
 * Alloc a new process in the process list, returns NULL if an error occurs.
 * In success case, the returned struct process is not fully initialized (pid is an
 * auto-incremented value, vm table is zeroed (but *not* freed), and the status
 * is set to PROCESS_STATE_CREATE)
 */
struct process *process_alloc();

/**
 * Free a process allocated using process_alloc().
 * WARNING : this function do not do any other job, the caller must free
 * unneeded pages, files, and any other things.
 */
void process_free(struct process *proc);


/**
 * Release the ASID of current process if not already ASID_INVALID.
 * This can be used if the process has terminated, or if we now it will
 * not have to be executed during a long time.
 */
//void process_release_asid(struct process *proc);


/**
 * Return the running process at the time this function is called.
 */
static inline struct process *process_get_current() {
	return _proc_current;
}


/**
 * Return the pid of the parent process, or 0 if not available.
 */
static inline pid_t process_get_ppid() {
	return _proc_current->parent == NULL ? 0 : _proc_current->parent->pid;
}


/**
 * Run given process, switching context from current kernel context to
 * process context_info.
 * ASID and other things are changed before the real context jump is done.
 */
void process_contextjmp(struct process *proc);


/**
 * Terminate a process (becoming a zombie), and store the given status as the
 * error/exits value returned by kill()-like syscalls.
 */
void process_terminate(struct process *proc, int status);

/**
 * Check if a given process is a descendant of process with given pid (child of
 * this process, or child of a descendant of this process).
 */
int process_is_descendant(struct process *proc, pid_t other);

/**
 * fork() syscall implementation, duplicate any memory area, create a kernel stack
 * and prepare the child process to be executed by scheduler.
 */
pid_t sys_fork();


/**
 * exit() syscall implementation, signal the end of the execution of a process.
 * After this call, the process is indicated as Zombie, and will wait for a
 * wait() call from it's parent process before to be realy removed from
 * existing tasks.
 */
void sys_exit(int status);

pid_t sys_getpid();

pid_t sys_getppid();

void *sys_sbrk(int incr);


/**
 * execve syscall implementation...
 */
int sys_execve(const char *filename, char *const argv[], char *const envp[]);

/**
 * chdir()/fchdir() syscalls, change the current working directory of the calling
 * process
 */
int sys_chdir(const char *path);

int sys_fchdir(int fd);


int sys_setpgid(pid_t pid, pid_t pgid);

pid_t sys_getpgid(pid_t pid);

#endif //_SYS_PROCESS_H
