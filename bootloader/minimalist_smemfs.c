#include "minimalist_smemfs.h"


// special memcpy in bootloader section :
void * bootloader_memcpy ( void * destination, const void * source, size_t num );

#define PAGESTATUS_NORMAL 		0x0000
#define PAGESTATUS_UNUSED 		0xFFFF
#define PAGESTATUS_FILETABLE	0x00FF

#define HEADER_SIZE				0x20
#define PREHEADER_SIZE			0x08
#define FILENAME_SIZE           0x18

#define ROOT_ID					0xFFFF

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


#define toupper(c) ((c)>='a' && (c)<='z' ? (c)-('a'-'A') : (c))
#define tolower(c) ((c)>='A' && (c)<='Z' ? (c)+('a'-'A') : (c))


//Internals functions prototypes :
/**
  * Return the fisrt file header byte address (at the end of pages header area)
  */
const unsigned char *bootloader_getFSHeaderAddress();

/**
  * Return the first header byte of the file (or directory) described by the name filename and the parent 
  *  directory ID given by parentDirId (ROOT_ID if the file is in the system root), if existing. Else 
  *  return NULL.
  * WARNING : the file name MUST be "atomic" (isn't a path, it's just the relative file name!)
  */
const unsigned char *bootloader_getAtomicFileHeader(const char *filename, unsigned short parentDirId);

/**
  * Return the first header byte of the file (or directory) with its path. This path is an absolute path 
  *  (starting with \\fls0\ or /fls0/) that can comport multiple sub-directory, for example "\\fls0\foo\bar.txt".
  * If don't exist, return NULL.
  */
const unsigned char *bootloader_getFileHeader(const char *filename);

/**
  * Compare the string str to the file/dir header name pointed by fileHeaderName. Return 0 if it match.
  * If str is too long to represent a file name (max : 8 char for the name, 1 char for the dot and 3 char for
  *  the extension), it will be resized using the Casio file name rules, with the suffix "~1".
  * caseRule indicate how to proceding with the character case. It's value may be CHAR_CASE_UPPER, CHAR_CASE_AUTO
  *  or CHAR_CASE_NOCHANGE.
  */
int bootloader_cmpstr2fhn(const char *str, const unsigned char *fileHeaderName, int caseRule);

/**
  * Return 1 if the fileHeader is a directory header, 0 else.
  */
int bootloader_isDirectory(const unsigned char *fileHeader);

/**
  * Return 1 if the fileHeader is a deleted file header, 0 else.
  */
int bootloader_isDeleted(const unsigned char *fileHeader);

/**
  * Return the file (or directory) internal ID.
  */
unsigned short bootloader_getFileId(const unsigned char *fileHeader);

/**
  * Return the file (or directory) parent directory ID.
  */
unsigned short bootloader_getFileParentDirId(const unsigned char *fileHeader);

/**
  * Return the number of fragment of file. If it's a directory, return -1.
  */
unsigned short bootloader_getFileFragNumber(const unsigned char *fileHeader);

/**
  * Return the total size of the file (sum of the size of all the fragments).
  */
unsigned int bootloader_getFileTotalSize(const unsigned char *fileHeader);

/**
  * Return the offset of a mempage from its ID, or -1 if this ID don't exist yet.
  */
unsigned int bootloader_getPageOffsetByID(unsigned short id);

/**
  * Return the address of the first data byte of the fragment at address fragAdr. If its mempage
  *  ID don't exist, this function return NULL.
  */
const unsigned char *bootloader_getFragDataAddress(const unsigned char *fragAdr);


// Functions definitions :

int fscasio_fopen_ro(const char * filename, struct _fscasio_file *dest) {
	const unsigned char *header;
	header = bootloader_getFileHeader(filename);
	if((header != NULL) && !bootloader_isDirectory(header)) {
		dest->pos = 0;
		dest->flags = 0;

		// Private data :
		dest->current_frag = 0;
		dest->curfrag_pos = 0;
		dest->index = header;
		dest->frag_num = bootloader_getFileFragNumber(header);
		dest->total_size = bootloader_getFileTotalSize(header);				
		return 0;
	}
	return -1;
}

/**
 * Get fragment size and data start from a given fragment number and header address.
 */
void get_frag_info(const unsigned char *header, int fragnum, int *fsize, unsigned char **fstart) {
		int pos = PREHEADER_SIZE + FILENAME_SIZE + HEADER_SIZE*fragnum;

		*fstart = (unsigned char*)bootloader_getFragDataAddress((unsigned char*)((int)(header)+pos));
		*fsize = *((unsigned short*)((int)(header)+pos+18))+1;
}


size_t fscasio_fread(void *ptr, size_t size, size_t nmemb, struct _fscasio_file *file) {
	if((size == 0) || (nmemb == 0) || (file->flags & _FILE_EOF_REATCHED)) {
		return 0;
	}
	else {
		int maxRead;
		int i, j, n, toRead;
		int posInFrag;
		int posInPtr;

		maxRead = file->total_size - file->pos;
		maxRead = maxRead<(size*nmemb) ? maxRead : (size*nmemb);

		n = file->pos + maxRead;
		j = file->pos;
		i = file->current_frag;
		posInFrag = file->curfrag_pos;
		posInPtr = 0;
		while(j<n) {
			int fragSize;
			unsigned char* fragStart;

			get_frag_info(file->index, i, &fragSize, &fragStart);
			fragSize -= posInFrag;
			toRead = (j+fragSize) < n ? fragSize : n-j;

			bootloader_memcpy((void*)((int)ptr + posInPtr), (void*)((int)(fragStart) + posInFrag), toRead);
			j+=toRead;
			posInPtr+=toRead;
			
			if(toRead == fragSize) {
				i++;
				posInFrag=0;
			}
			else posInFrag += toRead;
		}

		file->pos += maxRead;
		file->current_frag = i;
		file->curfrag_pos = posInFrag;
		if(file->pos >= file->total_size) file->flags |= _FILE_EOF_REATCHED;
		return maxRead;
	}
}


int fscasio_fseek(struct _fscasio_file *file, long int offset, int whence) {
	int newpos;
	
	newpos = (whence == SEEK_CUR) ? file->pos + offset : 
			( (whence == SEEK_SET) ? (int)offset : file->total_size + offset);

	if(newpos < 0 || newpos > file->total_size) return -1;
	else {
		int i, j, frag, posInFrag, oldPos;
		j=0;
		frag = -1;
		for(i=0; i<file->frag_num; i++) {
			oldPos = j;
			int fragSize;
			unsigned char *fragStart;
			get_frag_info(file->index, i, &fragSize, &fragStart);
			j += fragSize;
			if(j > newpos) {
				frag = i;
				posInFrag = newpos - oldPos;
				break;
			}
		}
		
		if(frag < 0) return -1;
		else {
			file->current_frag = frag;
			file->curfrag_pos = posInFrag;
			file->pos = newpos;
			file->flags &= ~(_FILE_EOF_REATCHED | _FILE_WRITTEN | _FILE_READED);
			return 0;
		}
	}
}


long int fscasio_ftell(struct _fscasio_file *file) {
	return file->pos;
}







const unsigned char *bootloader_getFSHeaderAddress() {
	unsigned char pageStartC;
	int i;

	pageStartC = CASIO_FS[0];
	for(i=0; CASIO_FS[i * HEADER_SIZE] == pageStartC; i++);
	return (const unsigned char *)((int)(CASIO_FS)+i*HEADER_SIZE);
}



const unsigned char *bootloader_getAtomicFileHeader(const char *filename, unsigned short parentDirId) {
	const unsigned char *start, *current;
	int i;

	start = bootloader_getFSHeaderAddress();
	current = start;

	for(i=0;(current[0] == FILE_HEADER_START) || (current[0] == FILE_HEADER_DELETED); i++) {
		if(!bootloader_isDeleted(current)) {
			if(parentDirId == bootloader_getFileParentDirId(current)) {
				if(bootloader_cmpstr2fhn(filename, (const unsigned char*)((int)(current)+PREHEADER_SIZE), CHAR_CASE_AUTO) == 0) {
					return current;
				}
			}
		}
		if(!bootloader_isDirectory(current)) current = 
			(const unsigned char*)((int)(current)+bootloader_getFileFragNumber(current)*HEADER_SIZE + PREHEADER_SIZE + FILENAME_SIZE);
		else current = (const unsigned char*)((int)(current)+PREHEADER_SIZE + FILENAME_SIZE);
	}
	
	// Not found...
	return NULL;
}



const unsigned char *bootloader_getFileHeader(const char *filename) {
	int i, start, n, stop;
	char s[30], sepchar;
	const unsigned char *ret, *tmp;
	unsigned short currentId;

	start=-1;
	ret = NULL;

	if(filename[0]=='\\' && filename[1]=='\\' && filename[2]=='f' && filename[3]=='l' && filename[4]=='s' && filename[5]=='0' && filename[6]=='\\') {
		start = 7;
		sepchar = '\\';
	}
	else if(filename[0]=='/' && filename[1]=='f' && filename[2]=='l' && filename[3]=='s' && filename[4]=='0' && filename[5]=='/') {
		start = 6;
		sepchar = '/';
	}

	if(start != -1) {
		n=start; 
		currentId = ROOT_ID;
		stop = 0;
		while(!stop) {
			for(i=0; (filename[i+n] != sepchar) && (filename[i+n] != 0) && (i<29); i++) s[i] = filename[i+n];
			s[i]=0;
			if(filename[i+n] == 0) {
				ret = bootloader_getAtomicFileHeader(s, currentId);
				stop = 1;
			}
			else {
				tmp = bootloader_getAtomicFileHeader(s, currentId);
				if(tmp == NULL) {
					ret = NULL;
					stop = 1;
				}
				else currentId = bootloader_getFileId(tmp);
			}
			n += i+1;
		}
	}
	return ret;
}



int bootloader_cmpstr2fhn(const char *str, const unsigned char *fileHeaderName, int caseRule) {
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
			if(caseRule == CHAR_CASE_AUTO){
				if((toupper(s[i]) != fileHeaderName[i*2+1]) && (tolower(s[i]) != fileHeaderName[i*2+1]))
					ret = -i-1;
			}
			else if (caseRule == CHAR_CASE_UPPER) {
				if(toupper(s[i]) != fileHeaderName[i*2+1])
					ret = -i-1;
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


int bootloader_isDirectory(const unsigned char *fileHeader) {
	if(fileHeader[1] == FILE_HEADER_DIR) return 1;
	return 0;
}


int bootloader_isDeleted(const unsigned char *fileHeader) {
	if(fileHeader[0] == FILE_HEADER_DELETED) return 1;
	return 0;
}


unsigned short bootloader_getFileId(const unsigned char *fileHeader) {
	return *((unsigned short*)((int)(fileHeader)+2));
}


unsigned short bootloader_getFileParentDirId(const unsigned char *fileHeader) {
	return *((unsigned short*)((int)(fileHeader)+6));
}


unsigned short bootloader_getFileFragNumber(const unsigned char *fileHeader) {
	if(bootloader_isDirectory(fileHeader)) return 0;
	else {
		return *((unsigned short*)((int)(fileHeader)+PREHEADER_SIZE+FILENAME_SIZE+10));
	}
}


unsigned int bootloader_getFileTotalSize(const unsigned char *fileHeader) {
	unsigned short fnum = bootloader_getFileFragNumber(fileHeader);
	if(fnum == 0) return 0;
	else {
		int i;
		unsigned int ret = 0;
		int pos = PREHEADER_SIZE+FILENAME_SIZE+18;
		for(i=0; i<fnum; i++, pos+=HEADER_SIZE) ret += *((unsigned short*)((int)(fileHeader)+pos))+1;
		return ret;
	}
}

//@TODO : This function never return -1 : add a 2nd condition in the 'for' loop.
unsigned int bootloader_getPageOffsetByID(unsigned short id) {
	unsigned char pageStartC;
	int i, pos;

	pos = 0;
	pageStartC = CASIO_FS[0];
	for(i=0; CASIO_FS[pos] == pageStartC; i++, pos+=HEADER_SIZE) 
		if(*((unsigned short*)((int)(CASIO_FS)+pos+10)) == id) return *((unsigned int*)((int)(CASIO_FS)+pos+4));
	return -1;
}


const unsigned char *bootloader_getFragDataAddress(const unsigned char *fragAdr) {
	unsigned int pageOff = bootloader_getPageOffsetByID(*((unsigned short*)((int)(fragAdr)+14)));
	if(pageOff != -1) {
		pageOff += (int)(CASIO_STORAGE_MEM) + *((unsigned short*)((int)(fragAdr)+16));
		return (const unsigned char*)pageOff;
	}
	else return NULL;
}

