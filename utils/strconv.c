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

#include "strconv.h"


char * strconv_int_dec(int n, char *string)
{
	if(n==0) {
		string[0] = '0';
		string[1] = '\0';
	}
	else {
		int  i;
		int  cpt;
		int start = 0;

		if (n<0) {
			start=1;
			string[0] = '-';
			n *= -1;
		}
		for (i = 1, cpt = 1; n / i >= 10; i *= 10, cpt++);
		for (cpt = start; i; cpt++, i /= 10) string[cpt] = (n / i) % 10 + '0';
		string[cpt] = '\0';
	}

	return string;
}


char * strconv_int_hex(unsigned int val, char *buf)
{
	int cpt = 0;
	int i = 0;
	char c;

	while((val & (0xF << ((7-cpt)*4) )) == 0 && cpt<8)
			cpt++;
	while(cpt<8) {
		c = (val & (0xF << ((7-cpt)*4) )) >> ((7-cpt)*4);
		if(c < 10)
			buf[i] = c + '0';
		else
			buf[i] = c - 10 + 'A';
		i++;
		cpt++;
	}

	// avoid the empty case if val is 0
	if(i==0) {
		buf[0] = '0';
		i=1;
	}
	buf[i] = '\0';
	return buf;
}


char * strconv_ptr(void* val, char *buf)
{
	int cpt = 0;
	char c;

	buf[0] = '0';
	buf[1] = 'x';

	while(cpt<8) {
		c = ((unsigned int)val & (0xF << ((7-cpt)*4) )) >> ((7-cpt)*4);
		if(c < 10)
			buf[cpt+2] = c + '0';
		else
			buf[cpt+2] = c - 10 + 'A';
		cpt++;
	}

	buf[cpt+2] = '\0';
	return buf;
}

