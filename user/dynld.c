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

#include "lib/syscalls.h"

#define write_const(fd, msg) write((fd), (msg), sizeof(msg)-1)

void *dynld_solvename(const char *symbol) {
	int i;

	for(i=0; symbol[i] != '\0'; i++);
	/*write_const(0, "Dynamic solving symbol '");
	write(0, symbol, i);
	write_const(0, "'\n");
	*/

	void *ret;
	if(dynbind(symbol, &ret) == 0)
		return ret;

	return NULL;
}

