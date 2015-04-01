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

#include "config_parser.h"



int config_read_tag(struct smem_file *file, char *buffer, int bufsize) {
	int c;
	int ret;

	// test first character
	c = smem_readchar(file);
	if(c < 0) {
		ret = CONFIG_TAG_EOF;
	}
	
	else if (c == '#' || c == '\n') {
		// comment, skip everything until end of line
		while(c != '\n' && c >= 0)
			c = smem_readchar(file);
		ret = CONFIG_TAG_EMPTY;
	}
	
	else if (c == '[') {
		int i = 0;

		// copy content under bracket
		while(c != '\n' && c >= 0 && c != ']' && i<bufsize) {
			c = smem_readchar(file);
			buffer[i] = (char)c;
			i++;
		}

		if(i<=bufsize && c == ']') {
			buffer[i-1] = '\0';
			ret = CONFIG_TAG_SCOPE;

			// skip the rest of the line
			while(c != '\n' && c >= 0)
				c = smem_readchar(file);
		}
		else {
			ret = CONFIG_TAG_UNKNOWN;
		}
	}

	else {
		int i = 1;

		// if we found an '=' character, this is an assignement
		buffer[0] = (char)c;

		while(c != '\n' && c >= 0 && c != '=' && i<bufsize) {
			c = smem_readchar(file);
			buffer[i] = (char)c;
			i++;
		}

		if(i<=bufsize && c == '=') {
			buffer[i-1] = '\0';
			ret = CONFIG_TAG_ASSIGN;
		}
		else {
			ret = CONFIG_TAG_UNKNOWN;
		}
	}


	return ret;
}



int config_read_assign_string(struct smem_file *file, char *buffer, int bufsize) {
	int c;

	// string must begin and end with '"' character
	c = smem_readchar(file);
	if(c == '"') {
		int i = 0;

		do {
			c = smem_readchar(file);
			buffer[i] = (char)c;
			i++;
		} while(c != '\n' && c >= 0 && c != '"' && i<bufsize);

		if(i<=bufsize && c == '"') {
			buffer[i-1] = '\0';
			
			// skip until line feed
			while(c != '\n' && c >= 0)
				c = smem_readchar(file);
		}
		else {
			// TODO return -2 if it's a string value, but too big for bufsize
			return -1;
		}
		
	}
	else {
		return -1;
	}

	return 0;
}



int config_read_assign_bool(struct smem_file *file, int *value) {
	int c;
	int tempval = -1;

	// accepted values are 'yes', 'no', 'y' and 'n'
	c = smem_readchar(file);
	if(c == 'y') {
		c = smem_readchar(file);
		if(c == '\n' || (c == 'e' && smem_readchar(file) == 's' && smem_readchar(file) == '\n') ) {
			tempval = 1;
		}
	}


	else if(c == 'n') {
		c = smem_readchar(file);
		if(c == '\n' || (c == 'o' && smem_readchar(file) == '\n') ) {
			tempval = 0;
		}
	}

	if(tempval != -1) {
		*value = tempval;
		return 0;
	}
	
	return -1;
}



int config_read_assign_int(struct smem_file *file, int *value) {
	int tmpval = 0;
	int c;

	do {
		c = smem_readchar(file);
		if(c >= '0' && c<= '9') {
			tmpval = tmpval*10 + c-'0';	
		}
	} while(c != '\n' && c >= 0 && c >= '0' && c <= '9');

	if(c == '\n' || c == -1) {
		// skip until line feed
		while(c != '\n' && c >= 0)
			c = smem_readchar(file);
		*value = tmpval;
		return 0;
	}

	return -1;
}
