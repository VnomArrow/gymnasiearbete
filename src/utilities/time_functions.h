#pragma once
#include "memory_leak_detector.h"
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include "thread.h"
#include "mutex.h"

inline std::tm localtime_xp(std::time_t timer) {
	std::tm bt{};
#if defined(__unix__)
	localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
	localtime_s(&bt, &timer);
#else
	static std::mutex mtx;
	std::lock_guard<std::mutex> lock(mtx);
	bt = *std::localtime(&timer);
#endif
	return bt;
}

// default = "YYYY-MM-DD HH:MM:SS"
inline std::string time_stamp(const std::string& fmt = "%F %T") {
	auto bt = localtime_xp(std::time(0));
	char buf[64];
	return { buf, std::strftime(buf, sizeof(buf), fmt.c_str(), &bt) };
}

//get number of milliseconds since 1970
inline long long int getTimeSinceEpoc() {
	return(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}

//warning! may sleep longer than expected
inline void sleep(long long int milliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

#define time__createTimer long long time__simpleTimer = getTimeSinceEpoc();
#define time__resetTimer time__simpleTimer = getTimeSinceEpoc()
#define time__getTimerTime (getTimeSinceEpoc() - time__simpleTimer)

class FpsTracker {
public:
	FpsTracker(int t_fps) { fps = t_fps; start_time = getTimeSinceEpoc(); }
	unsigned long long nextUpdate() { return ((1000 * update_count + 1) / fps) + start_time + paused_time; }
	bool isUpdateTime() { return (getTimeSinceEpoc() > nextUpdate() && pause_start_time == 0); }
	void waitToNextUpdate() { sleep(getTimeSinceEpoc() - nextUpdate()); }
	void pause() { pause_start_time = getTimeSinceEpoc(); }
	void unpause() { paused_time += getTimeSinceEpoc() - pause_start_time; pause_start_time = 0; }
	unsigned int update_count = 0;
	unsigned long long start_time;
	unsigned long long pause_start_time = 0;
	unsigned long long paused_time = 0;
private:
	FpsTracker();
	unsigned int fps;
};