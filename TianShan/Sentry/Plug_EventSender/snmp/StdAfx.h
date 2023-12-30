// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__13110055_DC3B_4A97_8B45_AE5A9E232D6A__INCLUDED_)
#define AFX_STDAFX_H__13110055_DC3B_4A97_8B45_AE5A9E232D6A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <ZQ_common_conf.h>
#include <windows.h>

#include <Log.h>
#include "SnmpSenderCfg.h"

#define LOG (*plog)
extern  ZQ::common::Log* plog;
extern Config::Loader< SnmpSenderInfo>* pSnmpSenderCfg;

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__13110055_DC3B_4A97_8B45_AE5A9E232D6A__INCLUDED_)
