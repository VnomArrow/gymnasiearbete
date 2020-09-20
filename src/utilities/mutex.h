/*
makes mutex work for both vs_community
and vs_code using mingw
*/

#pragma once
#include <mutex>
#if defined(_GLIBCXX_MUTEX)
// vs_code using mingw
	#include "mingw-std-threads-master/mingw.mutex.h"
#endif
