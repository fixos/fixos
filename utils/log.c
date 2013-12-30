#include "log.h"
#include "types.h"
#include "strconv.h"
#include <fs/vfs_file.h>

// normally provide by GCC, with only builtins calls...
#include <stdarg.h>

#define BUFFER_SIZE 256

static print_callback_t current_kernel_print = NULL;

// used by print_to_file()
static struct file *current_log_file = NULL;

static void print_to_file(const char *str);


void printk(const char *str, ...)
{
	if(current_kernel_print != NULL) {
		va_list vargs;
		char buf[BUFFER_SIZE];
		int i;
		int bufpos;

		va_start(vargs, str);

		i=0;
		bufpos = 0;
		while(str[i] != '\0') {
			char c = str[i];
			if(c != '%')
			{
				buf[bufpos] = c;
				bufpos++;
			}

			else if(str[i+1] != '\0')
			{
				buf[bufpos] = '\0';
				current_kernel_print(buf);
				bufpos = 0;

				i++;
				c = str[i];
				switch(c) {
				case 's':
				{
					const char * ps = va_arg(vargs, const char*);
					current_kernel_print(ps);
					break;
				}
				case 'd':
				{
					int pi = va_arg(vargs, int);
					strconv_int_dec(pi, buf);
					current_kernel_print(buf);
					break;
				}
				case 'x':
				{
					unsigned int pu = va_arg(vargs, unsigned int);
					strconv_int_hex(pu, buf);
					current_kernel_print(buf);
					break;
				}
				case 'p':
				{
					void * pp = va_arg(vargs, void*);
					strconv_ptr(pp, buf);
					current_kernel_print(buf);
					break;
				}
				case '%':
					buf[bufpos] = '%';
					bufpos++;
				}
			}
			// flush the buffer if needed
			if(bufpos >= BUFFER_SIZE-1 || str[i+1] == '\0') {
				buf[bufpos] = '\0';
				current_kernel_print(buf);
				bufpos = 0;
			}

			i++;
		}

		va_end(vargs);
	}
}


void set_kernel_print(print_callback_t func)
{
	current_kernel_print = func;
}


void set_kernel_print_file(struct file *logfile) {
	current_log_file = logfile;
	if(current_log_file != NULL) {
		current_kernel_print = &print_to_file;
	}
	else {
		current_kernel_print = NULL;
	}
}

void print_to_file(const char *str) {
	size_t len = 0;
	
	// no strlen() for now, basic implementation
	while(str[len]!='\0')
		len++;

	vfs_write(current_log_file, str, len);
}
