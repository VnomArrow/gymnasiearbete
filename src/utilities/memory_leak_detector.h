#pragma once
/*
memory leak detector

print out all memory not returned at the end of the program,
must be included in all files and requires mem_detect::init() 
being called once anytime before the program exits

requirements:
vs_community
*/


#if defined(__has_include)
#if __has_include("crtdbg.h")
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifdef _DEBUG
#define SUPPORT_FOR_CRTDBG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif
#endif

namespace mem_detect {
	inline void init() {
		#if defined(SUPPORT_FOR_CRTDBG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		#endif
	}
}