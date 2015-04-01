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

static inline int bitfield_get(const uint32 *bitfield, int bit) {
	return !!(bitfield[bit/BITFIELD_BITS] & (1 << (bit%BITFIELD_BITS)) );
}


static inline void bitfield_set(uint32 *bitfield, int bit) {
	bitfield[bit/BITFIELD_BITS] |= (1 << (bit%BITFIELD_BITS) );
}


static inline void bitfield_clear(uint32 *bitfield, int bit) {
	bitfield[bit/BITFIELD_BITS] &= ~(1 << (bit%BITFIELD_BITS) );
}


/**
 * Change the given bit to the given value.
 */
static inline void bitfield_set_value(uint32 *bitfield, int bit, int val) {
	if(val)
		bitfield_set(bitfield, bit);
	else
		bitfield_clear(bitfield, bit);
}


static inline void bitfield_all_clear(uint32 *bitfield, int size) {
	int i;
	for(i=0; i < ((size-1) / BITFIELD_BITS + 1); i++)
			bitfield[i] = 0x00000000;
}

#endif //_UTILS_BITFIELD_H
