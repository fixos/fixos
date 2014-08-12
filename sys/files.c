#include "files.h"
#include <utils/log.h>
#include <sys/process.h>
#include <fs/file.h>
#include <fs/vfs_file.h>
#include <fs/vfs.h>
#include <fs/pipe.h>
#include <interface/fixos/errno.h>
#include <fs/vfs_directory.h>



int sys_open(const char *name, int flags) {
	process_t *proc;
	int fd;

	//printk("Received sys_open :\n   ('%s', %d)\n", name, flags);
	proc = process_get_current();

	for(fd=0; fd<PROCESS_MAX_FILE && proc->files[fd] != NULL; fd++);
	if(fd < PROCESS_MAX_FILE) {
		inode_t *inode;

		// get the inode associated to file name, and open it
		inode = vfs_resolve(name);
		if(inode != NULL) {
			
			// O_CLOEXEC flag should not be seen by vfs level
			if(flags & O_CLOEXEC) {
				flags &= ~O_CLOEXEC;
				proc->fdflags[fd] = FD_CLOEXEC;
			}
			else {
				proc->fdflags[fd] = 0;
			}

			proc->files[fd] = vfs_open(inode, flags);
			if(proc->files[fd] != NULL) {
				//printk("sys_open: new fd = %d\n", fd);

				// done
				return fd;
			}
			else {
				printk("sys_open: unable to open ('%s', f=0x%x)\n", name, flags);
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
		return -EBADF;
	}	
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

	return -EBADF;
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


int sys_lseek(int fd, off_t offset, int whence) {
	process_t *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_lseek(proc->files[fd], offset, whence);	
	}
	else {
		printk("sys_lseek: invalid fd\n");
	}

	return -1;
}


int sys_fstat(int fd, struct stat *buf) {
	process_t *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_fstat(proc->files[fd], buf);	
	}
	else {
		printk("sys_fstat: invalid fd\n");
	}

	return -1;
}


int sys_stat(const char *path, struct stat *buf) {
	inode_t *target = vfs_resolve(path);

	if(target != NULL) {
		if(target->fs_op != NULL && target->fs_op->fs->iop.istat != NULL) {
			return target->fs_op->fs->iop.istat(target, buf);
		}
		vfs_release_inode(target);
	}
	return -1;
}



int sys_getdents(int fd, struct fixos_dirent *buf, size_t len) {
	process_t *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_dir_getdents(proc->files[fd], buf, len);
	}
	else {
		printk("sys_getdents: invalid fd\n");
		return -EBADF;
	}	
}


int sys_close(int fd) {
	process_t *proc;
	int ret;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		ret = vfs_close(proc->files[fd]);
		proc->files[fd] = NULL;
	}
	else {
		printk("sys_close: invalid fd\n");
		ret = -EBADF;
	}

	return ret;
}


int sys_dup(int oldfd) {
	int fd;

	for(fd=0; fd<PROCESS_MAX_FILE && _proc_current->files[fd] != NULL; fd++);
	if(fd >= PROCESS_MAX_FILE)
		return -EMFILE;

	if(oldfd < 0 || oldfd >= PROCESS_MAX_FILE)
		return -EBADF;

	// duplicate fds and clear FD_CLOEXEC flag in new descriptor
	_proc_current->fdflags[fd] = 0;
	_proc_current->files[fd] = _proc_current->files[oldfd];

	return fd;
}


int sys_dup2(int oldfd, int newfd) {
	if(oldfd < 0 || oldfd >= PROCESS_MAX_FILE || newfd < 0
			|| newfd >= PROCESS_MAX_FILE)
	{
		return -EBADF;
	}

	if(_proc_current->files[oldfd] == NULL)
		return -EBADF;

	if(oldfd != newfd) {
		// close newfd file if needed, then duplicate
		if(_proc_current->files[newfd] != NULL)
			vfs_close(_proc_current->files[newfd]);

		_proc_current->fdflags[newfd] = 0;
		_proc_current->files[newfd] = _proc_current->files[oldfd];
	}

	return newfd;
}
