#ifndef _FS_SMEMFS_PRIMITIVES_NG_H
#define _FS_SMEMFS_PRIMITIVES_NG_H

#include <utils/types.h>


// entry of "super block" table (describe each memory area used for SMEM file system
struct smemfs_block_entry {
	uint8 bl_magic;  // seems to be often 0x42 or 0x02, but not always...	
	uint8 unknown1;  // 0x00 ?

	uint16 bl_num; // block number (not the block ID!)
	uint32 rel_offset; // block relative offset in memory area
	
	uint8 bl_used; // 0xFF if not used, 0x00 else
	uint8 bl_filetable; // 0xFF if block is used by File Header table, 0x00 if it's for data
	uint16 bl_id; // block ID, used to refer to it from file header

	// not realy used, seems to contain only "FF FF FF FF 00 00 00 01 FF FF FF FF FF FF FF FF FF FF FF FF"
	uint8 magic_end[20];
};


#define SMEMFS_BL_USED		0x00
#define SMEMFS_BL_UNUSED	0xFF

#define SMEMFS_BL_FILETABLE	0xFF
#define SMEMFS_BL_DATA		0x00



// Pre-Header for file and directories entries (followed by one frag_header by fragment
// for files, or nothing else for directories).
struct smemfs_file_preheader {
	uint8 magic; // 0x51 if entry exists, 0x01 if entry refers to a deleted file
	uint8 type; // 0x10 for a directory, 0x20 for file preheader

	uint16 entry_id; // entry ID (unique only between the same type -> file/directory)

	uint16 unknown1; // 0xFFFF on root, 0x0110 for file in other directories
	uint16 parent_id; // directory ID of entry, 0xFFFF is for root

	uint16 name[8+1+3]; // FONTCHAR style name (16 bits character), 8 chars for left part name
						// and 3 for extension (after the '.')
};

#define SMEMFS_FILE_EXISTS	0x51
#define SMEMFS_FILE_DELETED	0x01

#define SMEMFS_FILE_DIR		0x10
#define SMEMFS_FILE_FILE_H	0x20
#define SMEMFS_FILE_FILE_FRAG 0x30

#define SMEMFS_FILE_ROOT_ID	0xFFFF

#define SMEMFS_FILE_UNK1_DIR 0x0110
#define SMEMFS_FILE_UNK1_ROOT 0xFFFF




// Fragment header, one per fragment in file
struct smemfs_frag_header {
	uint8 magic; // same as for smemfs_file_preheader
	uint8 type; // always 0x30

	uint16 frag_id; // fragment ID
	uint16 unknown1; // 0x0120 in these exemples
	
	uint16 file_id; // file entry ID (same in all fragments of a given file)
	uint16 unknown2; // 0x0002 in most case, 0x0001 some times :x
	
	uint16 frag_total_num; // total number of fragments for this file
	uint16 frag_num; // number of this fragment (1 <= frag_num <= frag_total_num)

	uint16 data_block_id; // ID of blocks containing data for this fragment
	uint16 data_offset; // offset of fragment data in the given block

	// WARNING : the size is (real_size-1), so *do not forget* to add 1 to
	// have the real size of data!
	uint16 data_size; // size of this fragment data

	uint8 _fill[12] ; // contains only 0xFF
};

#define SMEMFS_FRAG_UNK1_MAGIC		0x0120
#define SMEMFS_FRAG_UNK2_MAGIC1		0x0002
#define SMEMFS_FRAG_UNK2_MAGIC2		0x0001


#define CHAR_CASE_NOCHANGE		0
#define CHAR_CASE_UPPER			1
#define CHAR_CASE_AUTO			2

#define CHAR_ASCII_AUTO			1

/**
 * Initialize static FS related data, such as begin of headers.
 * In the same time, this function may check if the given address seems to be
 * a valid SMEM FS (according to known properties) to avoid manipulating a not
 * known version of the Casio's FS.
 * fs_start is the first byte of FS header, and base_address is the address
 * of EEPROM beginning.
 * Returns 0 if initialisation is done, negative value if FS doesn't seems to
 * be valid.
 *
 * WARNING: this function *must* be called before any smemfs_prim_* function!
 *
 * TODO check FS validity
 */
int smemfs_prim_init(void* fs_start, void* base_address);



/**
  * Return the fisrt file header byte address (at the end of pages header area)
  */
void* smemfs_prim_get_file_header(void *fs_start);


/**
  * Return the first header byte of the file (or directory) described by the name filename and the parent 
  *  directory ID given by parentDirId (ROOT_ID if the file is in the system root), if exists. Else 
  *  return NULL.
  * WARNING : the file name MUST be "atomic" (isn't a path, it's just the relative file name!)
  */
struct smemfs_file_preheader* smemfs_prim_get_atomic_file(const char *filename, uint16 parent_dir);


struct smemfs_file_preheader* smemfs_prim_get_file_byid(uint16 entry_id, int isdir);


/**
  * Compare the string str to the file/dir header name pointed by file_name.
  * Returns 0 when matching.
  * If str is too long to represent a file name (max : 8 char for the name,
  * 1 char for the dot and 3 char for the extension), it will be resized using
  * the Casio file name rules, with the suffix "~1".
  * caseRule indicate how to proceding with the character case.
  * It's value may be CHAR_CASE_UPPER, CHAR_CASE_AUTO or CHAR_CASE_NOCHANGE.
  */
int cmpstr2fhn(const char *str, const uint16 *file_name, int caseRule);


/**
 * Obtain the name of a file from its header, and copy it in the specified
 * buffer using the specified copy rule.
 * With CHAR_ASCII_AUTO, the buffer must be 13 bytes length or more.
 * Returns the buffer itself.
 */
char* smemfs_prim_get_file_name(struct smemfs_file_preheader *header, char *buffer, int copyRule);



/**
 * Look for a child of prent_dir into the headers.
 * prev is the first address to look from (NULL is used to look from the first entry).
 * NULL is returned if no more entry are found.
 */
struct smemfs_file_preheader* smemfs_prim_get_next_child(struct smemfs_file_preheader *prev, uint16 parent_dir);


/**
 * Returns the number of fragment of the given file from its header.
 * 0 is returned if it's a directory.
 */
int smemfs_prim_get_frag_num(struct smemfs_file_preheader *header);

/**
 * Returns total size of the file (sum of all fragment size).
 * If header is a directory header, returns 0.
 */
size_t smemfs_prim_get_file_size(struct smemfs_file_preheader *header);

/**
 * Return block header for given block ID, or NULL if not found.
 */
struct smemfs_block_entry* smemfs_prim_get_block_header(uint16 block_id);

/**
 * Returns address of first fragment byte (offset in block + block offset + base address)
 */
void* smemfs_prim_get_frag_data(struct smemfs_frag_header *frag);

#endif //_FS_SMEMFS_PRIMITIVES_NG_H
