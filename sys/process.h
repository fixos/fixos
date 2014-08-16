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


// process status
#define PROCESS_STATE_RUNNING			2
// used at process creation :
#define PROCESS_STATE_CREATE			3
// used after calling exit()
#define PROCESS_STATE_ZOMBIE			5
// sleeping states
#define PROCESS_STATE_STOPPED			4
#define PROCESS_STATE_INTERRUPTIBLE		6
#define PROCESS_STATE_UNINTERRUPTIBLE	7



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

struct tty;

struct process {
	// address on the kernel stack of the process, or NULL if running out of any
	// interrupt...
	struct _context_info *acnt;

	void *kernel_stack;

	pid_t pid;
	pid_t pgid;

	// virtual memory managing data :
	struct addr_space addr_space;
	struct page_dir *dir_list;

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

	pid_t ppid;
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
	// the current_brk is the current address of the top of the heap (changed
	// by sbrk() )
	void *current_brk;

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
extern inline struct process *process_get_current() {
	return _proc_current;
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
