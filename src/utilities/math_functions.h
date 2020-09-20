#pragma once
#include "memory_leak_detector.h"
#include <math.h>

// Modulo but always positive
inline int pMod(int num, int mod) {
	return ((num % mod) + mod) % mod;
}
inline double pMod(double num, int mod) {
	return fmod(fmod(num, mod) + mod, mod);
}
inline double pMod(double num, double mod) {
	return fmod(fmod(num, mod) + mod, mod);
}

#define PI 3.14159265

inline double degreesFromOrigin(double x, double y) {
	if (x != 0) {
		return 180 * (double)(x < 0) + (atan(y / x) * 180) / PI;
	}
	return 90 + (double)(y < 0) * 180;
}

inline double radiansFromOrigin(double x, double y) {
	if (x != 0) {
		return (x < 0) * PI + (atan(y / x) * 180);
	}
	return PI / 2 + (y < 0) * PI;
}

// See if value is between two other values
template<typename T>
inline bool isBetween(T val, T min, T max) { return val > min && val < max; }