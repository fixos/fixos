#include "log.h"
#include "types.h"
#include "strconv.h"
#include <fs/vfs_file.h>
#include <sys/cmdline.h>

// normally provide by GCC, with only builtins calls...
// FIXME replace by in-kernel definition with __builtin_va_*
#include <stdarg.h>

#define BUFFER_SIZE 256

static print_callback_t current_kernel_print = NULL;

// used by print_to_file()
static struct file *current_log_file = NULL;

// dynamic level, default is to display every non debug message
static int printk_dynamic_level = LOG_INFO;

static void print_to_file(const char *str);

void printk_internal(int level, const char *str, ...)
{
	if(level >= printk_dynamic_level && current_kernel_print != NULL) {
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


void printk_set_level(int level) {
	printk_dynamic_level = level;
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


// for parsing command-line parameter "loglevel="
static int parse_loglevel(const char *val) {
	if((*val >= '0' && *val <= '7') && val[1] == '\0') {
		printk_dynamic_level = *val - '0';
		return 0;
	}

	printk(LOG_WARNING, "malformed 'loglevel' parameter\n");
	return -1;
}

KERNEL_BOOT_ARG(loglevel, parse_loglevel);
