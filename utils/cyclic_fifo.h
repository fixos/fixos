#ifndef _UTILS_CYCLIC_FIFO_H
#define _UTILS_CYCLIC_FIFO_H

/**
 * Implementation of FIFO-like data structure, for character buffer.
 * To have as less copy as possible, the fixed-size buffer are used in a cyclic
 * way (the top of the FIFO is moving inside buffer each time some bytes are
 * read).
 */

#include <utils/types.h> 

struct cyclic_fifo {
	size_t max_size;
	size_t top;
	size_t size;
	char *buffer;
};


/**
 * Copy nb bytes from data into the FIFO. 
 */
void cfifo_push(struct cyclic_fifo *fifo, const char *data, size_t nb);

/**
 * Copy at most nb bytes from the FIFO to data.
 */
void cfifo_pop(struct cyclic_fifo *fifo, char *data, size_t nb);


#endif //_UTILS_CYCLIC_FIFO_H
