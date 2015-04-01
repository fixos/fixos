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

#include "smemfs_primitives_ng.h"


// tmp?
#define tolower(c) (( (c) >= 'A' && (c) <= 'Z') ? (c)-'A'+'a' : (c) )

#define toupper(c) (( (c) >= 'a' && (c) <= 'z') ? (c)-'a'+'A' : (c) )



// begin address of Casio's EEPROM memory area (0xA0000000 in all known models)
// used as base address for all block offset
static void* _smemfs_base_address = NULL;

// begin address of Casio's SMEM filesystem block definition
static struct smemfs_block_entry* _smemfs_start = NULL;

// begin address of Casio's SMEM filesystem file headers
static struct smemfs_file_preheader* _smemfs_file_start = NULL;

// magic character for block header
static uint8 _smemfs_block_magic = 0;



int smemfs_prim_init(void* fs_start, void *base_address) {
	_smemfs_start = fs_start;
	_smemfs_base_address = base_address;

	_smemfs_block_magic = ((uint8*)fs_start)[0];
	_smemfs_file_start = smemfs_prim_get_file_header(fs_start);

	// TODO chek if FS is as expected
	return 0;
}



void* smemfs_prim_get_file_header(void *fs_start) {
	struct smemfs_block_entry *cur_entry = fs_start;
	
	// check every block header, before to find an other character as magic char
	while(cur_entry->bl_magic == _smemfs_block_magic)
		cur_entry++;

	return cur_entry;
}


struct smemfs_file_preheader* smemfs_prim_get_atomic_file(const char *filename, uint16 parent_dir) {
	struct smemfs_file_preheader* current;

	current = _smemfs_file_start;
	while(current->magic == SMEMFS_FILE_EXISTS || current->magic == SMEMFS_FILE_DELETED) {
		// don't test filename if it's a deleted entry
		if(current->magic == SMEMFS_FILE_EXISTS) {
			if(parent_dir == current->parent_id) {
				if(cmpstr2fhn(filename, current->name, CHAR_CASE_AUTO) == 0) {
					return current;
				}
			}
			
		}

		// in all case, skip preheader and fragment header(s)
		current++;
		struct smemfs_frag_header *fragment = (void*)current;
		while(fragment->type == SMEMFS_FILE_FILE_FRAG)
			fragment++;
		current = (void*)fragment;
	}

	// Not found...
	return NULL;
}



struct smemfs_file_preheader* smemfs_prim_get_file_byid(uint16 entry_id, int isdir) {
	struct smemfs_file_preheader* current;
	uint8 type;

	current = _smemfs_file_start;
	type = isdir ? SMEMFS_FILE_DIR : SMEMFS_FILE_FILE_H;
	while(current->magic == SMEMFS_FILE_EXISTS || current->magic == SMEMFS_FILE_DELETED) {
		// check if entry is not deleted and has the good ID
		if(current->magic == SMEMFS_FILE_EXISTS && current->type == type && current->entry_id == entry_id) {
			return current;
		}

		// in all case, skip preheader and fragment header(s)
		current++;
		struct smemfs_frag_header *fragment = (void*)current;
		while(fragment->type == SMEMFS_FILE_FILE_FRAG)
			fragment++;
		current = (void*)fragment;
	}

	// Not found...
	return NULL;
}


int cmpstr2fhn(const char *str, const uint16 *file_name, int caseRule) {
	const unsigned char *fileHeaderName = (void*)file_name;
	int ret, i, j, k;
	char s[13];

	for(i=0; (str[i] != 0) && (str[i] != '.') && (i<8); i++) s[i]=str[i];
	if((i==8) && (str[8] != '.') && (str[8] != 0)) {
		s[6]='~';
		s[7]='1';
	}
	j=i;
	if((str[i] != '.') && (str[i] != 0)) for(j=i; (str[j] != '.') && (str[j] != 0); j++);
	if(str[j] != 0) for(k=0; (str[j+k] != 0) && (k<4); k++, i++) s[i]=str[j+k];
	s[i] = 0;
	
	j=1;
	ret = 0;
	for (i=0; i<12; i++) {
		if(s[i]==0) j=0;
		if(j) {
			if(caseRule == CHAR_CASE_AUTO) {
				if((toupper(s[i]) != fileHeaderName[i*2+1]) && (tolower(s[i]) != fileHeaderName[i*2+1])) ret = -i-1;
			}
			else if (caseRule == CHAR_CASE_UPPER) {
				if(toupper(s[i]) != fileHeaderName[i*2+1]) ret = -i-1;
			}
			// Else, don't change the character case (eq CHAR_CASE_NOCHANGE).		
			else if(s[i] != fileHeaderName[i*2+1]) ret = -i-1;
		}
		else if(fileHeaderName[i*2+1] != 0xFF) ret = -i-1;

//
		if(ret<0) return ret;
//
	}

	return ret;
}




char* smemfs_prim_get_file_name(struct smemfs_file_preheader *header, char *buffer, int copyRule) 
{
	int bpos = 0;
	int i;

	for(i=0; i<12; i++) {
		if(copyRule == CHAR_ASCII_AUTO) {
			if((header->name[i] & 0x00FF)  != 0xFF) {
				buffer[bpos] = header->name[i] & 0x00FF;
				bpos++;
			}
		}
	}

	buffer[bpos] = '\0';
	return buffer;
}



struct smemfs_file_preheader* smemfs_prim_get_next_child(struct smemfs_file_preheader *prev, uint16 parent_dir) {
	struct smemfs_file_preheader* current;
	int skip;

	// if prev is NULL, begin from first entry without skipping the current one
	current = prev == NULL ? _smemfs_file_start : prev;
	skip = prev != NULL;

	while(current->magic == SMEMFS_FILE_EXISTS || current->magic == SMEMFS_FILE_DELETED) {
		if(!skip) {
			// check if entry is not deleted and has the good parent ID
			if(current->magic == SMEMFS_FILE_EXISTS && current->parent_id == parent_dir) {
				return current;
			}
		}
		skip = 0;

		// in all case, skip preheader and fragment header(s)
		current++;
		struct smemfs_frag_header *fragment = (void*)current;
		while(fragment->type == SMEMFS_FILE_FILE_FRAG)
			fragment++;
		current = (void*)fragment;
	}

	// Not found...
	return NULL;
}


int smemfs_prim_get_frag_num(struct smemfs_file_preheader *header) {
	if(header->type == SMEMFS_FILE_DIR)
		return 0;
	else {
		struct smemfs_frag_header *frag = (void*)(header+1);
		return frag->frag_total_num;
	}
}


size_t smemfs_prim_get_file_size(struct smemfs_file_preheader *header) {
	int fragnb;
	int i;
	struct smemfs_frag_header *frag;
	size_t ret = 0;

	fragnb = smemfs_prim_get_frag_num(header);
	frag = (void*)(header+1);
	
	for(i=0; i<fragnb; i++, frag++)
		ret += frag->data_size + 1;

	return ret;
}


struct smemfs_block_entry* smemfs_prim_get_block_header(uint16 block_id) {
	struct smemfs_block_entry *cur_entry = _smemfs_start;
	
	// check every block header, before to find an other character as magic char
	while(cur_entry->bl_magic == _smemfs_block_magic) {
		if(cur_entry->bl_id == block_id)
			return cur_entry;
		cur_entry++;
	}

	return NULL;
}


void* smemfs_prim_get_frag_data(struct smemfs_frag_header *frag) {
	struct smemfs_block_entry *block;

	block = smemfs_prim_get_block_header(frag->data_block_id);
	if(block != NULL) {
		return _smemfs_base_address + block->rel_offset + frag->data_offset;
	}

	return NULL;
}
