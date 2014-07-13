#ifndef _UTILS_BITFIELD_H
#define _UTILS_BITFIELD_H

/**
 * Large bitfields manipulation helper.
 * A bitfield is an array of 32-bits integer, and the position of the nth bit
 * is considered as bit (n%32) in cell number (n/32).
 */

#include <utils/types.h>


#define BITFIELD_BITS	32

#define BITFIELD_STATIC(name, bits) \
	uint32 name[(bits-1) / BITFIELD_BITS + 1]

extern inline int bitfield_get(const uint32 *bitfield, int bit) {
	return !!(bitfield[bit/BITFIELD_BITS] & (1 << (bit%BITFIELD_BITS)) );
}


extern inline void bitfield_set(uint32 *bitfield, int bit) {
	bitfield[bit/BITFIELD_BITS] |= (1 << (bit%BITFIELD_BITS) );
}


extern inline void bitfield_clear(uint32 *bitfield, int bit) {
	bitfield[bit/BITFIELD_BITS] &= ~(1 << (bit%BITFIELD_BITS) );
}


/**
 * Change the given bit to the given value.
 */
extern inline void bitfield_set_value(uint32 *bitfield, int bit, int val) {
	if(val)
		bitfield_set(bitfield, bit);
	else
		bitfield_clear(bitfield, bit);
}


extern inline void bitfield_all_clear(uint32 *bitfield, int size) {
	int i;
	for(i=0; i < ((size-1) / BITFIELD_BITS + 1); i++)
			bitfield[i] = 0x00000000;
}

#endif //_UTILS_BITFIELD_H
