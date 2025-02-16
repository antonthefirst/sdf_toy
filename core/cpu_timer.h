#pragma once
#include "platform.h" // for osTimer

__inline Time getTime() {
	return (Time)osTimer();
}
__inline double getTimeFrequency() {
	return (double)osTimerFrequency();
}
__inline double secFromTime(Time t) {
	static const double f = (double)osTimerFrequency();
	return double(t) / f;
}
_inline double msecFromTime(Time t) {
	return secFromTime(t) * 1000.0;
}
__inline double secBetween(Time from, Time to) {
	return secFromTime(to - from);
}
__inline double msecBetween(Time from, Time to) {
	return secFromTime(to - from) * 1000.0;
}
__inline double secSince(Time from) {
	return secBetween(from, getTime());
}
__inline double msecSince(Time from) {
	return msecBetween(from, getTime());
}

// simple timer for code blocks
struct BlockTimer {
	Time time_start;
	const char* name;
	BlockTimer(const char* n = "") {
		name = n;
		time_start = getTime();
	}
	~BlockTimer() {
		Time time_now = getTime();
		print("Block time of %s: %fms", name, msecBetween(time_start, time_now));
	}
};

// simple timer from start of app
void timeSetStart();
double secSinceStart();
int64_t timeCounterSinceStart();