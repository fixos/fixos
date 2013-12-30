#ifndef _UTILS_LOG_H
#define _UTILS_LOG_H


typedef void(*print_callback_t)(const char*);

struct file;

/**
 * printf-like function to log misc kernel messages
 * Currently, only some modifier are allowed : %s, %x, %p, %d
 * The print callback will be called at least once, but it may be called many time.
 */
void printk(const char *str, ...) __attribute__ ((format (printf, 1, 2)));

/**
 * Set the printk function used for print a string chunk.
 */
void set_kernel_print(print_callback_t func);


/**
 * Set printk callback function to vfs_write(logfile, msg, strlen(msg)).
 * This function override the callback defined by set_kernel_print().
 */
void set_kernel_print_file(struct file *logfile);

#endif //_UTILS_LOG_H
