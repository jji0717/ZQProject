// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__14F8332A_1B65_4EE6_B377_4A0EEDBAFCF6__INCLUDED_)
#define AFX_STDAFX_H__14F8332A_1B65_4EE6_B377_4A0EEDBAFCF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <ZQ_common_conf.h>
#include <windows.h>

extern DWORD _iLogSlot; //tls index
#define LOGPTR          ((ZQ::common::Log*)(TlsGetValue(_iLogSlot)))
#define	TSLOG           (*LOGPTR)

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__14F8332A_1B65_4EE6_B377_4A0EEDBAFCF6__INCLUDED_)
