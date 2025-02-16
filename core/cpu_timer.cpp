#include "cpu_timer.h"

static int64_t time_start = 0;

void timeSetStart() {
	time_start = getTime();
}

double secSinceStart() {
	return secFromTime(getTime() - time_start);
}
int64_t timeCounterSinceStart() {
	return getTime() - time_start;
}