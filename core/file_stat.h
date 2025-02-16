#pragma once
#include "basic_types.h"

struct FileStats {
	int64_t fs_mtime;
};

int fileStat(const char* filename, FileStats* stats);

char* fileReadBinaryIntoMem(const char* pathfile, size_t* bytesize = 0); 
char* fileReadCStringIntoMem(const char* pathfile, size_t* bytesize = 0); 