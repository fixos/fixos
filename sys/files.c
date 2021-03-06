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
	struct process *proc;
	int fd;

	//printk(LOG_DEBUG, "Received sys_open :\n   ('%s', %d)\n", name, flags);
	proc = process_get_current();

	for(fd=0; fd<PROCESS_MAX_FILE && proc->files[fd] != NULL; fd++);
	if(fd < PROCESS_MAX_FILE) {
		struct inode *inode;

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
				//printk(LOG_DEBUG, "sys_open: new fd = %d\n", fd);

				// done
				return fd;
			}
			else {
				printk(LOG_DEBUG, "sys_open: unable to open ('%s', f=0x%x)\n", name, flags);
			}
		}
		else {
			printk(LOG_DEBUG, "sys_open: no such inode found\n");
		}
	}
	else {
		printk(LOG_WARNING, "sys_open: no more file desc\n");
	}
	return -1;
}



ssize_t sys_read(int fd, char *dest, int nb) {
	struct process *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_read(proc->files[fd], dest, nb);	
	}
	else {
		printk(LOG_DEBUG, "sys_read: invalid fd\n");
		return -EBADF;
	}	
}



ssize_t sys_write(int fd, const char *source, int nb){
	struct process *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_write(proc->files[fd], source, nb);	
	}
	else {
		printk(LOG_DEBUG, "sys_write: invalid fd (%d)\n", fd);
	}	

	return -EBADF;
}

int sys_ioctl(int fd, int request, void *arg) {
	struct process *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_ioctl(proc->files[fd], request, arg);	
	}
	else {
		printk(LOG_DEBUG, "sys_ioctl: invalid fd (%d)\n", fd);
	}	

	return -1;
}


int sys_pipe2(int pipefd[2], int flags) {
	struct process *proc;
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
			printk(LOG_ERR, "sys_pipe2: pipe creation failed\n");
		}
	}
	else {
		printk(LOG_WARNING, "sys_pipe2: no more file desc\n");
	}
	return -1;

}


int sys_lseek(int fd, off_t offset, int whence) {
	struct process *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_lseek(proc->files[fd], offset, whence);	
	}
	else {
		printk(LOG_DEBUG, "sys_lseek: invalid fd\n");
	}

	return -1;
}


int sys_fstat(int fd, struct stat *buf) {
	struct process *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_fstat(proc->files[fd], buf);	
	}
	else {
		printk(LOG_DEBUG, "sys_fstat: invalid fd\n");
	}

	return -1;
}


int sys_stat(const char *path, struct stat *buf) {
	struct inode *target = vfs_resolve(path);

	if(target != NULL) {
		if(target->fs_op != NULL && target->fs_op->fs->iop.istat != NULL) {
			return target->fs_op->fs->iop.istat(target, buf);
		}
		vfs_release_inode(target);
	}
	return -1;
}



int sys_getdents(int fd, struct fixos_dirent *buf, size_t len) {
	struct process *proc;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		return vfs_dir_getdents(proc->files[fd], buf, len);
	}
	else {
		printk(LOG_DEBUG, "sys_getdents: invalid fd\n");
		return -EBADF;
	}	
}


int sys_close(int fd) {
	struct process *proc;
	int ret;

	proc = process_get_current();
	if(fd>=0 && fd<PROCESS_MAX_FILE && proc->files[fd] != NULL) {
		ret = vfs_close(proc->files[fd]);
		proc->files[fd] = NULL;
	}
	else {
		printk(LOG_DEBUG, "sys_close: invalid fd\n");
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
