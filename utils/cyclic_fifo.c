#include "cyclic_fifo.h"
#include <utils/strutils.h>



void cfifo_push(struct cyclic_fifo *fifo, const char *data, size_t nb) {
	nb = nb > (fifo->max_size - fifo->size) ? (fifo->max_size - fifo->size) : nb;

	if(nb > 0) {
		size_t bottom;

		// compute 'bottom' position, first byte to write
		bottom = (fifo->top+fifo->size) % fifo->max_size;

		// if bottom is after top in the buffer (same position -> size == 0)
		if(bottom >= fifo->top) {
			size_t realnb = fifo->max_size - bottom;
			realnb = realnb > nb ? nb : realnb;

			memcpy(fifo->buffer + bottom, data, realnb);
			data += realnb;
			
			// if a part is not copied
			if(realnb != nb) {
				memcpy(fifo->buffer, data, nb - realnb);
			}
		}

		else {
			memcpy(fifo->buffer + bottom, data, nb);
		}

		fifo->size += nb;
	}
}


void cfifo_pop(struct cyclic_fifo *fifo, char *data, size_t nb) {
	nb = nb > fifo->size ? fifo->size : nb;

	if(nb > 0) {
		size_t endpos;

		// compute 'read end' position, last byte to read
		endpos = (fifo->top + nb-1) % fifo->max_size;

		if(endpos < fifo->top) {
			size_t realnb = fifo->max_size - fifo->top;
			realnb = realnb > nb ? nb : realnb;

			memcpy(data, fifo->buffer + fifo->top, realnb);
			data += realnb;
			
			// if a part is not copied
			if(realnb != nb) {
				memcpy(data, fifo->buffer, nb - realnb);
			}
		}

		else {
			memcpy(data, fifo->buffer + fifo->top, nb);
		}

		fifo->size -= nb;
		fifo->top = (endpos + 1) % fifo->max_size;
	}
}
