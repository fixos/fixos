#ifndef _FS_SMEMFS_PRIMITIVES_H
#define _FS_SMEMFS_PRIMITIVES_H

/**
 * Low-level primitives for Casio SMEM FS manipulation.
 * WARNING : these functions may cause damages to EEPROM integrity,
 * including Casio's OS failures, if they're bad used - and if writing
 * is implemented ;).
 */


// very important definitions, which is probably model-dependant :/
#define CASIO_FS ((const unsigned char*)(0xA0270000))

#define CASIO_STORAGE_MEM ((const unsigned char*)(0xA0000000)) 


#define PAGESTATUS_NORMAL 		0x0000
#define PAGESTATUS_UNUSED 		0xFFFF
#define PAGESTATUS_FILETABLE	0x00FF

#define HEADER_SIZE				0x20
#define PREHEADER_SIZE			0x08
#define FILENAME_SIZE           0x18

#define ROOT_ID					0xFFFF
#define INVALID_NODE			0xFFFFFFFF

//First file header byte value :
#define FILE_HEADER_DELETED		0x01
#define FILE_HEADER_START		0x51
//Second file header byte value :
#define FILE_HEADER_FILE		0x20
#define FILE_HEADER_POSTFILE	0x30
#define FILE_HEADER_DIR			0x10



#define CHAR_CASE_NOCHANGE		0
#define CHAR_CASE_UPPER			1
#define CHAR_CASE_AUTO			2

#define CHAR_ASCII_AUTO			1



/**
  * Return the fisrt file header byte address (at the end of pages header area)
  */
const unsigned char *getFSHeaderAddress();

/**
  * Return the first header byte of the file (or directory) described by the name filename and the parent 
  *  directory ID given by parentDirId (ROOT_ID if the file is in the system root), if existing. Else 
  *  return NULL.
  * WARNING : the file name MUST be "atomic" (isn't a path, it's just the relative file name!)
  */
const unsigned char *getAtomicFileHeader(const char *filename, unsigned short parentDirId);

/**
 * Look for the the file with ID 'fileId' and return his header if exists.
 * Else return NULL.
 * Note : the header may be a removed file header, use isDeleted to check
 */
const unsigned char *getFileHeader(unsigned short fileId);

/**
  * Compare the string str to the file/dir header name pointed by fileHeaderName. Return 0 if it match.
  * If str is too long to represent a file name (max : 8 char for the name, 1 char for the dot and 3 char for
  *  the extension), it will be resized using the Casio file name rules, with the suffix "~1".
  * caseRule indicate how to proceding with the character case. It's value may be CHAR_CASE_UPPER, CHAR_CASE_AUTO
  *  or CHAR_CASE_NOCHANGE.
  */
int cmpstr2fhn(const char *str, const unsigned char *fileHeaderName, int caseRule);


/**
 * Obtain the name of a file from its internal ID, and copy it in the
 * specified buffer using the specified copy rule.
 * With CHAR_ASCII_AUTO, the buffer must be 13 bytes length or more.
 */
int getFileName(const unsigned char *header, char *buffer, int copyRule); 

/**
 * Look for a child of prentDirId into the headers.
 * prev is the first address to look for (NULL if it's the first call).
 * NULL is returned if no more entry are found.
 */
const unsigned char *getNextChildHeader(const unsigned char *prev, unsigned short parentDirId);

/**
  * Return 1 if the fileHeader is a directory header, 0 else.
  */
int isDirectory(const unsigned char *fileHeader);

/**
  * Return 1 if the fileHeader is a deleted file header, 0 else.
  */
int isDeleted(const unsigned char *fileHeader);

/**
  * Return the file (or directory) internal ID.
  */
unsigned short getFileId(const unsigned char *fileHeader);

/**
  * Return the file (or directory) parent directory ID.
  */
unsigned short getFileParentDirId(const unsigned char *fileHeader);

/**
  * Return the number of fragment of file. If it's a directory, return -1.
  */
unsigned short getFileFragNumber(const unsigned char *fileHeader);

/**
  * Return the total size of the file (sum of the size of all the fragments).
  */
unsigned int getFileTotalSize(const unsigned char *fileHeader);

/**
  * Return the offset of a mempage from its ID, or -1 if this ID don't exist yet.
  */
unsigned int getPageOffsetByID(unsigned short id);

/**
  * Return the address of the first data byte of the fragment at address fragAdr. If its mempage
  *  ID don't exist, this function return NULL.
  */
const unsigned char *getFragDataAddress(const unsigned char *fragAdr);



#endif //_FS_SMEMFS_PRIMITIVES_H
