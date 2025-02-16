#include "dir.h"

#include "core/dirent.h"
#include "math/maths.h" // for min

#pragma warning(disable:4706) // Assignment within conditional

static void dirScanInternal(const char* directory, const char* suffix_or_extension, DirScanFullCallback full_callback, DirScanPathfileCallback pathfile_callback, void* userdata) {
	char strtmp[2048];
	char nametmp[1024];
	char exttmp[32];
	size_t dir_namlen = strlen(directory);
	memcpy(strtmp, directory, min(dir_namlen + 1, sizeof(strtmp)));
	strtmp[dir_namlen] = 0;
	DIR *dirp = opendir(strtmp);
	int extlen = suffix_or_extension ? (int)strlen(suffix_or_extension) : 0;
	if (dirp) {
		struct dirent *entry;
		while ((entry = readdir(dirp))) {
			if (entry->d_namlen > size_t(extlen) && (extlen == 0 || strstr(entry->d_name, suffix_or_extension))) {
				memcpy(strtmp + dir_namlen, entry->d_name, min(entry->d_namlen + 1, sizeof(strtmp))); // save one slot for 0
				strtmp[dir_namlen + entry->d_namlen] = 0;
				if (full_callback) {
					memcpy(nametmp, entry->d_name, entry->d_namlen - extlen);
					nametmp[entry->d_namlen - extlen] = 0;
					memcpy(exttmp, entry->d_name + entry->d_namlen - extlen, extlen);
					exttmp[extlen] = 0;
					full_callback(userdata, strtmp, nametmp, exttmp);
				}
				if (pathfile_callback)
					pathfile_callback(userdata, strtmp);
			}
		}
		closedir(dirp);
	}
}
void dirScan(const char* directory, const char* suffix_or_extension, DirScanFullCallback callback, void* userdata) {
	dirScanInternal(directory, suffix_or_extension, callback, NULL, userdata);
}
void dirScan(const char* directory, const char* suffix_or_extension, DirScanPathfileCallback callback, void* userdata) {
	dirScanInternal(directory, suffix_or_extension, NULL, callback, userdata);
}

#ifdef _WIN64
#include <Windows.h>

void dirCreate(const char* directory) {
	CreateDirectory(directory, NULL);
}

#endif