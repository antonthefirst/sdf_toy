#include "dev_menus.h"
#include "core/crc.h"
#include <map>

// Pointers into hash should not invalidate, so use map.
static std::map<int, bool> isOpen;

static int hashStr(const char* str) {
	return crc8(127, str, (int)strlen(str));
}

bool* getDevMenuOpen(const char* menu) {
	int h = hashStr(menu);
	if (isOpen.find(h) == isOpen.end())
		isOpen[h] = false; // Initialize to false.
	return &isOpen[h];
}

void setDevMenuOpen(const char* menu, bool open) {
	isOpen[hashStr(menu)] = open;
}