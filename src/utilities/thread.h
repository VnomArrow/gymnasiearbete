/*
makes thread work for both vs_community
and vs_code using mingw
*/

#pragma once
#include <thread>
#if defined(_GLIBCXX_THREAD)
    #include "mingw-std-threads-master/mingw.thread.h"
#endif

//pass function followed by args
#define thread__createThread(...) \
{ std::thread t1(__VA_ARGS__); \
t1.detach(); }

#define thread__maxThreads() \
std::thread::hardware_concurrency()