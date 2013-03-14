#include "smemfs_primitives.h"
#include <utils/types.h>


// tmp?
#define tolower(c) (( (c) >= 'A' && (c) <= 'Z') ? (c)-'A'+'a' : (c) )

#define toupper(c) (( (c) >= 'a' && (c) <= 'z') ? (c)-'a'+'A' : (c) )


const unsigned char *getFSHeaderAddress() {
	unsigned char pageStartC;
	int i;

	pageStartC = CASIO_FS[0];
	for(i=0; CASIO_FS[i * HEADER_SIZE] == pageStartC; i++);
	return (const unsigned char *)((int)(CASIO_FS)+i*HEADER_SIZE);
}



const unsigned char *getAtomicFileHeader(const char *filename, unsigned short parentDirId) {
	const unsigned char *start, *current;
	int i;

	start = getFSHeaderAddress();
	current = start;

	for(i=0;(current[0] == FILE_HEADER_START) || (current[0] == FILE_HEADER_DELETED); i++) {
		if(!isDeleted(current)) {
			if(parentDirId == getFileParentDirId(current)) {
				if(cmpstr2fhn(filename, (const unsigned char*)((int)(current)+PREHEADER_SIZE), CHAR_CASE_AUTO) == 0) {
					return current;
				}
			}
		}
		if(!isDirectory(current)) current = 
			(const unsigned char*)((int)(current)+getFileFragNumber(current)*HEADER_SIZE + PREHEADER_SIZE + FILENAME_SIZE);
		else current = (const unsigned char*)((int)(current)+PREHEADER_SIZE + FILENAME_SIZE);
	}
	
	// Not found...
	return NULL;
}



const unsigned char *getFileHeader(unsigned short fileId, int isDir) {
	const unsigned char *start, *current;
	int i;
	unsigned char checkchar = 0x20; // code for a normal file

	if(isDir)
		checkchar = 0x10;

	start = getFSHeaderAddress();
	current = start;

	for(i=0;(current[0] == FILE_HEADER_START) || (current[0] == FILE_HEADER_DELETED); i++) {
		if(!isDeleted(current)) {
			if(fileId == getFileId(current) && current[1] == checkchar) 
				return current;
		}
		if(!isDirectory(current)) current = 
			(const unsigned char*)((int)(current)+getFileFragNumber(current)*HEADER_SIZE + PREHEADER_SIZE + FILENAME_SIZE);
		else current = (const unsigned char*)((int)(current)+PREHEADER_SIZE + FILENAME_SIZE);
	}
	
	// Not found...
	return NULL;
}



int cmpstr2fhn(const char *str, const unsigned char *fileHeaderName, int caseRule) {
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


int getFileName(const unsigned char *header, char *buffer, int copyRule) 
{
	int bpos = 0;
	const unsigned char *fileName = (const unsigned char*)((int)(header)+PREHEADER_SIZE); 
	int i;

	for(i=0; i<12; i++) {
		if(copyRule == CHAR_ASCII_AUTO) {
			if(fileName[i*2+1] != 0xFF) {
				buffer[bpos] = fileName[i*2+1];
				bpos++;
			}
		}
	}

	buffer[bpos] = '\0';

	return 0;
	
}



const unsigned char *getNextChildHeader(const unsigned char *prev, unsigned short parentDirId)
{
	const unsigned char *start, *current;
	int i;
	int skipfirst = 0;

	if(prev == NULL)
		start = getFSHeaderAddress();
	else {
		skipfirst = 1;
		start = prev;
	}
	current = start;

	for(i=0;(current[0] == FILE_HEADER_START) || (current[0] == FILE_HEADER_DELETED); i++) {
		if(!skipfirst) {
			if(!isDeleted(current) && parentDirId == getFileParentDirId(current)) 
					return current;
		}
		else
			skipfirst = 0;

		if(!isDirectory(current)) current = 
			(const unsigned char*)((int)(current)+getFileFragNumber(current)*HEADER_SIZE + PREHEADER_SIZE + FILENAME_SIZE);
		else current = (const unsigned char*)((int)(current)+PREHEADER_SIZE + FILENAME_SIZE);
	}
	
	// Not found...
	return NULL;

}


int isDirectory(const unsigned char *fileHeader) {
	if(fileHeader[1] == FILE_HEADER_DIR) return 1;
	return 0;
}


int isDeleted(const unsigned char *fileHeader) {
	if(fileHeader[0] == FILE_HEADER_DELETED) return 1;
	return 0;
}


unsigned short getFileId(const unsigned char *fileHeader) {
	return *((unsigned short*)((int)(fileHeader)+2));
}


unsigned short getFileParentDirId(const unsigned char *fileHeader) {
	return *((unsigned short*)((int)(fileHeader)+6));
}


unsigned short getFileFragNumber(const unsigned char *fileHeader) {
	if(isDirectory(fileHeader)) return 0;
	else {
		return *((unsigned short*)((int)(fileHeader)+PREHEADER_SIZE+FILENAME_SIZE+10));
	}
}


unsigned int getFileTotalSize(const unsigned char *fileHeader) {
	unsigned short fnum = getFileFragNumber(fileHeader);
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
unsigned int getPageOffsetByID(unsigned short id) {
	unsigned char pageStartC;
	int i, pos;

	pos = 0;
	pageStartC = CASIO_FS[0];
	for(i=0; CASIO_FS[pos] == pageStartC; i++, pos+=HEADER_SIZE) 
		if(*((unsigned short*)((int)(CASIO_FS)+pos+10)) == id) return *((unsigned int*)((int)(CASIO_FS)+pos+4));
	return -1;
}


const unsigned char *getFragDataAddress(const unsigned char *fragAdr) {
	unsigned int pageOff = getPageOffsetByID(*((unsigned short*)((int)(fragAdr)+14)));
	if(pageOff != -1) {
		pageOff += (int)(CASIO_STORAGE_MEM) + *((unsigned short*)((int)(fragAdr)+16));
		return (const unsigned char*)pageOff;
	}
	else return NULL;
}
