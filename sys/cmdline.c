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
