#include "pipe.h"
#include "file.h"
#include "vfs_file.h"
#include <utils/cyclic_fifo.h>
#include <sys/memory.h>
#include <sys/scheduler.h>


/**
 * For now, the implementation use a single physical page, which store the
 * "cyclic fifo" data structure and its buffer.
 */


struct pipe_page {
	struct cyclic_fifo fifo;
	// usage counter, free the allocated page when 0 is reached
	int count;
	char buffer[PM_PAGE_BYTES - sizeof(struct cyclic_fifo) - sizeof(int)];
};


struct file_operations pipe_file_ops = {
	.read = &pipe_read,
	.write = &pipe_write,
	.release = &pipe_release
};



int pipe_create(struct file *files[2]) {
	struct file *in;
	struct file *out;
	struct pipe_page *page;

	// get the two file struct
	in = vfs_file_alloc();
	out = vfs_file_alloc();
	page = arch_pm_get_free_page(MEM_PM_CACHED);
	
	if(in == NULL || out == NULL || page == NULL) {
		if(in != NULL)
			vfs_file_free(in);
		if(out != NULL)
			vfs_file_free(out);
		if(page != NULL)
			arch_pm_release_page(page);

		return -1;
	}

	page->fifo.size = 0;
	page->fifo.max_size = sizeof(page->buffer);
	page->fifo.buffer = page->buffer;
	page->fifo.top = 0;

	page->count = 2;

	in->inode = NULL;
	in->pos = 0;
	in->flags = O_RDONLY;
	in->private_data = page;
	in->op = &pipe_file_ops;

	out->inode = NULL;
	out->pos = 0;
	out->flags = O_WRONLY;
	out->private_data = page;
	out->op = &pipe_file_ops;

	files[0] = in;
	files[1] = out;

	return 0;
}



ssize_t pipe_read(struct file *filep, void *dest, size_t len) {
	volatile struct cyclic_fifo *fifo;

	// check for data in FIFO, and wait until enought data are present
	fifo = &((struct pipe_page *)filep->private_data)->fifo;
	while(fifo->size < len) {
		// TODO implement waitqueue, and sleep really
		// TODO check for signals!
		sched_schedule();
	}

	cfifo_pop((struct cyclic_fifo*)fifo, dest, len);
	return len;
}



ssize_t pipe_write(struct file *filep, void *source, size_t len) {
	volatile struct cyclic_fifo *fifo;

	fifo = &((struct pipe_page *)filep->private_data)->fifo;
	while(fifo->max_size - fifo->size < len) {
		// TODO implement waitqueue, and sleep really
		// TODO check for signals!
		sched_schedule();
	}

	cfifo_push((struct cyclic_fifo*)fifo, source, len);
	return len;
}



int pipe_release(struct file *filep) {
	struct pipe_page *page;

	page = (struct pipe_page*)(filep->private_data);
	page->count--;
	if(page->count <= 0) {
		arch_pm_release_page(page);
	}
	return 0;
}


