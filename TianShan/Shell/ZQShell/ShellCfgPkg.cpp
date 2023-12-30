#include <windows.h>
#include <tchar.h>
#include "zqcfgpkg.h"
#include "ShellCfgPkg.h"

HANDLE ShellCfgInitEx(TCHAR* appname, DWORD* pdwNumValues, TCHAR* lpszProdName)
{
    HANDLE hRet;
    
    hRet = CFG_INITEx(appname, pdwNumValues, lpszProdName);
    return hRet;
}

//modify by dony 20061026 change the seachange WCHAR to TCHAR style
CFGSTATUS ShellCfgGetValue(HANDLE handle, TCHAR* attr_name, BYTE* buffer,
                            DWORD* buffer_size, DWORD* attr_type)
{
    CFGSTATUS cfgRet = CFG_SUCCESS;
    cfgRet = CFG_GET_VALUE(handle, attr_name, buffer, buffer_size, attr_type);
    return cfgRet;
}

//modify by dony 20061026 change the seachange WCHAR to TCHAR style
CFGSTATUS ShellCfgGetParentValues(HANDLE handle, TCHAR* attr_name, BYTE* buffer, 
                                DWORD* buffer_size, DWORD* attr_type)
{
    CFGSTATUS cfgRet = CFG_SUCCESS;

    cfgRet = CFG_GET_PARENT_VALUES(handle, attr_name, buffer, buffer_size, attr_type);

     return cfgRet;

}

//modify by dony 20061026 change the seachange WCHAR to TCHAR style
CFGSTATUS ShellCfgSetValue(HANDLE handle, TCHAR* attr_name, BYTE* attr_buff,
      DWORD attr_buff_len, DWORD attr_type)
{
    CFGSTATUS cfgRet = CFG_SUCCESS;


    cfgRet = CFG_SET_VALUE(handle, attr_name, attr_buff, attr_buff_len, attr_type);

     return cfgRet;

}


CFGSTATUS ShellCfg_Term(HANDLE hCfg)
{
	CFGSTATUS cfgRet = CFG_SUCCESS;

	cfgRet  =CFG_TERM(hCfg);
    return cfgRet;
}


const char* getVerbosityStr(int level)
{
	return _levelstrs[level % (sizeof(_levelstrs)/sizeof(char*))];
}
