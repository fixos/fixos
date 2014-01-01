#include "files.h"
#include <utils/log.h>


int sys_open(const char *name, int mode) {
	printk("Received sys_open :\n   ('%s', %d)\n", name, mode);
	return -1;
}

ssize_t sys_read(int fd, char *dest, int nb) {
	return -1;
}

ssize_t sys_write(int fd, const char *source, int nb){
	return -1;
}

