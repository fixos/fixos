#include "files.h"
#include <utils/log.h>
#include <sys/process.h>
#include <fs/file.h>
#include <fs/vfs_file.h>
#include <fs/vfs.h>
#include <fs/pipe.h>



int sys_open(const char *name, int mode) {
	process_t *proc;
	int fd;

	printk("Received sys_open :\n   ('%s', %d)\n", name, mode);
	proc = process_get_current();

	for(fd=0; fd<PROCESS_MAX_FILE && proc->files[fd] != NULL; fd++);
	if(fd < PROCESS_MAX_FILE) {
		inode_t *inode;

		// get the inode associated to file name, and open it
		inode = vfs_resolve(name);
		if(inode != NULL) {
			
			proc->files[fd] = vfs_open(inode);
			if(proc->files[fd] != NULL) {
				printk("sys_open: new fd = %d\n", fd);

				// done
				return fd;
			}
			else {
				printk("sys_open: unable to open\n");
			}
		}
		else {
			printk("sys_open: no such inode found\n");
		}
	}
	else {
		printk("sys_open: no more file desc\n");
	}
	return -1;
}



ssize_t sys_read(int fd, char *dest, int nb) {
	process_t *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_read(proc->files[fd], dest, nb);	
	}
	else {
		printk("sys_read: invalid fd\n");
	}	

	return -1;
}



ssize_t sys_write(int fd, const char *source, int nb){
	process_t *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_write(proc->files[fd], source, nb);	
	}
	else {
		printk("sys_write: invalid fd (%d)\n", fd);
	}	

	return -1;
}

int sys_ioctl(int fd, int request, void *arg) {
	process_t *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_ioctl(proc->files[fd], request, arg);	
	}
	else {
		printk("sys_ioctl: invalid fd (%d)\n", fd);
	}	

	return -1;
}


int sys_pipe2(int pipefd[2], int flags) {
	process_t *proc;
	int fdin, fdout;

	(void)flags;

	proc = process_get_current();
	for(fdin=0; fdin<PROCESS_MAX_FILE && proc->files[fdin] != NULL; fdin++);
	for(fdout=fdin+1; fdout<PROCESS_MAX_FILE && proc->files[fdout] != NULL; fdout++);

	if(fdin < PROCESS_MAX_FILE && fdout < PROCESS_MAX_FILE) {
		struct file *files[2];
		if(pipe_create(files) == 0) {
			proc->files[fdin] = files[0];
			pipefd[0] = fdin;
			proc->files[fdout] = files[1];
			pipefd[1] = fdout;

			return 0;
		}
		else {
			printk("sys_pipe2: pipe creation failed\n");
		}
	}
	else {
		printk("sys_pipe2: no more file desc\n");
	}
	return -1;

}
