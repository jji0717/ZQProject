#ifndef _DEBUG_MEMORY_LEAK_
#define _DEBUG_MEMORY_LEAK_


#ifdef _DEBUG
#define DEBUG_CLIENTBLOCK   new( _NORMAL_BLOCK, __FILE__, __LINE__)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new DEBUG_CLIENTBLOCK
#else
#define DEBUG_CLIENTBLOCK
#endif



#endif//_DEBUG_MEMORY_LEAK_
