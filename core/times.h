#pragma once
#include <time.h>       /* time_t, struct tm, time, localtime */

#pragma warning(disable:4996) // Deprecated localtime

inline void timeNow(time_t time_now, int* year, int* month, int* day, int* hour, int* minute, int* second) {
	struct tm * timeinfo;
	timeinfo = localtime(&time_now);
	if (!timeinfo) return;
	if (year) *year = 1900 + timeinfo->tm_year;
	if (month) *month = 1 + timeinfo->tm_mon;
	if (day) *day = timeinfo->tm_mday;
	if (hour) *hour = timeinfo->tm_hour;
	if (minute) *minute = timeinfo->tm_min;
	if (second) *second = timeinfo->tm_sec;
}

// Returns seconds since epoch (1900)
// It's well known that time_t is seconds since epoch, but it's not technically guaranteed by the spec.
// It's handy having a monotonically increasing 32-bit int though, even though it'll overflow in 2038.
inline uint32_t secSinceEpoch32() {
	return uint32_t(time(nullptr));
}