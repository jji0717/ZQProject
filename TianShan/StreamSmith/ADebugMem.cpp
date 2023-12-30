
//I dont' want include adebugmem.h, so define this macro
#define __DEBUG_MEMORY_H__

//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#include <Windows.h>
#include <crtdbg.h>
#include <string.h>
#include <stdlib.h>

//#include "vld.h"

_CrtMemState state1,state2,state3;
void _RegDebugNew( void )
{
	_CrtSetDbgFlag ( _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF );
	//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF|_CRTDBG_REPORT_FLAG );
	_CrtMemCheckpoint(&state1);
   
}
void* __cdecl operator new( size_t nSize, const char* lpszFileName, int nLine )
{
	return ::operator new(nSize, _NORMAL_BLOCK, lpszFileName, nLine); 
	//void* p = _malloc_dbg( nSize, _CLIENT_BLOCK, lpszFileName, nLine );
	//return p;
}
void __cdecl operator delete( void* p, const char* /*lpszFileName*/, int /*nLine*/ )
{    
    _free_dbg( p, _NORMAL_BLOCK ); 
}

void dumpDifferent(_CrtMemState* state);
void __declspec(dllexport) ShowMemoryDifference()
{
//	_CrtMemCheckpoint(&state2);
//	_CrtMemDifference(&state3,&state1,&state2);
	_CrtMemDumpAllObjectsSince(&state1);
//	dumpDifferent(&state3);
	_CrtMemCheckpoint(&state1);
}

typedef struct _CrtMemBlockHeader
{
        struct _CrtMemBlockHeader * pBlockHeaderNext;
        struct _CrtMemBlockHeader * pBlockHeaderPrev;
        char *                      szFileName;
        int                         nLine;
        size_t                      nDataSize;
        int                         nBlockUse;
        long                        lRequest;
        unsigned char               gap[4];
        /* followed by:
         *  unsigned char           data[nDataSize];
         *  unsigned char           anotherGap[nNoMansLandSize];
         */
} _CrtMemBlockHeader;
void dumpDifferent(_CrtMemState* state)
{
	_CrtMemBlockHeader* _current = state->pBlockHeader;
	_CrtMemBlockHeader* _prev = (_current==NULL?NULL:_current->pBlockHeaderPrev);
	char szBuf[2048];
	char szTemp[32];
	while (_current)
	{
		if(_current->szFileName && _current->nBlockUse == _CLIENT_BLOCK)
		{
			strcpy(szBuf,_current->szFileName);
			strcat(szBuf,"(");
			strcat(szBuf,itoa(_current->nLine,szTemp,10));
			strcat(szBuf,"): dataSize:");
			strcat(szBuf,itoa((int)_current->nDataSize,szTemp,10));
			strcat(szBuf,"\n");
			OutputDebugStringA(szBuf);
		}
		_current=_current->pBlockHeaderNext;
	}
	_current=_prev;
	while (_current)
	{
		if(_current->szFileName && _current->nBlockUse == _CLIENT_BLOCK)
		{
			strcpy(szBuf,_current->szFileName);
			strcat(szBuf,"(");
			strcat(szBuf,itoa(_current->nLine,szTemp,10));
			strcat(szBuf,"): dataSize:");
			strcat(szBuf,itoa((int)_current->nDataSize,szTemp,10));
			strcat(szBuf,"\n");
			OutputDebugStringA(szBuf);
		}
		_current=_current->pBlockHeaderPrev;
	}
}
#endif//_DEBUG