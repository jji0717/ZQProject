// LogPageOpe.h: interface for the LogPageOpe class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGPAGEOPE_H__A035B96B_05BE_47EB_811E_A8962B549305__INCLUDED_)
#define AFX_LOGPAGEOPE_H__A035B96B_05BE_47EB_811E_A8962B549305__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
#include "windows.h"

class LogPageOpe  
{
public:
	void GetPointPage(INT64 nSize, DWORD nPage, const char* pFileName,const char* pType = "filelog");
	DWORD GetPageCount(INT64 nSize, const char* pFileName);
	void GetHelp();
	void GetSCLogPage(INT64 nSize,DWORD nPage, const char* pFileName);
	INT64 GetLogPos(HANDLE hFile);
	void Error();
	LogPageOpe();
	virtual ~LogPageOpe();

	void Analyse(int argc, char** argv);
};

#endif // !defined(AFX_LOGPAGEOPE_H__A035B96B_05BE_47EB_811E_A8962B549305__INCLUDED_)
