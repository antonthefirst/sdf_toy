#ifndef _HEADER_TIMER_TREE_
#define _HEADER_TIMER_TREE_

#if _WIN32
#include <Windows.h>
inline double getCurrTime() {
  	LARGE_INTEGER freq, time;
  	QueryPerformanceFrequency(&freq);
  	QueryPerformanceCounter(&time);
    return (double)((double)time.QuadPart / (double)freq.QuadPart);;
}
static __inline uint64_t getTimeCount()
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return (uint64_t)ticks.QuadPart;
}

static __inline uint64_t getTimeCountFreq()
{
	LARGE_INTEGER ticksPerSecond;
	QueryPerformanceFrequency(&ticksPerSecond);
	return (uint64_t)ticksPerSecond.QuadPart;
}
#else
//no timer :(
#endif

#include "maths.h"

#define TIMER_TREE_DEBUG
//#define DISABLE_TIMER

#ifdef TIMER_TREE_DEBUG
#include <stdio.h> //for printf
#endif

#define TIMER_TREE_MAX_TIMERS 256

struct TimerResult {
	const char* name;   //name of the timer
	const char* parent; //name of the parent
	int depth;          //the depth of the tree, useful for drawing
	float time;         //time in milliseconds
	int count;          //number of times the timer was started/stopped paused/unpaused
};

struct TimerState {
	const char* name;
	uint64_t start;
	uint64_t total;
	bool paused;
	int count;
	TimerResult* res;
};

class TimerTree {
	TimerResult results[TIMER_TREE_MAX_TIMERS];
	TimerResult* resultPtr;
	TimerState stateStack[TIMER_TREE_MAX_TIMERS];
	TimerState* statePtr;
public:
	TimerTree() {
		reset();
	}
	inline void reset() {
#ifndef DISABLE_TIMER
		statePtr = stateStack-1;
		resultPtr = results;
#endif
	}
	inline void start(const char* name) {
#ifndef DISABLE_TIMER
#ifdef TIMER_TREE_DEBUG
		if ((statePtr-stateStack) >= TIMER_TREE_MAX_TIMERS-1) { printf("TimerTree (start): Can't start any more timers, stack full.\n"); return; }
		if ((resultPtr-results) >= TIMER_TREE_MAX_TIMERS-2) { printf("TimerTree (start): Can't start any more timers, results full (%d).\n", int(resultPtr-results)); return; } //-2 because resultPtr starts equal to results, unlike statePtr
#endif
		++statePtr;
		statePtr->name = name;
		statePtr->start = getTimeCount();
		statePtr->total = 0;
		statePtr->count = 0;
		statePtr->paused = false;
		statePtr->res = resultPtr++;
#endif
	}
	//same as start, but initializes start to 0 (to be used with "unpause/pause" subtiming)
	inline void startPaused(const char* name) {
#ifndef DISABLE_TIMER
#ifdef TIMER_TREE_DEBUG
		if ((statePtr-stateStack) >= TIMER_TREE_MAX_TIMERS-1) { printf("TimerTree (startPaused): Can't start any more timers, stack full.\n"); return; }
		if ((resultPtr-results) >= TIMER_TREE_MAX_TIMERS-2) { printf("TimerTree (start): Can't start any more timers, results full (%d).\n", int(resultPtr-results)); return; } //-2 because resultPtr starts equal to results, unlike statePtr
#endif
		++statePtr;
		statePtr->name = name;
		statePtr->start = 0;
		statePtr->total = 0;
		statePtr->count = 0;
		statePtr->paused = true;
		statePtr->res = resultPtr++;
#endif
	}
	inline void unpause() {
#ifndef DISABLE_TIMER
		statePtr->start = getTimeCount();
		statePtr->paused = false;
#endif
	}
	inline void pause() {
#ifndef DISABLE_TIMER
		uint64_t currTime = getTimeCount();
		statePtr->total += currTime - statePtr->start;
		statePtr->paused = true;
		++statePtr->count;
#endif
	}
	inline void stop() {
#ifndef DISABLE_TIMER
		//get time asap
		uint64_t currTime = getTimeCount();

#ifdef TIMER_TREE_DEBUG
		if ((statePtr-stateStack) < 0) { printf("TimerTree (stop): No timers to stop.\n"); return; }
#endif
		//if the timer was running, add the current time to the total
		if (!statePtr->paused) {
			statePtr->total += currTime - statePtr->start;
			++statePtr->count;
		}
		//if it was not then do nothing (total was already computed by pause() or not at all)
		else {

		}

		static float tfreq = 0.0f;
		if (tfreq==0.0f) tfreq = (float)getTimeCountFreq();
		statePtr->res->time = statePtr->total/tfreq*1000.0f;
		statePtr->res->name = statePtr->name;
		statePtr->res->parent = (statePtr-1)->name;
		statePtr->res->depth = int(statePtr - stateStack);
		statePtr->res->count = statePtr->count;
		--statePtr;
#endif
	}
	inline int getResults(TimerResult* res, int maxTimers) {
#ifndef DISABLE_TIMER
		int num = min(maxTimers, int(resultPtr-results));
		memcpy(res, results, num*sizeof(TimerResult));
		return num;
#else
		return 0;
#endif
	}
};

struct PrintTimer {
	uint64_t t;
	const char* name;
	PrintTimer(const char* n = "")  {
		name = n;
		t = getTimeCount();
	}
	~PrintTimer() {
		t = getTimeCount() - t;
		static float tfreq = 0.0f;
		if (tfreq == 0.0f) tfreq = (float)getTimeCountFreq();
		printf("%s - %fms (%lld ticks)\n", name, t / tfreq * 1000.0f, t);
	}
};

#include <deque>
template<typename T>
class MovingAvg {
	std::deque<T> samples;
	int samples_max;
public:
	MovingAvg() {
		window(1);
	}
	MovingAvg(int num_samples) {
		window(num_samples);
	}
	void window(int num_samples) {
		samples_max = num_samples;
	}
	int numSamples() {
		return samples.size();
	}
	void push(const T& val) {
		samples.push_front(val);
		while ((int)samples.size() > samples_max)
			samples.pop_back();
	}
	void fill(const T& val) {
		samples.resize(samples_max);
		for (unsigned int i = 0; i < samples.size(); ++i)
			samples[i] = val;
	}
	T average() {
		if (samples.size()) {
			T sum = 0;
			for (unsigned int i = 0; i < samples.size(); ++i)
				sum += samples[i];
			return sum / T(samples.size());
		} else {
			return 0;
		}
	}
	T spread() {
		if (samples.size()) {
			T smin = samples[0];
			T smax = samples[0];
			for (unsigned int i = 0; i < samples.size(); ++i) {
				if (samples[i] < smin) smin = samples[i];
				if (samples[i] > smax) smax = samples[i];
			}
			return smax-smin;
		} else {
			return 0;
		}
	}
};

struct TimerResultAvg {
	const char* name;
	float time;
	float spread;
};

#include <map>
class TimerTreeAvg {
	typedef std::map<const char*, MovingAvg<float> > ResultMap;
	ResultMap result_map;
	int window;
public:
	TimerTreeAvg() : window(1) { }
	TimerTreeAvg(int num_samples) : window(num_samples) { }
	inline void pushResults(TimerResult* res, int num_results) {
		for (int i = 0; i < num_results; ++i) {
			result_map[res[i].name].window(window);
			result_map[res[i].name].push(res[i].time);
		}
	}
	inline int getResults(TimerResultAvg* res, int max_results) {
		//int num = std::min((int)result_map.size(), max_results);
		int i = 0;
		for (ResultMap::iterator r = result_map.begin(); r != result_map.end(); ++r) {
			if (i >= max_results) break;
			res[i].name = r->first;
			res[i].time = r->second.average();
			res[i].spread = r->second.spread();
			++i;
		}
		return i;
	}
};

#endif
