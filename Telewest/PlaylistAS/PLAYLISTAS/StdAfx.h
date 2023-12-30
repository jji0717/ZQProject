// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A397CF78_BFEB_45FC_8A8F_394D94468B21__INCLUDED_)
#define AFX_STDAFX_H__A397CF78_BFEB_45FC_8A8F_394D94468B21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <comutil.h>
#include <atlbase.h>

// below is to support MSSOAP3.0
#import "..\PlaylistSoapUDTMapper\ReleaseUMinDependency\PlaylistSoapUDTMapper.dll" 
using namespace PLAYLISTSOAPUDTMAPPERLib; 

#import "msxml4.dll" 

#import "mssoap30.dll" exclude("IStream", "ISequentialStream", "_LARGE_INTEGER", "_ULARGE_INTEGER", "tagSTATSTG", "_FILETIME", "tagSAFEARRAYBOUND", "IErrorInfo")
using namespace MSSOAPLib30;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A397CF78_BFEB_45FC_8A8F_394D94468B21__INCLUDED_)
