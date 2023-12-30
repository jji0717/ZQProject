
// Wrappers around new cfgpkg

#ifndef _SHELLCFGPKG_H_
#define _SHELLCFGPKG_H_

#ifdef _DEBUG
	#if defined _UNICODE || defined UNICODE
		#  define VERLIBEXT "U_d.lib"
	#else
		#  define VERLIBEXT "_d.lib"
    #endif
#else
	#if defined _UNICODE || defined UNICODE
		#  define VERLIBEXT "U.lib"
	#else
		#  define VERLIBEXT ".lib"
	#endif
#endif


#pragma comment(lib, "ZQCfgPkg" VERLIBEXT)
#pragma comment(lib, "ZQAppshell" VERLIBEXT)
//#pragma comment(lib, "ZQSnmpManPkg" VERLIBEXT)

static char* _levelstrs[8]={("EMERG"),("ALERT"),("CRIT"),("ERROR"),("WARNING"),("NOTICE"),("INFO"),("DEBUG") };

HANDLE ShellCfgInitEx(TCHAR* appname, DWORD* dwNumValues, TCHAR* lpszProdName);

//modify by dony 20061026 change the seachange WCHAR to TCHAR style
CFGSTATUS ShellCfgGetValue(HANDLE handle, TCHAR* attr_name, BYTE* buffer,
                            DWORD* buffer_size, DWORD* attr_type);

//modify by dony 20061026 change the seachange WCHAR to TCHAR style
CFGSTATUS ShellCfgGetParentValues(HANDLE handle, TCHAR* attr_name, BYTE* buffer, 
                                DWORD* buffer_size, DWORD* attr_type);

//modify by dony 20061026 change the seachange WCHAR to TCHAR style
CFGSTATUS ShellCfgSetValue(HANDLE handle, TCHAR* attr_name, BYTE* attr_buff,
      DWORD attr_buff_len, DWORD attr_type);

CFGSTATUS ShellCfg_Term(HANDLE hCfg);

const char* getVerbosityStr(int level);

#endif