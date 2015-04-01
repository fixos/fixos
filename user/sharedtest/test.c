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

#include <lib/syscalls.h>
#include "test.h"
#include "shared_def.h"

#define write_const(fd, msg) write((fd), (msg), sizeof(msg)-1)

int abc = 2;
int __exported def = 1;

int myfunc2() {
	myfunc(458);
	write_const(0, "myfunc2()\n");
	return 1;
}

int __exported myfunc(int a) {
	abc = a;
	def = -a;
	name("test myfunc");
	return -1;
}

int __exported tada(int a) {
	abc = -a;
	def = a;
	myfunc2();
	return -5;
}

int __exported name(const char * str) {
	int i;

	for(i=0; str[i] != '\0'; i++);
	write_const(0, "shared name(\"");
	write(0, str, i);
	write_const(0, "\")\n");

	return str == (void*)0;
}

