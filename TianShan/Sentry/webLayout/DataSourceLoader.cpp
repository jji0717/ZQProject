// DataSourceLoader.cpp: implementation of the DataSourceLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DataSourceLoader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace ZQTianShan
{
namespace Layout
{

#ifdef ZQ_OS_MSWIN
//_hDll should be NULL if any error occur during the init
DataSourceLoader::DataSourceLoader(const char *dllpath, const char *logfilepath)
:_hDll(NULL), _initProc(NULL), _uninitProc(NULL)
{
    if(NULL == dllpath || NULL == logfilepath)
        return;

    _hDll = LoadLibraryA(dllpath);
    if(NULL == _hDll)
    {
        //glog(ZQ::common::Log::L_ERROR,"LoadDataSource() can't load dataSource %s and last error code is %u",attr._strDataSource.c_str(),GetLastError());
        return;
    }
    _initProc = (EntryFunc_Initialize)GetProcAddress(_hDll , "TCInitialize");
    _uninitProc = (EntryFunc_Uninitialize)GetProcAddress(_hDll,"TCUninitialize");
    if(NULL == _initProc || NULL == _uninitProc)
    {
        FreeLibrary(_hDll);
        _hDll = NULL;
        return;
    }
    try{
        if(0 != _initProc(logfilepath))
        {
            FreeLibrary(_hDll);
            _hDll = NULL;
            return;
        }
    }catch(...)
    {
        FreeLibrary(_hDll);
        _hDll = NULL;
        return;
    }
}

DataSourceLoader::~DataSourceLoader()
{
    if(NULL == _hDll)
        return;

    if(_uninitProc){
        try{
            _uninitProc();
        }catch(...){
        }
    }
    FreeLibrary(_hDll);
    _hDll = NULL;
}

#else
#include <dlfcn.h>
DataSourceLoader::DataSourceLoader(const char *dllpath, const char *logfilepath)
:_hDll(NULL), _initProc(NULL), _uninitProc(NULL)
{
    if(NULL == dllpath || NULL == logfilepath)
        return;

    _hDll = dlopen(dllpath,RTLD_LAZY);
    if(NULL == _hDll)
    {
        //glog(ZQ::common::Log::L_ERROR,"LoadDataSource() can't load dataSource %s and last error code is %u",attr._strDataSource.c_str(),GetLastError());
        return;
    }
    _initProc = (EntryFunc_Initialize)dlsym(_hDll , "TCInitialize");
    _uninitProc = (EntryFunc_Uninitialize)dlsym(_hDll,"TCUninitialize");
    if(NULL == _initProc || NULL == _uninitProc)
    {
        dlclose(_hDll);
        _hDll = NULL;
        return;
    }
    try{
        if(0 != _initProc(logfilepath))
        {
            dlclose(_hDll);
            _hDll = NULL;
            return;
        }
    }catch(...)
    {
        dlclose(_hDll);
        _hDll = NULL;
        return;
    }
}

DataSourceLoader::~DataSourceLoader()
{
    if(NULL == _hDll)
        return;

    if(_uninitProc){
        try{
            _uninitProc();
        }catch(...){
        }
    }
    dlclose(_hDll);
    _hDll = NULL;
}

#endif
}}//namespace ZQTianShan::Layout
