   // ===========================================================================

#ifndef __MINIDUMP_H__
#define __MINIDUMP_H__

#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <stdlib.h>

extern "C"
{
#include <dbghelp.h>
}


// -----------------------------
// class MiniDump
// -----------------------------
class MiniDump
{
public:
	typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);

	MiniDump();
	void SaveProcessLog(TCHAR* DumpPath =NULL, TCHAR* ModuleName =NULL,PROCESS_INFORMATION *proc_info =NULL);
		
	BOOL setDumpPath(TCHAR* DumpPath);
	void enableFullMemoryDump(BOOL bEnable){ _bEnableFullMemoryDump = bEnable;}

	typedef void (WINAPI *ExceptionCallBack)(DWORD ExceptionCode, PVOID ExceptionAddress);
	
	//this callback will be called after the minidump created
	void setExceptionCB(ExceptionCallBack cb);
	LONG  CallProceAPI();
protected:
	
	static LONG WINAPI _OnUnhandledException( struct _EXCEPTION_POINTERS *pExceptionInfo );
	static TCHAR _moduleName[128], _dumpPath[_MAX_PATH];
	static TCHAR _moduleFullPath[_MAX_PATH];
	static LONG _hOldFilter;
	static BOOL _bEnableFullMemoryDump;
	static ExceptionCallBack		_pExceptionCB;
	static PROCESS_INFORMATION     *m_proc_info;
};
#endif // __MINIDUMP_H__