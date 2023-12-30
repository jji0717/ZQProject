// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__EC763C89_EFF3_4331_B406_53B874DCBED6__INCLUDED_)
#define AFX_STDAFX_H__EC763C89_EFF3_4331_B406_53B874DCBED6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <ZQ_common_conf.h>
// TODO: reference additional headers your program requires here
#include "ZQSnmp.h"
#include <snmp.h>
#include "Log.h"

extern ZQ::common::Log* _pReporter;
#define	SALOG		(*_pReporter)

using namespace ZQ::common;
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__EC763C89_EFF3_4331_B406_53B874DCBED6__INCLUDED_)
