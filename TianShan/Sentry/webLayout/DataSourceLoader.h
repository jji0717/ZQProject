// DataSourceLoader.h: interface for the DataSourceLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATASOURCELOADER_H__3CDA41E8_E44C_4655_9208_5901C299B212__INCLUDED_)
#define AFX_DATASOURCELOADER_H__3CDA41E8_E44C_4655_9208_5901C299B212__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <TsLayout.h>
namespace ZQTianShan
{
namespace Layout
{

class DataSourceLoader  
{
public:
	DataSourceLoader(const char *dllpath, const char *logfilepath);
	~DataSourceLoader();
#ifdef ZQ_OS_MSWIN
    HMODULE DllHandle(){
        return _hDll;
    }
#else
	void* DllHandle()
	{
		return _hDll;
	}
#endif
private:
#ifdef ZQ_OS_MSWIN
    HMODULE _hDll;
#else
	void* _hDll;
#endif
    ZQTianShan::Layout::EntryFunc_Initialize _initProc;
    ZQTianShan::Layout::EntryFunc_Uninitialize _uninitProc;
};

}}//namespace ZQTianShan::Layout
#endif // !defined(AFX_DATASOURCELOADER_H__3CDA41E8_E44C_4655_9208_5901C299B212__INCLUDED_)
