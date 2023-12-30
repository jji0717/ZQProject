// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__20C2869D_44DB_438D_9921_F709650BE5CF__INCLUDED_)
#define AFX_STDAFX_H__20C2869D_44DB_438D_9921_F709650BE5CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#define loglog(x) {  FILE *pFile;\
	                 DWORD dwTime=timeGetTime();\
	                 pFile=fopen("c:\\SW_log.txt","a+");\
				     fprintf(pFile,"[%.6d]%s\n",dwTime,(x));\
				     fclose(pFile);\
                  }
#include "SMGLog.h"
// TODO: reference additional headers your program requires here
//#define LOGLEVELNUMBER	 5					//log level number, when your log level is greater than it, log is not written to file. 
//#define LOGFILENAME		"DataWatcherSource.log"	//log file name.
//#define LOGFILESIZE		 2*1024*1024	    //log file max size. 2MB.

#define NEED_LOGINFORMATION
#define NEED_EVENTLOG

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__20C2869D_44DB_438D_9921_F709650BE5CF__INCLUDED_)
