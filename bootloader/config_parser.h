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

#ifndef _BOOTLOADER_CONFIG_PARSER_H
#define _BOOTLOADER_CONFIG_PARSER_H

/**
 * Bootloader Configuration File functions and definitions.
 * These functions provide a medium-level layer to parse and process the
 * bootloader configuration file format.
 */

#include "smem_file.h"

/**
 * Values used to define the 'Tag' type (kind of line in config file)
 */
// line is blank or a comment
#define CONFIG_TAG_EMPTY	0
// line is a scope definition (like "[Something]")
#define CONFIG_TAG_SCOPE	1
// line is an attribute definition (like "something=value")
#define CONFIG_TAG_ASSIGN	2
// unexpected syntax
#define CONFIG_TAG_UNKNOWN	-1
// end of file
#define CONFIG_TAG_EOF		-2


/**
 * Read the current line, returns the tag type (CONFIG_TAG_xxx), and
 * copy tag value for some kind of tags :
 * - if line is a CONFIG_TAG_SCOPE, buffer is filled with scope name
 * - if line is a CONFIG_TAG_ASSIGN, buffer is filled with attribute name,
 *   and current position in file is just after the '=' symbol
 *
 * For all other tags than CONFIG_TAG_ASSIGN, the position on the file
 * after calling this function is just after the next line feed.
 */
int config_read_tag(struct smem_file *file, char *buffer, int bufsize);


/**
 * Read the string value at the current position.
 * A string value is delimited by double quote character, like "this is a string".
 *
 * Returns 0 if string value is read correctly and buffer is filled with the
 * string content, a negative value else :
 * -1 is returned if any syntax error is encountered (missing quote)
 * -2 is returned if string content is bigger than bufsize (in this case,
 *    buffer is partialy filled but not zero-terminated). 
 */
int config_read_assign_string(struct smem_file *file, char *buffer, int bufsize);



int config_read_assign_bool(struct smem_file *file, int *value);


int config_read_assign_int(struct smem_file *file, int *value);

#endif //_BOOTLOADER_CONFIG_PARSER_H
