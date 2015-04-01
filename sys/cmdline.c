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

#include "cmdline.h"
#include <utils/strutils.h>


// defined by linker script :
extern struct kernel_arg argvector_begin;
extern struct kernel_arg argvector_end;


// FIXME max_size is not really respected...
void cmdline_parse(char *bootargs, size_t max_size) {
	char *curarg;

	curarg = bootargs;
	while(*curarg != '\0' && (curarg - bootargs) < max_size) {
		char *endarg;
		char *valbegin = NULL;
		char *valend = NULL;
		char *nextarg;
		const struct kernel_arg *karg;

		// find the '=' or ' ' character (end of arg name)
		for(endarg = curarg; *endarg != '=' && *endarg != ' ' && *endarg != '\0';
				endarg++);

		// if '=' character, get the value
		if(*endarg == '=') {
			valbegin = endarg+1;
			for(valend = valbegin; *valend != ' ' && *valend != '\0'; valend++);

			nextarg = *valend == '\0' ? valend : valend+1;
		}
		else {
			nextarg = *endarg == '\0' ? endarg : endarg+1;
		}

		// we replace some character by '\0' to separate independant strings
		*endarg = '\0';
		if(valend != NULL)
			*valend = '\0';

		// search in registered boot arguments
		for(karg = &argvector_begin; karg < &argvector_end; karg++) {
			if(!strcmp(karg->argname, curarg)) {
				karg->callback(valbegin);
			}
		}

		curarg = nextarg;
	}
}
