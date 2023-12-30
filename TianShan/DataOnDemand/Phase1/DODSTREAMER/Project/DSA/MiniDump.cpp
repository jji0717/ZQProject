// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: MiniDump.cpp,v 1.20 2004/07/26 10:56:35 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : A mini dump for the application crash
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase1/DODSTREAMER/Project/DSA/MiniDump.cpp $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 1     08-12-08 11:11 Li.huang
// 
// 2     07-01-31 16:23 Cary.xiao
// 
// 2     07-01-11 11:40 Cary.xiao
// 
// 1     07-01-03 12:42 Cary.xiao
// 
// 1     06-10-27 12:37 Cary.xiao
// 
// 5     05-11-24 14:31 Jie.zhang
// add full memory dump
// 
// 4     05-09-09 16:37 Jie.zhang
// 
// 3     05-09-09 11:12 Jie.zhang
// 
// 2     05-09-09 9:37 Jie.zhang
// 
// 1     8/25/05 5:25p Hui.shao
// ===========================================================================
#include <tchar.h>
#include "MiniDump.h"
#include <stdio.h>


namespace ZQ {
	namespace common {


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
		BOOL MiniDump::_bEnableFullMemoryDump = FALSE;
		
		void MiniDump::trace(const TCHAR *fmt, ...)
		{
#ifdef _DEBUG
			TCHAR msg[2048];
			va_list args;
			
			va_start(args, fmt);
			_vstprintf(msg, fmt, args);
			va_end(args);
			
			::MessageBox(NULL, msg, _moduleName, MB_OK);
#endif // _DEBUG
		}
		
		MiniDump::MiniDump(TCHAR* DumpPath, TCHAR* ModuleName)
		{
			if (NULL != _hOldFilter) // already set
				return;
			
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

					trace(_T("Dump path %s is error, use default %s as current dump path"), DumpPath, _dumpPath);
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
			_hOldFilter = (LONG) ::SetUnhandledExceptionFilter( _OnUnhandledException );
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
				trace(_T("loading %s"), DbgHelpPath);
				hDll = ::LoadLibrary( DbgHelpPath );
			}
			
			if (NULL == hDll)
			{
				// load any version in the search path
				trace(_T("loading %s"), DBGHELP_DLL);
				hDll = ::LoadLibrary(DBGHELP_DLL);
			}
			
			MINIDUMPWRITEDUMP pDump = NULL;
			if (NULL == hDll || NULL == (pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, DUMP_METHOD)))
			{
				trace(_T("failed to load expected %s"), DBGHELP_DLL);
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
			
			trace(_T("hDll=%08x; pDump=%08x"), hDll, pDump);
			TCHAR Scratch[_MAX_PATH];
			
			TCHAR  tszFileName[512];
			{
				//set the file name as module name & time leak20050215121510.dmp
				SYSTEMTIME st;
				GetLocalTime(&st);
				_stprintf(tszFileName, _T("%s%s%d%02d%02d%02d%02d%02d.dmp"), _dumpPath, _moduleName,
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			}
			trace(_T("FileName=%s"), tszFileName);

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
				BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, (MINIDUMP_TYPE) (dwFlag), &ExInfo, NULL, NULL );
				if (bOK)
				{
					_stprintf(Scratch, _T("Saved dump file to '%s'"), _dumpPath );
					trace(_moduleName, Scratch);
					retval = EXCEPTION_EXECUTE_HANDLER;
				}
				else
				{
					_stprintf( Scratch, _T("Failed to save dump file to '%s' (error %d)"), _dumpPath, GetLastError() );
					trace(_moduleName, Scratch);
				}
				
				::CloseHandle(hFile);
			}
			else
			{
				_stprintf( Scratch, _T("Failed to create dump file '%s' (error %d)"), _dumpPath, GetLastError() );
				trace(_moduleName, Scratch);
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
		
		void MiniDump::setExceptionCB(ExceptionCallBack cb)
		{
			_pExceptionCB = cb;
		}
} // namespace common
} // namespace ZQ
