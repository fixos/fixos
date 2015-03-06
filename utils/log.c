#include "log.h"
#include "types.h"
#include "strconv.h"
#include <sys/cmdline.h>
#include <sys/tty.h>

// normally provide by GCC, with only builtins calls...
// FIXME replace by in-kernel definition with __builtin_va_*
#include <stdarg.h>

#define BUFFER_SIZE 256

static print_callback_t current_kernel_print = NULL;

// console TTY used (NULL if using early printk)
static struct tty *_current_console_tty = NULL;

// dynamic level, default is to display every non debug message
static int printk_dynamic_level = LOG_INFO;

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


static void printk_write_tty(const char *str) {
	size_t len = 0;
	
	// no strlen() for now, basic implementation
	while(str[len]!='\0')
		len++;

	tty_write(_current_console_tty, str, len);
}

void printk_set_console_tty(struct tty *tty) {
	_current_console_tty = tty;
	if(_current_console_tty != NULL) {
		current_kernel_print = &printk_write_tty;
	}
	else {
		current_kernel_print = NULL;
	}
}


void printk_force_flush() {
	if(_current_console_tty != NULL)
		tty_force_flush(_current_console_tty);
}


// global to reduce stack usage (better if used after... like... a stack overflow?)
// 48 is enought for all line cases (2*16 bytes + '\0')
static char _printmem_buf[48];

// TODO use this in strutils
const static char *_print_hexcar = "0123456789ABCDEF";

void print_memory(int level, void *addr, size_t len) {
	if(level >= printk_dynamic_level) {
		int pos = 0;

		printk(level, "Memory content [%p~%p] :\n", addr, addr+len);
		while(pos < len) {
			int bufpos;
			size_t firstoffset = pos;

			// prepare the next 16 bytes...
			for(bufpos=0; pos<len && (pos - firstoffset) < 16; pos++, bufpos+=2) {
				unsigned char c;

				// add an extra space each 4 bytes to help readibility
				if((pos - firstoffset) != 0 && (pos - firstoffset) % 4 == 0)
					_printmem_buf[bufpos++] = ' ';

				c = ((unsigned char*)addr)[pos];
				_printmem_buf[bufpos] = _print_hexcar[(int)(c >> 4)];
				_printmem_buf[bufpos+1] = _print_hexcar[(int)(c && 0x0F)];
			}
			_printmem_buf[bufpos] = '\0';

			printk(level, "%p: %s\n", addr + firstoffset, _printmem_buf);
		}
	}
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
