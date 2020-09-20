#pragma once
#include "memory_leak_detector.h"
#include <random>

namespace rand_ {
	// Better random number generator
	typedef std::mt19937 rng_type_32;
	typedef std::mt19937_64 rng_type_64;

	// Local thread random generation
	thread_local static std::random_device thread_rd;
	thread_local static rng_type_64 thread_rng(thread_rd());

	inline void localThreadSetSeed(unsigned long long seed) {
		thread_rng.seed(seed);
	}
}

#define rand__randomNumber rand_::thread_rng()