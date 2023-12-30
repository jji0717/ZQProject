
//I dont' want include adebugmem.h, so define this macro
#define _AHEI_DEBUG_MEMORY_H__

//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG

#include <crtdbg.h>
void _RegDebugNew( void )
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
}
void* __cdecl operator new( size_t nSize, const char* lpszFileName, int nLine )
{
	void* p = _malloc_dbg( nSize, _NORMAL_BLOCK, lpszFileName, nLine );
	return p;
}
void __cdecl operator delete( void* p, const char* /*lpszFileName*/, int /*nLine*/ )
{    
    _free_dbg( p, _NORMAL_BLOCK ); 
}
#endif//_DEBUG