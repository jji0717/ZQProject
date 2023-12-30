// ===========================================================================
#include "MDump.h"
#include <tchar.h>
#include <stdio.h>


//check the path, if not exist then create it, if create fail then return false
static bool validatePath(const TCHAR *     wszPath )
{
	if (-1 != ::GetFileAttributes(wszPath))
		return true;
	
	DWORD dwErr = ::GetLastError();
	if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
	{
		if (!::CreateDirectory(wszPath, NULL))
		{
			dwErr = ::GetLastError();
			if ( dwErr != ERROR_ALREADY_EXISTS)
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	
	return true;
}


#define DBGHELP_DLL _T("DBGHELP.DLL")
#define DUMP_METHOD "MiniDumpWriteDump"

		LONG MiniDump::_hOldFilter = NULL;
		TCHAR MiniDump::_moduleName[128], MiniDump::_dumpPath[_MAX_PATH], MiniDump::_moduleFullPath[_MAX_PATH];
		MiniDump::ExceptionCallBack	MiniDump::_pExceptionCB = NULL;
		BOOL MiniDump::_bEnableFullMemoryDump = TRUE;
		
		PROCESS_INFORMATION     *  MiniDump::m_proc_info =NULL;
		MiniDump::MiniDump()
		{

		}
		void MiniDump::SaveProcessLog(TCHAR* DumpPath, TCHAR* ModuleName,PROCESS_INFORMATION *proc_info)
		{
			if (NULL != _hOldFilter) // already set
				return;
			
			m_proc_info = (PROCESS_INFORMATION*)malloc(sizeof(PROCESS_INFORMATION));
			m_proc_info->hProcess = NULL;
			m_proc_info->hThread = NULL;
			m_proc_info->dwProcessId = 0;
			m_proc_info->dwThreadId = 0;
			m_proc_info = proc_info;

			::GetModuleFileName(NULL, _moduleFullPath, _MAX_PATH);
			
			TCHAR* pos = _tcsrchr(_moduleFullPath, _T('\\'));
			if (ModuleName == NULL)
			{
				_tcscpy(_moduleName, (NULL != pos) ? pos+1 : _moduleFullPath);
				pos  = _tcsrchr(_moduleName, _T('.'));	//remove ".exe"
				if (pos)
					*pos = _T('\0');
			}
			else
				_tcscpy(_moduleName, ModuleName);
			
			if (DumpPath == NULL)
			{
				// work out a good place for the dump file
				if (GetTempPath( _MAX_PATH, _dumpPath ) == 0)
					_tcscpy( _dumpPath, _T("c:\\temp\\") );
			}
			else
			{
				if (!validatePath(DumpPath))
				{
					// work out a good place for the dump file
					if (GetTempPath( _MAX_PATH, _dumpPath ) == 0)
						_tcscpy( _dumpPath, _T("c:\\temp\\") );

					
				}
				else
				{
					_tcscpy(_dumpPath, DumpPath);

					// check if the last char if is '\\'
					{
						TCHAR* pPtr = _dumpPath;
						while (*pPtr) pPtr++;	// to the end
						pPtr--;
						while (*pPtr==_T(' '))pPtr--;	//remove the ' ' if any
						if (*pPtr!=_T('\\'))
						{
							pPtr++;
							*pPtr++ = _T('\\');
							*pPtr =  _T('\0');
						}
						else
						{
							pPtr++;
							*pPtr =  _T('\0');
						}
					}
				}
			}
				
			// now register the unhandled exception handler
//			_hOldFilter = (LONG) ::SetUnhandledExceptionFilter( _OnUnhandledException );
		}
		
		BOOL MiniDump::setDumpPath(TCHAR* DumpPath)
		{
			if (!DumpPath)
				return FALSE;

			// verify the file
			if (!validatePath(DumpPath))
				return FALSE;

			_tcscpy(_dumpPath, DumpPath);

			// check if the last char if is '\\'
			{
				TCHAR* pPtr = _dumpPath;
				while (*pPtr) pPtr++;	// to the end
				pPtr--;
				while (*pPtr==_T(' '))pPtr--;	//remove the ' ' if any
				if (*pPtr!=_T('\\'))
				{
					pPtr++;
					*pPtr++ = _T('\\');
					*pPtr =  _T('\0');
				}
				else
				{
					pPtr++;
					*pPtr =  _T('\0');
				}
			}

			return TRUE;
		}

		LONG  MiniDump::CallProceAPI()
		{
			LONG retval = EXCEPTION_CONTINUE_SEARCH;
			
			// firstly see if dbghelp.dll is in the same dirctory of the application
			HMODULE hDll = NULL;
			TCHAR DbgHelpPath[_MAX_PATH];
			
			TCHAR *pos = _tcsrchr(_moduleFullPath, _T('\\'));
			if (pos)
			{
				_tcscpy(DbgHelpPath, _moduleFullPath);
				_tcscpy(DbgHelpPath + (pos - _moduleFullPath +1), DBGHELP_DLL );
				
				hDll = ::LoadLibrary( DbgHelpPath );
			}
			
			if (NULL == hDll)
			{
				// load any version in the search path
				
				hDll = ::LoadLibrary(DBGHELP_DLL);
			}
			
			MINIDUMPWRITEDUMP pDump = NULL;
			if (NULL == hDll || NULL == (pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, DUMP_METHOD)))
			{
				
				try
				{
					// TODO: place code here to do some last minute processing in the face
					// of an unhandled exception before Dr. Watson kills us and creates a
					// crash dump.
					//#pragma message("TODO: Implement code to flush logs etc. in the face of an unhandled exception")
					
					retval = EXCEPTION_CONTINUE_SEARCH;
				}
				catch (...)
				{
				}
				
				

				return retval;
			}
			
			
			TCHAR Scratch[_MAX_PATH];
			
			TCHAR  tszFileName[512];
			{
				//set the file name as module name & time leak20050215121510.dmp
				SYSTEMTIME st;
				GetLocalTime(&st);
				_stprintf(tszFileName, _T("%s%s%d%02d%02d%02d%02d%02d.dmp"), _dumpPath, _moduleName,
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			}
			
			

			// create the file
			HANDLE hFile = ::CreateFile( tszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL );
			
			if (hFile!=INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
								
				ExInfo.ThreadId = m_proc_info->dwThreadId;
				ExInfo.ExceptionPointers = NULL;
				ExInfo.ClientPointers = FALSE;
				
				
				
				DWORD dwFlag = MiniDumpWithDataSegs|MiniDumpNormal;
				if (_bEnableFullMemoryDump)
					 dwFlag = dwFlag | MiniDumpWithFullMemory;

				// write the dump
			//	BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, (MINIDUMP_TYPE) (dwFlag), &ExInfo, NULL, NULL );
//				BOOL bOK = pDump( m_proc_info->hProcess,m_proc_info->dwProcessId,hFile,(MINIDUMP_TYPE)(dwFlag),&ExInfo, NULL,NULL);
				BOOL bOK = pDump( m_proc_info->hProcess,m_proc_info->dwProcessId,hFile,(MINIDUMP_TYPE)(dwFlag),NULL, NULL,NULL);
				if (bOK)
				{
					_stprintf(Scratch, _T("Saved dump file to '%s'"), tszFileName );
					
					retval = EXCEPTION_EXECUTE_HANDLER;
				}
				else
				{
					_stprintf( Scratch, _T("Failed to save dump file to '%s' (error %d)"), tszFileName, GetLastError() );
					
				}
				
				::CloseHandle(hFile);
			}
			else
			{
				_stprintf( Scratch, _T("Failed to create dump file '%s' (error %d)"), tszFileName, GetLastError() );
				
			}

			// call the callback if not NULL
			if (_pExceptionCB)
			{
				DWORD dwExceptionCode = 0;
				PVOID pExceptionAddr = 0;

				
			}
			::MessageBox( NULL, Scratch,_T("Information"), MB_OK );
			return retval;


		}
		LONG WINAPI MiniDump::_OnUnhandledException(struct _EXCEPTION_POINTERS *pExceptionInfo )
		{
			LONG retval = EXCEPTION_CONTINUE_SEARCH;
			
			// firstly see if dbghelp.dll is in the same dirctory of the application
			HMODULE hDll = NULL;
			TCHAR DbgHelpPath[_MAX_PATH];
			
			TCHAR *pos = _tcsrchr(_moduleFullPath, _T('\\'));
			if (pos)
			{
				_tcscpy(DbgHelpPath, _moduleFullPath);
				_tcscpy(DbgHelpPath + (pos - _moduleFullPath +1), DBGHELP_DLL );
				
				hDll = ::LoadLibrary( DbgHelpPath );
			}
			
			if (NULL == hDll)
			{
				// load any version in the search path
				
				hDll = ::LoadLibrary(DBGHELP_DLL);
			}
			
			MINIDUMPWRITEDUMP pDump = NULL;
			if (NULL == hDll || NULL == (pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, DUMP_METHOD)))
			{
				
				try
				{
					// TODO: place code here to do some last minute processing in the face
					// of an unhandled exception before Dr. Watson kills us and creates a
					// crash dump.
					//#pragma message("TODO: Implement code to flush logs etc. in the face of an unhandled exception")
					
					retval = EXCEPTION_CONTINUE_SEARCH;
				}
				catch (...)
				{
				}
				
				// call the callback if not NULL
				if (_pExceptionCB)
				{
					DWORD dwExceptionCode = 0;
					PVOID pExceptionAddr = 0;

					if (pExceptionInfo && pExceptionInfo->ExceptionRecord)
					{
						dwExceptionCode = pExceptionInfo->ExceptionRecord->ExceptionCode;
						pExceptionAddr = pExceptionInfo->ExceptionRecord->ExceptionAddress;
					}

					_pExceptionCB(dwExceptionCode, pExceptionAddr);
				}

				return retval;
			}
			
			
			TCHAR Scratch[_MAX_PATH];
			
			TCHAR  tszFileName[512];
			{
				//set the file name as module name & time leak20050215121510.dmp
				SYSTEMTIME st;
				GetLocalTime(&st);
				_stprintf(tszFileName, _T("%s%s%d%02d%02d%02d%02d%02d.dmp"), _dumpPath, _moduleName,
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			}
			

			// create the file
			HANDLE hFile = ::CreateFile( tszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL );
			
			if (hFile!=INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
				
				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = pExceptionInfo;
				ExInfo.ClientPointers = FALSE;
				
				DWORD dwFlag = MiniDumpWithDataSegs|MiniDumpNormal;
				if (_bEnableFullMemoryDump)
					 dwFlag = dwFlag | MiniDumpWithFullMemory;

				// write the dump
			//	BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, (MINIDUMP_TYPE) (dwFlag), &ExInfo, NULL, NULL );
				BOOL bOK = pDump( m_proc_info->hProcess,m_proc_info->dwProcessId,hFile,(MINIDUMP_TYPE)(dwFlag),&ExInfo, NULL,NULL);
				if (bOK)
				{
					_stprintf(Scratch, _T("Saved dump file to '%s'"), tszFileName );
					
					retval = EXCEPTION_EXECUTE_HANDLER;
				}
				else
				{
					_stprintf( Scratch, _T("Failed to save dump file to '%s' (error %d)"), tszFileName, GetLastError() );
					
				}
				
				::CloseHandle(hFile);
			}
			else
			{
				_stprintf( Scratch, _T("Failed to create dump file '%s' (error %d)"), tszFileName, GetLastError() );
				
			}

			// call the callback if not NULL
			if (_pExceptionCB)
			{
				DWORD dwExceptionCode = 0;
				PVOID pExceptionAddr = 0;

				if (pExceptionInfo && pExceptionInfo->ExceptionRecord)
				{
					dwExceptionCode = pExceptionInfo->ExceptionRecord->ExceptionCode;
					pExceptionAddr = pExceptionInfo->ExceptionRecord->ExceptionAddress;
				}

				_pExceptionCB(dwExceptionCode, pExceptionAddr);
			}
			::MessageBox( NULL, Scratch,_T("Information"), MB_OK );
			return retval;
		}
		
		void MiniDump::setExceptionCB(ExceptionCallBack cb)
		{
			_pExceptionCB = cb;
		}
