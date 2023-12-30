/*
** Copyright (c) 2006  by
** zq Technology Inc., West Concord, Mass.
** All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
** laws of the United States.
** 
** The software contained  on  this media is proprietary to and embodies the
** confidential technology of zq  Technology  Inc.   Possession, use,
** duplication or dissemination of the software and media is authorized only
** pursuant to a valid written license from zq Technology Inc.
** 
** This software is furnished under a  license  and  may  be used and copied
** only in accordance with the terms of  such license and with the inclusion
** of the above copyright notice.  This software or any other copies thereof
** may not be provided or otherwise made available to  any other person.  No
** title to and ownership of the software is hereby transferred.
** 
** The information in this software is subject to change without  notice and
** should not be construed as a commitment by zq Technology Inc.
** 
** zq  assumes  no  responsibility  for the use or reliability of its
** software on equipment which is not supplied by zq.
** 
** RESTRICTED RIGHTS  LEGEND  Use,  duplication,  or  disclosure by the U.S.
** Government is subject  to  restrictions  as  set  forth  in  Subparagraph
** (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
*/
/*
**
** title:       zqcfgpkg.cpp
**
** version:     T1.0
**
** facility:    zqCfgPkg DLL
**
** abstract:    This is the subroutine package used by applications to access
**        the zq product configuration information in the registry.
**
** Revision History: 10/21/2006  created  DYH
** ------------------------
**
**   Rev       Date           Who               Description
**  ------ ---------------- ------  -----------------------------------
**									Removed queue, and removed a lot of code.
**                                  You know, the use of __try/__finally here is
**                                  really another way of using ``goto''
**                                  statements.  Those should probably be re-
**                                  factored out someday on top of what I did
**                                  today.
**									Removed coupling between sessions and thread IDs.
**									SPR #995 Expand environment variables in CFG_GET_PARENT_VALUES
**									Added CFG_GET_MGMT_PORT.
**									Made generic for all zq products, Unicode.
**                                  Part of ITV work, but usable for all future products.
**								    create key if it doesn't exist in INIT and SUBKEY
**									Added CFG_FREE() routine for MFC based MANPKG
**								    Removed excessive NT Eventlogging (CfgGetValue failures)
**								    Removed TC special case from CFG_INIT
**									Bullet-proofing tasks
**						
*/

/*
 * This next #definition stuff will get removed when an ASCII interface
 * is developed
 */

#ifndef UNICODE
//#error No interface to CfgPkg defined for ASCII.  You must build with UNICODE. // modidy by dony 20061021
#endif
////////////////////////////////////////////////

//#define BLDCFGPKG   // temporary building

/* System header files */
#include <windows.h>
#include <commdlg.h>
#include <dlgs.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys\stat.h>
#include <tchar.h>
#include <winnt.h>

/* Non-system header files */
#include "zqcfgpkg.h"

/* Defines */
#define REGRETRY                5 // Must be >= 3

/*
 * Basic paths under HKEY_LOCAL_MACHINE for CDCI software and topology
 * information, as well as system software configuration.
 */
// modify by dony 20061021 


static LPCTSTR StreamPath       = _T("SOFTWARE\\ZQ Interactive\\CDCI\\StreamTopology");
static LPCTSTR MgmtPortsPath    = _T("SOFTWARE\\ZQ Interactive\\Management\\CurrentVersion\\Services");
static LPCTSTR SysSvcsPath      = _T("SYSTEM\\CurrentControlSet\\Services");

 
/* Internal routines and defines */
#define CFG_OPEN                1     /* Open and close used with CfgGetValue */
#define CFG_CLOSE               2
#define get_active_key(buf, ptr) \
_stprintf((TCHAR*)buf, _T("%s\\Services\\%s"), (CFG_INSTANCE*)ptr->mainkey, (CFG_INSTANCE*)ptr->subkey)



/* Data structure definitions
 *
 * A CFG_INSTANCE structure is allocated at cfg_init time and used to hold
 * information about the state of the thread(s) calling the cfgpkg.
 */
struct CFG_INSTANCE
{
    unsigned short      size;
    unsigned short      type;
    TCHAR               app_name[MAXOBJNAMELEN+1];
    TCHAR               subkey[MAXSUBKEYLEN+1];
    TCHAR               mainkey[MAXMAINKEYLEN+1];
};
typedef CFG_INSTANCE *PCFG_INSTANCE;

/* Forward function prototypes */

CFGSTATUS ConnectToRegistry(TCHAR *computer, HKEY *hremkey);
CFGSTATUS CfgGetValue(
    HKEY                htopkey,            /* handle to top-level key */
    TCHAR*              pkeypath,           /* path to key of interest */
    HKEY*               phtargkey,          /* hnd to key of interest */
    TCHAR*              pvalname,           /* Name of val to fetch */
    void*               pvalue,             /* Ptr to value storage */
    DWORD*              pvalsiz,            /* Ptr to value size var */
    DWORD*              pvaltype,           /* Ptr to value type var */
    unsigned short      bflags);            /* Key open/close flag */

VOID LogNTEvent(DWORD eventID, WORD strcnt, TCHAR **strarray);
LONG ExcFilter(LPEXCEPTION_POINTERS lpEP, TCHAR *caller);

/*
 * CFG_INIT positions the caller at the root of the app's configuration
 * parameters, validating that the app_name has a key present under
 * ..\zq\CDCI\CurrentVersion\Services. It returns a handle which must
 * be used in subsequent calls to get and set values. It allocates memory for
 * a cfg_instance structure and links it into the list after making sure the
 * calling thread hasn't already called cfg_init.
 *
 * v3_0  Make function idempotent.
 */
HANDLE
CFG_INIT(
    TCHAR           *app_name,
    DWORD           *num_values,
	BOOL            bServicesMode)
{
    return CFG_INITEx(app_name, num_values, _T("CDCI"),bServicesMode);
}

/*
 * CFG_INIT positions the caller at the root of the app's configuration
 * parameters, validating that the app_name has a key present under
 * ..\zq\CDCI\CurrentVersion\Services. It returns a handle which must
 * be used in subsequent calls to get and set values. It allocates memory for
 * a cfg_instance structure and links it into the list after making sure the
 * calling thread hasn't already called cfg_init.
 *
 * v3_5  Make function idempotent.
 //m_hCfg = CFG_INITEx((TCHAR*)m_sServiceName,&dwRegNum,(TCHAR*)m_sProductName);	
 */
HANDLE CFG_INITEx(
    TCHAR           *app_name,
    DWORD           *num_values,
    TCHAR           *mainkey,
	BOOL            bServicesMode)
{
    TCHAR           pathbuff[MAX_PATH];
    HKEY            hkey;
    PCFG_INSTANCE   pCfgInstance = NULL;
    LONG            status;
    DWORD           dwDisposition;

    try
    {
        // Read Application service key from registry.
		if ( bServicesMode)
		{
			_stprintf(pathbuff, _T("SOFTWARE\\ZQ Interactive\\%s\\CurrentVersion\\Services\\%s"), mainkey, app_name);
		}
		else
		{
			_stprintf(pathbuff, _T("SOFTWARE\\ZQ Interactive\\%s"),app_name);
		}

	//int iLen = lstrlen(pathbuff);

        
        // Use RegCreateKeyEx to open the key so the key will be created if it
        // does not exist.
        status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                pathbuff,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkey,
                                &dwDisposition);
        if (ERROR_SUCCESS != status)
        {
            TCHAR szErr1[MAX_PATH+64];
            TCHAR szErr2[32];
            TCHAR *pszErrStr[3] = {_T("CFG_INIT"), szErr1, szErr2};
            _stprintf(pszErrStr[1], _T("RegOpenKeyEx(HKEY_LOCAL_MACHINE, %s, ...)"), pathbuff);
            _stprintf(pszErrStr[2], _T("%d"), status);
            LogNTEvent(CFG_FAILURE, 3, pszErrStr);
            SetLastError(status);
            
        }

        // The app is there - find out how many values are in its key.
        status = RegQueryInfoKey(hkey,
                                 0,         // no lpszClass
                                 0,         // no lpcchClass
                                 0,         // lpdwReserved
                                 0,         // lpcSubKeys
                                 0,         // lpcchMaxSubkey
                                 0,         // lpcchMaxClass
                                 num_values,// lpcValues
                                 0,         // lpcchMaxValueName
                                 0,         // lpcbMaxValueData
                                 0,         // lpcbSecurityDescriptor
                                 0);        // lpftLastWriteTime
        RegCloseKey(hkey);

        // Quit if failed to read Service registry key.
        if (ERROR_SUCCESS != status)
        {
            TCHAR szErr1[MAX_PATH+64];
            TCHAR szErr2[32];
            TCHAR *pszErrStr[3] = {_T("CFG_INIT"), szErr1, szErr2};
            _stprintf(pszErrStr[1], _T("RegQueryInfoKey(%s, ...)"), pathbuff);
            _stprintf(pszErrStr[2], _T("%d"), status);
            LogNTEvent(CFG_FAILURE, 3, pszErrStr);
            SetLastError(status);
            
        }

		// Create configuration context block.
		pCfgInstance = new CFG_INSTANCE;
		if (NULL != pCfgInstance)
		{
			pCfgInstance->size = sizeof(CFG_INSTANCE);
			pCfgInstance->type = 0;


			if ( bServicesMode)
			{
				_stprintf(pCfgInstance->mainkey, _T("SOFTWARE\\ZQ Interactive\\%s\\CurrentVersion"), mainkey);
			}
			else
			{
				memset(pCfgInstance->mainkey,0,sizeof(pCfgInstance->mainkey));
			}
        
			//
			// The app_name becomes the subkey under ..\Services for this app.
			// The subkey field actually holds the subkey substring that 
			// determines which key in the registry is to be manipulated in
			// subsequent calls. It is initialized to be the same as the app's
			// "root" key under Services.
			//
			_tcsncpy(pCfgInstance->app_name, app_name, MAXOBJNAMELEN);
			_tcsncpy(pCfgInstance->subkey, app_name, MAXSUBKEYLEN);
		}
		else
		{
			TCHAR *pszErrStr[3] = {_T("CFG_INIT"), _T("malloc"), _T("OutOfMemory")};
			LogNTEvent(CFG_FAILURE, 3, pszErrStr);
			SetLastError(ERROR_NOT_ENOUGH_MEMORY); /* Set up failure status */
	  //      ;
		}
		
	}
	catch(...)
    {
    }

    return HANDLE( pCfgInstance );
}

/*
 * CFG_GET_PARENT_VALUES returns value data from the specified attribute in the
 * parent key of the application's current key. Typically, that will be the
 * Services key under ..\zq\CDCI\CurrentVersion.
 */
CFGSTATUS
CFG_GET_PARENT_VALUES(
    HANDLE          handle,
    TCHAR           *attr_name,
    BYTE            *buffer,
    DWORD           *buffer_size,
    DWORD           *attr_type,
	BOOL            bServicesMode)
{
    HKEY            hkey;
    PCFG_INSTANCE   pCfgInstance = PCFG_INSTANCE( handle );
    TCHAR           pathbuff[MAX_PATH];
    TCHAR           *backslash;
    DWORD           dwStatus = CFG_SYSERR;
    DWORD           dwBufSize = *buffer_size;  // save the total buffer size

    if (NULL == pCfgInstance)
    {
        return CFG_BAD_HANDLE;
    }

    try
    {
        get_active_key(pathbuff, pCfgInstance);

        backslash = _tcsrchr(pathbuff, _T('\\')); // Find a backslash
        if (NULL != backslash)
            pathbuff[backslash-pathbuff] = _T('\0');

        // Open registry key, retrieve value, close key.
        dwStatus = CfgGetValue(HKEY_LOCAL_MACHINE,
                                 pathbuff,
                                 &hkey,
                                 attr_name,
                                 buffer,
                                 buffer_size,
                                 attr_type,
                                 (CFG_OPEN | CFG_CLOSE));
        if (REG_EXPAND_SZ == *attr_type)
        {
            // Expand the environment variables for the caller.
            DWORD dwCount;
            DWORD dwSize;
            BYTE* pBuf = new BYTE[dwBufSize];
            dwSize = dwBufSize / sizeof(TCHAR); // # bytes / (bytes/char) = chars
            memset(pBuf, 0, dwBufSize);
            dwCount = ExpandEnvironmentStrings((LPCTSTR)buffer, (LPTSTR)pBuf, dwSize);
            if (0 != dwCount)
            {
                memcpy(buffer, pBuf, dwBufSize);
            }
            else //SPR 2356
            {
                dwStatus = CFG_FAIL;
                *buffer_size = 0;
            }
            if (dwCount > dwSize)
            {
                dwCount = dwSize;
            }
            //SPR 1812 fixed by Anand Bosco
            dwSize = dwCount * sizeof(TCHAR);
            *buffer_size = dwSize;
            //SPR 1812 fixed by Anand Bosco

            delete pBuf;
        }
    }
	catch(...)
    {
        dwStatus = CFG_SYSERR;
    }

    return CFGSTATUS( dwStatus );
}


/*
 * CFG_GET_VALUE retrieves the data value for the specified attribute.
 //CFGSTATUS cStatus = CFG_GET_VALUE(hCfg, (TCHAR*)_T("RefreshInterval"), (BYTE*)&dwValue, &dwSize, &dwType);
 */
CFGSTATUS CFG_GET_VALUE(
    HANDLE          handle,
    TCHAR           *attr_name,
    BYTE            *buffer,
    DWORD           *buffer_size,
    DWORD           *attr_type,
	BOOL            bServicesMode)
{
    HKEY            hkey;
    PCFG_INSTANCE   pCfgInstance = PCFG_INSTANCE( handle );
    TCHAR           pathbuff[MAX_PATH];
    DWORD           dwStatus = CFG_SYSERR;
    DWORD           dwBufSize = *buffer_size;  // save the total buffer size

    if (NULL == pCfgInstance)
    {
        return CFG_BAD_HANDLE;
    }
    try
    {
		if ( bServicesMode)
		{
			get_active_key(pathbuff, pCfgInstance);
		}
		else
		{
			_stprintf(pathbuff, _T("SOFTWARE\\ZQ Interactive\\%s"),pCfgInstance->subkey);
		}

        // Open registry key, retrieve value, close key.
        dwStatus = CfgGetValue(HKEY_LOCAL_MACHINE,
                                 pathbuff,
                                 &hkey,
                                 attr_name,
                                 buffer,
                                 buffer_size,
                                 attr_type,
                                 (CFG_OPEN | CFG_CLOSE));
        if (REG_EXPAND_SZ == *attr_type)
        {
            // expand the environment variables for the caller
            DWORD dwCount;
            DWORD dwSize;
            BYTE* pBuf = new BYTE[dwBufSize];
            dwSize = dwBufSize / sizeof(TCHAR); // # bytes / (bytes/char) = chars
            memset(pBuf, 0, dwBufSize);
            if (0 != (dwCount = ExpandEnvironmentStrings((LPCTSTR)buffer, (LPTSTR)pBuf, dwSize)) )
            {
                memcpy(buffer, pBuf, dwBufSize);
            }
            else //SPR 2356
            {
                dwStatus = CFG_FAIL;
                *buffer_size = 0;
            }
            if(dwCount > dwSize)
            {
                dwCount = dwSize;
            }

            //SPR 1812 fixed by Anand Bosco
            dwSize = dwCount * sizeof(TCHAR);
            *buffer_size = dwSize;
            //SPR 1812 fixed by Anand Bosco

            delete pBuf;
        }
    }
	catch(...)
    {
        dwStatus = CFG_SYSERR;
    }

    return CFGSTATUS( dwStatus );
}

/*
 * CFG_SET_VALUE sets the data value for the specified attribute, creating the
 * attribute if it does not exist.
 */
CFGSTATUS
CFG_SET_VALUE(
    HANDLE          handle,
    TCHAR           *attr_name,
    BYTE            *attr_buff,
    DWORD           attr_buff_len,
    DWORD           attr_type,
	BOOL            bServicesMode)
{
    HKEY            hkey;
    PCFG_INSTANCE   pCfgInstance = PCFG_INSTANCE( handle );
    TCHAR           pathbuff[MAX_PATH];
    LONG            status;
    DWORD           dwStatus = CFG_SYSERR;

    if (NULL == pCfgInstance)
    {
        return CFG_BAD_HANDLE;
    }

    try
    {
		if ( bServicesMode)
		{
			get_active_key(pathbuff, pCfgInstance);
		}
		else
		{
			_stprintf(pathbuff, _T("SOFTWARE\\ZQ Interactive\\%s"),pCfgInstance->subkey);
		}

        // Try to Open registry key, set value, close key.
        try
        {
            status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                  pathbuff,
                                  0,
                                  KEY_ALL_ACCESS,
                                  &hkey);
            if (ERROR_SUCCESS == status)
            {
                status = RegSetValueEx(hkey,
                                       attr_name,
                                       0,
                                       attr_type,
                                       attr_buff,
                                       attr_buff_len);
                RegCloseKey(hkey);
            }
        }
		catch(...)
		{
		}

        if (ERROR_SUCCESS == status)
        {
            dwStatus = CFG_SUCCESS;
        }
        else
        {
            SetLastError(status);
            dwStatus = CFG_SYSERR;
        } 
    }
	catch(...)
    {
        dwStatus = CFG_SYSERR;
    }

    return CFGSTATUS( dwStatus );
}

/*
 * CFG_SUBKEY makes the specified subkey the current key from which to retrieve
 * values and parent values.
 */
CFGSTATUS
CFG_SUBKEY(
    HANDLE          handle,
    TCHAR           *subkey,
    DWORD           *num_values,
	BOOL            bServicesMode)
{
    TCHAR           pathbuff[MAX_PATH];
    HKEY            hkey;
    PCFG_INSTANCE   pCfgInstance = PCFG_INSTANCE( handle );
    CFGSTATUS       result = CFG_SYSERR;
    DWORD           dwDisposition;
    LONG            status;

    if (NULL == pCfgInstance)
    {
        return CFG_BAD_HANDLE;
    }

    try
    {
        /*
         * A NULL subkey will make the app's root key the new active key. Any
         * other value is a path underneath the current subkey. Build the new key
         * string and test it out. 
         */
		if ( bServicesMode)
		{
			if (NULL == subkey)
			{
				_stprintf(pathbuff, _T("%s\\Services\\%s"),
						  pCfgInstance->mainkey,
						  pCfgInstance->app_name);
			}
			else
			{
				_stprintf(pathbuff, _T("%s\\Services\\%s\\%s"),
						  pCfgInstance->mainkey,
						  pCfgInstance->subkey, subkey);
			}
		}
		else
		{
			if (NULL == subkey)
			{
				_stprintf(pathbuff, _T("%s"),
						  pCfgInstance->subkey);
			}
			else
			{
				_stprintf(pathbuff, _T("%s\\%s"),
						  pCfgInstance->subkey,
						  subkey);
			}
		}
        /*
         * Open the key and query for the number of values. If all that works,
         * make this the new subkey.
         * Use RegCreateKeyEx to open the key so the key will be created if
         * it does not exist.
         */
        status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                pathbuff,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ,
                                NULL,
                                &hkey,
                                &dwDisposition);
        if (ERROR_SUCCESS == status)
        {
            status = RegQueryInfoKey(hkey,
                                     0,         // no lpszClass
                                     0,         // no lpcchClass
                                     0,         // lpdwReserved
                                     0,         // lpcSubKeys
                                     0,         // lpcchMaxSubkey
                                     0,         // lpcchMaxClass
                                     num_values,// lpcValues
                                     0,         // lpcchMaxValueName
                                     0,         // lpcbMaxValueData
                                     0,         // lpcbSecurityDescriptor
                                     0);        // lpftLastWriteTime
            RegCloseKey(hkey);
        
            //
            // The new validated subkey is either the original root (if subkey
            // is NULL), or the current subkey with the new subkey appended.
            //
            if (NULL == subkey)
            {
                _tcsncpy(pCfgInstance->subkey, pCfgInstance->app_name,
                         MAXSUBKEYLEN);
            }
            else
            {
                _tcscat(pCfgInstance->subkey, _T("\\"));
                _tcscat(pCfgInstance->subkey, subkey);
            }
            result = CFG_SUCCESS;
        }
        else
        {
            SetLastError(status);
            result = CFG_SYSERR;
        }
    }
	catch(...)
    {
        result = CFG_SYSERR;
    }
    
    return result;
}

CFGSTATUS CFG_SUBKEY_EXISTS(HANDLE   handle,LPCTSTR  subkey,BOOL bServicesMode)
{
    TCHAR           szSubKey[MAX_PATH];
    PCFG_INSTANCE   pCfgInstance = PCFG_INSTANCE( handle );
    CFGSTATUS       result = CFG_SYSERR;

    if (NULL == pCfgInstance)
    {
        return CFG_BAD_HANDLE;
    }

    try
    {
        //
        // A NULL subkey will make the app's root key the new active key. Any
        // other value is a path underneath the current subkey. Build the new
        // key string and test it out. 
        //
		if ( bServicesMode)
		{
			if (NULL == subkey)
			{
				_stprintf(szSubKey, _T("%s\\Services\\%s"),
						  pCfgInstance->mainkey,
						  pCfgInstance->app_name);
			}
			else
			{
				_stprintf(szSubKey, _T("%s\\Services\\%s\\%s"),
						  pCfgInstance->mainkey,
						  pCfgInstance->subkey, subkey);
			}
		}
		else
		{
			if (NULL == subkey)
			{
				_stprintf(szSubKey, _T("%s"),
						  pCfgInstance->subkey);
			}
			else
			{
				_stprintf(szSubKey, _T("%s\\%s"),
						  pCfgInstance->subkey,
						  subkey);
			}
		}
        //
        // Open the key to test for its existence.
        //
        HKEY hkey = NULL;
        LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                   szSubKey,
                                   0, // reserved (must be 0)
                                   KEY_READ,
                                   &hkey);
        if (ERROR_SUCCESS == status)
        {
            RegCloseKey(hkey);
            result = CFG_SUCCESS;
        }
        else
        {
            result = CFG_NO_SUCH_SUBKEY;
        }
    }
	catch(...)
    {
        result = CFG_SYSERR;
    }

    return result;
}

CFGSTATUS CFG_VALUE_EXISTS(HANDLE handle, LPCTSTR varName,BOOL bServicesMode )
{
	HKEY            hkey;
    PCFG_INSTANCE   pCfgInstance = PCFG_INSTANCE( handle );
    TCHAR           pathbuff[MAX_PATH];
    LONG            status;
    DWORD           dwStatus = CFG_SYSERR;
	DWORD           dwType;
	DWORD           dwSize;
	TCHAR           lpData[MAX_PATH];


	memset(pathbuff,0,sizeof(pathbuff));
	memset(lpData,0,sizeof(lpData));


    if (NULL == pCfgInstance)
    {
        return CFG_BAD_HANDLE;
    }

    try
    {
		if ( bServicesMode)
		{
			get_active_key(pathbuff, pCfgInstance);
		}
		else
		{
			_stprintf(pathbuff, _T("SOFTWARE\\ZQ Interactive\\%s"),pCfgInstance->subkey);
		}
		
		status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                  pathbuff,
                                  0,
                                  KEY_ALL_ACCESS,
                                  &hkey);
        if (ERROR_SUCCESS == status)
        {
			dwSize = MAX_PATH*(sizeof(TCHAR)); 
		    status = RegQueryValueEx(hkey,varName, NULL, &dwType, (LPBYTE)lpData, &dwSize);
			if ( status == ERROR_SUCCESS)
			{
				dwStatus = CFG_SUCCESS;
				RegCloseKey(hkey);
			}
			else
			{
				dwStatus = CFG_NO_SUCH_VARNAME;
			}
		}
	}
	catch(...)
	{
		dwStatus = CFG_NO_SUCH_VARNAME;
	}
	return CFGSTATUS( dwStatus );
}
/*
/*
 * CFG_GETMGMNTPORT returns the ManUtil/Manpkg port number for a given service.
 *
 * handle (IN) -- a hadle for the CfgSession, returned from a call to CFG_INITEx
 * port_number (OUT) -- receives the management port number of the service specified.
 *              By current convention, this is taken from 
 *              HKLM\Software\zq\Management\CurrentVersion\Services\<app_name>
 *              where <app_name> is the value supplied in CFG_INITEx.
 */
CFGSTATUS
CFG_GET_MGMT_PORT(
    HANDLE          handle, 
    DWORD*          port_number
    )
{
    PCFG_INSTANCE   pCfgInstance = PCFG_INSTANCE( handle );
    TCHAR           pathbuff[MAX_PATH];
    CFGSTATUS       result = CFG_SYSERR;
    TCHAR           buffer[MAXMAINKEYLEN];
    DWORD           buffersize;
    DWORD           attr_type;
    HKEY            hkUnusedOutputKey;

    if (NULL == pCfgInstance)
    {
        return CFG_BAD_HANDLE;
    }

    try
    {
        _stprintf(pathbuff, _T("%s\\%s"),
                  MgmtPortsPath,
                  pCfgInstance->app_name);

        buffersize = MAXMAINKEYLEN;
        
        // Open registry key, retrieve value, close key.
        result = CfgGetValue(HKEY_LOCAL_MACHINE,
                               pathbuff,
                               &hkUnusedOutputKey,
                               _T("MgmtPortNumber"),
                               buffer,
                               &buffersize,
                               &attr_type,
                               (CFG_OPEN | CFG_CLOSE));
         
        if (CFG_SUCCESS == result)
        {
            if (REG_DWORD == attr_type)
            {
                *port_number = *PDWORD( buffer );
                result = CFG_SUCCESS;
            }
            else
            {
                result = CFG_ENTRY_WRONG_TYPE;
            }
        }
    }
	catch(...)
    {
        result = CFG_SYSERR;
    }

    return result;
}

/*
 * CFG_TERM frees memory used by the handle's configuration instance. It 
 * must be called by the thread that called CFG_INIT.
 */
CFGSTATUS
CFG_TERM(
    HANDLE          handle)
{
    PCFG_INSTANCE   pCfgInstance = PCFG_INSTANCE( handle );
    CFGSTATUS       result = CFG_SYSERR;

    if (NULL == pCfgInstance)
    {
        return CFG_BAD_HANDLE;
    }

    try
    {
        delete pCfgInstance;
        pCfgInstance = NULL;

        result = CFG_SUCCESS;
    }
	catch(...)
    {
        result = CFG_SYSERR;
    }
    return result;
}

/*
 * CFG_GET_CDCI_VERSION returns the major and minor version strings stored in
 * the CDCI key on the local node.
 */
CFGSTATUS
CFG_GET_CDCI_VERSIONEx(
    HANDLE          handle,
    BYTE            *major,
    DWORD           *major_len,
    BYTE            *minor,
    DWORD           *minor_len
    )
{
    HKEY            hkey;
    PCFG_INSTANCE   pCfgInstance = PCFG_INSTANCE( handle );
    CFGSTATUS       result = CFG_SYSERR;

    if (NULL == pCfgInstance)
    {
        return CFG_BAD_HANDLE;
    }

    //
    // Open the ..\CDCI\CurrentVersion key and read out the major and minor
    // version numbers.
    //
    try
    {
        result = CfgGetValue(HKEY_LOCAL_MACHINE,
                               pCfgInstance->mainkey,
                               &hkey,
                               _T("MajorVersion"),
                               major,
                               major_len,
                               NULL,
                               CFG_OPEN);
        if (CFG_SUCCESS == result)
        {
            result = CfgGetValue(HKEY_LOCAL_MACHINE,
                                   pCfgInstance->mainkey,
                                   &hkey,
                                   _T("MinorVersion"),
                                   minor,
                                   minor_len,
                                   NULL,
                                   CFG_CLOSE);
        }
    }
	catch(...)
    {
        result = CFG_SYSERR;
    }
    return result;
}

/*
 * CFG_GET_CDCI_VERSION returns the major and minor version strings stored in
 * the CDCI key on the local node.
 */
CFGSTATUS
CFG_GET_CDCI_VERSION(
    BYTE            *major,
    DWORD           *major_len,
    BYTE            *minor,
    DWORD           *minor_len
    )
{

	TCHAR           CurVerPath[] = _T("SOFTWARE\\ZQ Interactive\\CDCI\\CurrentVersion");

    HKEY            hkey;
    CFGSTATUS       result = CFG_SYSERR;

    //
    // Open the ..\CDCI\CurrentVersion key and read out the major and minor
    // version numbers.
    //
    try
    {
        result = CfgGetValue(HKEY_LOCAL_MACHINE,
                               CurVerPath,
                               &hkey,
                               _T("MajorVersion"),
                               major,
                               major_len,
                               NULL,
                               CFG_OPEN);
        if (CFG_SUCCESS == result)
        {
            result = CfgGetValue(HKEY_LOCAL_MACHINE,
                                   CurVerPath,
                                   &hkey,
                                   _T("MinorVersion"),
                                   minor,
                                   minor_len,
                                   NULL,
                                   CFG_CLOSE);
        }
    }
	catch(...)
    {
        result = CFG_SYSERR;
    }
    return result;
}

/*
 * CFG_GET_STREAM_TOPOLOGY returns data into an array of vstream_obj structures.
 * Depending on the direction parameter, the local system's parent or child
 * objects in the videostream hierarchy are returned. 
 */
CFGSTATUS
CFG_GET_STREAM_TOPOLOGY(
    TCHAR           *refpoint,
    SHORT           direction,
    struct vstream_obj
                    vstreamobjs[],
    DWORD           *obj_cnt
    )
{
    HKEY            hkey, hremkey;
    WORD            w;
    DWORD           systype, dwStatus, dwRetStatus;
    DWORD           name_len;
    DWORD           valsize;
    TCHAR           pathbuff[MAX_PATH];

	TCHAR           CurVerPath[] = _T("SOFTWARE\\ZQ Interactive\\CDCI\\CurrentVersion");

    _try {
    
        hkey = hremkey = NULL;   /* Initialize for cleanup's check */
        _try {
            // Quit if invalid direction code
            if((direction != CFG_UPSTREAM) && (direction != CFG_DOWNSTREAM)) {
                  dwRetStatus = CFG_BADPARAM;
                _leave;
            }

            // If Reference Point specified, then connect to remote registry
            //         Quit if connection failure
            if (refpoint != NULL) {
                if((dwStatus = ConnectToRegistry(refpoint, &hremkey)) != CFG_SUCCESS) _leave;
            } else
                hremkey = HKEY_LOCAL_MACHINE;

            _stprintf(pathbuff, _T("%s\\Services"), CurVerPath);
            valsize = sizeof(int);
            if((dwRetStatus = CfgGetValue(hremkey,
                            pathbuff,
                            &hkey,
                            _T("SystemType"),
                            &systype,
                            &valsize,
                            NULL,
                            (CFG_OPEN | CFG_CLOSE))) != CFG_SUCCESS) _leave;

               // Now open the registry key for the upstream or downstream node list.
            _stprintf(pathbuff, _T("%s\\%s"), StreamPath, direction == CFG_UPSTREAM ? _T("Upstream") : _T("Downstream"));
            if ((dwStatus = (DWORD)RegOpenKeyEx(hremkey, // Top level pre-defined key
                  pathbuff,
                  0,                /* reserved */
                  KEY_READ,         /* access */
                  &hkey)) == ERROR_SUCCESS) {

                   // Retrieve information about as many subkeys as we can.
                // 
                for (w = 0; w < *obj_cnt; w++) {
                    name_len = MAXOBJNAMELEN;
                    dwStatus = (DWORD)RegEnumKeyEx(hkey,
                        w,
                        vstreamobjs[w].obj_name,/* Place name in vstreamobj */
                        &name_len,        /* Length of obj_name buffer */
                        NULL,                   /* lpdwReserved */
                        NULL,                   /* lpszClass */
                        NULL,                   /* lpcchClass */
                        (FILETIME*)&pathbuff);   /* lpftLastWrite */

                    if (dwStatus == ERROR_SUCCESS) {
                        if((systype == CFG_INSERTION_SYSTEM) && (direction == CFG_DOWNSTREAM))
                            vstreamobjs[w].obj_type = VSTREAM_INSERT_DEV;
                        else
                            vstreamobjs[w].obj_type = VSTREAM_SYSTEM;
                    } else {
                        if (dwStatus == ERROR_NO_MORE_ITEMS) {
                            *obj_cnt = w;
                            break;
                        } else {
                            SetLastError(dwStatus);
                            dwRetStatus = CFG_SYSERR;
                            _leave;
                        }
                       }
                }
                // At this point we know that (1) there are no more items,
                // or (2) that there may be more items.  
                // The function specification and the function usage does not care about this ambiquous condition....
                // If circumstances change, then the design should be fixed.
                dwRetStatus = CFG_SUCCESS;
           } else {
                   SetLastError(dwStatus);
                dwRetStatus = CFG_SYSERR;
           }

        } _finally {
            if(hkey != NULL) RegCloseKey(hkey);
            if(refpoint != NULL) RegCloseKey(hremkey);
        }
    } _except (ExcFilter(GetExceptionInformation(), _T("CFG_"))) {
        dwStatus = CFG_SYSERR;
    }
    return (CFGSTATUS)dwRetStatus;
}

/*
 * CFG_GET_STREAM_OBJ_INFO returns the parameters of one of the videostream
 * objects returned by CFG_GET_STREAM_TOPOLOGY. Currently, this code only 
 * guesses at what those parameters will be, so don't take it too seriously.
 *  -DH 2/15/94
 */
CFGSTATUS
CFG_GET_STREAM_OBJ_INFO(
    struct vstream_obj_info
                    *stream_info)
{
    int             i;
    DWORD           valsize, status = CFG_SYSERR;
    HKEY            hremkey, hobjkey;
    TCHAR           pathbuff[MAX_PATH];
    TCHAR           devarray[MAXNETADAPTERS*MAXOBJNAMELEN];
    TCHAR           *devptr;   

	TCHAR           CurVerPath[] = _T("Software\\ZQ Interactive\\CDCI\\CurrentVersion");


    _try {
        _try {
/*
 * Start by examining the type of object we're interested in. For systems, 
 * we'll open the (remote) registry to access to the configuration info.
 */
            switch (stream_info->obj.obj_type) {
                case (VSTREAM_SYSTEM):
                  if((status = ConnectToRegistry(stream_info->obj.obj_name, &hremkey)) != CFG_SUCCESS)
                      _leave;

                    /*
                     * Find out what kind of system it is...
                     */
                  _stprintf(pathbuff, _T("%s\\Services"), CurVerPath);
                  valsize = sizeof(DWORD);
                  if((status = CfgGetValue(hremkey,
                                pathbuff,
                                &hobjkey,
                                _T("SystemType"),
                                &stream_info->sys_obj.systype,
                                &valsize,
                                NULL,
                                (CFG_OPEN | CFG_CLOSE))) != CFG_SUCCESS)
                        _leave;

                  /*
                   * Now open the key on the remote registry. Note that eventually there
                   * will probably be several keys to open to extract all the values we
                   * need. For now, just get TCP/IP address.
                   */
                  _stprintf(pathbuff, _T("%s\\Tcpip\\Linkage"), SysSvcsPath);
                  valsize = sizeof(devarray);
                  if((status = CfgGetValue(hremkey,
                                pathbuff,
                                &hobjkey,
                                _T("Bind"),
                                devarray,
                                &valsize,
                                NULL,
                                (CFG_OPEN | CFG_CLOSE))) != CFG_SUCCESS)
                        _leave;
                /* 
                 * Parse through the binding information for device. The string
                 * returned for the binding is of the form
                 * "\device\streams\<devname>." Devname can then be found under
                 * ..CurrentControlSet\Services.
                 *
                 * Begin by storing the device name information in the adapter_info
                 * structures passed by the caller.
                 */

                  devptr = _tcsrchr(devarray, _T('\\'));
                  for (i = 0;
                      (i < stream_info->sys_obj.no_netadapt) && (devptr != NULL);
                       i++) {
                    _tcsncpy(stream_info->sys_obj.adapter[i].dev_name, 
                              ++devptr, MAXDEVNAMELEN);
                    devptr = _tcsrchr(devptr + (_tcslen(devptr)+1), _T('\\'));
                    /*
                     * As we get the device names of the adapters, go look for them
                     * as keys under CurrentControlSet\Services. The Parameters subkey
                     * contains the media type for the adapter.
                     */
                    _stprintf(pathbuff, _T("%s\\%s\\Parameters"), SysSvcsPath, stream_info->sys_obj.adapter[i].dev_name);
                    valsize = sizeof(DWORD);
                    if((status = CfgGetValue(hremkey,
                                pathbuff,
                                &hobjkey,
                                _T("MediaType"),
                                &stream_info->sys_obj.adapter[i].net_type,
                                &valsize,
                                NULL,
                                (CFG_OPEN | CFG_CLOSE))) != CFG_SUCCESS)
                          _leave;
                  /*
                   * Now go for the IP address in the Tcpip subkey.
                   */
                    _tcscat(pathbuff, _T("\\Tcpip"));
                    valsize = MAXIPADDRLEN;
                    if((status = CfgGetValue(hremkey,
                                pathbuff,
                                &hobjkey,
                                _T("IPAddress"),
                                stream_info->sys_obj.adapter[i].inet_addr,
                                &valsize,
                                NULL,
                                (CFG_OPEN | CFG_CLOSE))) != CFG_SUCCESS)
                          _leave;

                  }                    /* End for loop through net adapters*/
                  stream_info->sys_obj.no_netadapt = i;    // Return the number found

                  break;
              /*
               * For insertion devices, get the system name that the device is on from
               * the dev_obj structure and open the (remote) registry. If the system
               * name is specified as NULL, use the local registry.
               */
                case (VSTREAM_INSERT_DEV):
    
                  if(_tcslen(stream_info->dev_obj.IS_name) != 0) {
                    if((status = ConnectToRegistry(stream_info->dev_obj.IS_name, &hremkey)) != CFG_SUCCESS)
                      _leave;
                  }
                  else
                    hremkey = HKEY_LOCAL_MACHINE;
                /*
                 * Insertion devices are downstream of any registry. Form the registry
                 * path to the insertion device and read the first value.
                 */
                  _stprintf(pathbuff, _T("%s\\Downstream\\%s"),
                          StreamPath, stream_info->obj.obj_name);
                  valsize = sizeof(DWORD);
                  if((status = CfgGetValue(hremkey,
                                pathbuff,
                                &hobjkey,
                                _T("DTMFDelay"),
                                &stream_info->dev_obj.DTMF_delay,
                                &valsize,
                                NULL,
                                CFG_OPEN)) != CFG_SUCCESS)
                    _leave;
                /*
                 * Continue... get active window value.
                 */
                  valsize = sizeof(DWORD);
                  if((status = CfgGetValue(hremkey,
                                pathbuff,
                                &hobjkey,
                                _T("ActiveWindow"),
                                &stream_info->dev_obj.act_wind,
                                &valsize,
                                NULL,
                                0)) != CFG_SUCCESS)
                    _leave;
                 /*
                  * Finally, get the physical device name and close the key.
                  */
                  valsize = MAXDEVNAMELEN;
                  if((status = CfgGetValue(hremkey,
                                pathbuff,
                                &hobjkey,
                                _T("HardwarePort"),
                                stream_info->dev_obj.dev_name,
                                &valsize,
                                NULL,
                                CFG_CLOSE)) != CFG_SUCCESS)
                        _leave;
                  break;

                default:
                  status = CFG_BAD_OBJTYPE;
                  _leave;
              }

        } _finally {
          if(hremkey != HKEY_LOCAL_MACHINE) RegCloseKey(hremkey);
        }
    } _except (ExcFilter(GetExceptionInformation(), _T("CFG_"))) {
        status = CFG_SYSERR;
    }

    return (CFGSTATUS)status;
}

/*
 * Utility Routines
 *
 *  CfgGetValue - get a value from a key, optionally opening and/or
 *                closing the key
 */

/*
 *  CfgGetValue(htopkey, pkeypath, phtargkey, pvalname, pvalue, pvalsiz
 *              bflags)
 *
 *  Description:
 *    Retrieve a value from a key, optionally opening it first. Optionally
 *    close the key.
 * 
 *  Input:
 *    htopkey   - handle to the top-level key for open
 *    pkeypath  - pointer to the registry path of the key to open
 *    phtargkey - pointer to a handle for the key, if not opening
 *    pvalname  - pointer to a string containing the value name
 *    pvalue    - pointer to storage to contain the value
 *    pvalsiz   - pointer to an integer to contain the size of the value
 *    bflags    - controls whether the key is opened on entry and/or 
 *                closed on exit
 *  Output:
 *    phtargkey - returns a handle to the opened key, if not closing
 *    pvalue    - pointer to the value retrieved
 *    pvalsiz   - pointer to an integer indicating the size of the value
 *
 *  Returns:
 *    CFG_SUCCESS - all went well
 *    CFG_SYSERR  - a registry function failed
 *
 *  Side Effects:
 *    Calls SetLastError for any failures detected from NT. Closes key 
 *    on error regardless of flag value and sets the key value to NULL.
 *
 *  Assumptions:
 */
static CFGSTATUS
CfgGetValue(
    HKEY            htopkey, 
    TCHAR*          pkeypath, 
    HKEY*           phtargkey,
    TCHAR*          pvalname, 
    void*           pvalue, 
    DWORD*          pvalsiz, 
    DWORD*          pvaltype,
    unsigned short  bflags)
{
    CFGSTATUS       result = CFG_SYSERR;

    try
    {
        LONG status = ERROR_SUCCESS;

        try
        {
            // Conditionally try to open registry key.
            if (bflags & CFG_OPEN) 
            {
                status = RegOpenKeyEx(htopkey,
                                      pkeypath,
                                      0,
                                      KEY_READ,
                                      phtargkey);

                if (ERROR_SUCCESS != status)
                {
                    ;
                }
            }

            status = RegQueryValueEx(*phtargkey,
                                     pvalname,          // Value name
                                     NULL,
                                     pvaltype,          // Value type
                                     PBYTE( pvalue ),   // Actual value
                                     pvalsiz);
            if (ERROR_SUCCESS != status)
            {
                ;
            }

            // Conditionally close registry key.
            if (bflags & CFG_CLOSE)
            {
                RegCloseKey(*phtargkey);
            }

            result = CFG_SUCCESS;
        }
		catch(...)
		{
		}
		
        if (CFG_SUCCESS != result)
        {
            RegCloseKey(*phtargkey);
            *phtargkey = NULL;
            SetLastError(status);
            if (ERROR_FILE_NOT_FOUND == status)
                result = CFG_ENTRY_NOT_FOUND;
            else
                result = CFG_SYSERR;
        }
		if ( status != ERROR_SUCCESS)
		{
			result = CFG_SYSERR;
		}
    }
	catch(...)
    {
        result = CFG_SYSERR;
    }

  return result;
}

/*
 ******************************************************************************
 *  ConnectToRegistry()
 *
 *  Description:
 *         Connect to the remote registry on the system specified by the argument.
 *        Retry up to REGRETRY times if the RPC server on the remote system is too
 *        busy.
 *
 *  Input:
 *        computer    - name of the remote system to connect to.
 *
 *  Output:
 *
 *  Returns:
 *
 *  Side Effects:
 *
 *  Assumptions:
 ******************************************************************************
 */
CFGSTATUS
ConnectToRegistry(
    TCHAR           *computer,
    HKEY            *hremkey
    )
{
    CFGSTATUS       result = CFG_SYSERR;

    try
    {
        LONG status = ERROR_SUCCESS;

        try
        {
            TCHAR computername[MAX_COMPUTERNAME_LENGTH + 3];

            _stprintf(computername, _T("\\\\%s"), computer);

            for (int i = 1; i < REGRETRY; i++)
            {
                status = RegConnectRegistry(computername,
                                            HKEY_LOCAL_MACHINE,
                                            hremkey);
                if (ERROR_SUCCESS == status)
                    break;
                else 
                    if (RPC_S_SERVER_TOO_BUSY == status)
                         Sleep(i * 1000);
                    else 
                        ;
            }
        }
		catch(...)
		{
		}

        if (ERROR_SUCCESS == status)
            result = CFG_SUCCESS;
        else
        {
            result = CFG_SYSERR;
            SetLastError(status);
        }
    }
	catch(...)
    {
        result = CFG_SYSERR;
    }

   return result;
}


// ExcFilter -- Exception Filter
// Returns - EXCEPTION_EXECUTE_HANDLER
/* 
 * An exception filter, to print exception stuff
 */
static LONG
ExcFilter(
    LPEXCEPTION_POINTERS
                    lpEP,
    TCHAR           *caller
    )
{
    TCHAR detail[128], errnum[20], szAddress[20];
    DWORD Code = lpEP->ExceptionRecord->ExceptionCode;
    PVOID Address = lpEP->ExceptionRecord->ExceptionAddress;
    
    TCHAR * ArrayErrorStr[4] = { errnum,
                                 szAddress,
                                 detail,
                                 caller };

    _stprintf(errnum, _T("%x"), Code);
    _stprintf(szAddress, _T("%x"), Address);

    if (EXCEPTION_ACCESS_VIOLATION == Code)
    {
        _stprintf(detail, _T(": attempt to %s data at address %x"),
                  lpEP->ExceptionRecord->ExceptionInformation[0] ? _T("read") : _T("write"),
                  lpEP->ExceptionRecord->ExceptionInformation[1]);
    }
    else
    {
        detail[0] = _T('\n');
    }

    LogNTEvent(CFG_EXCEPTION, 4, ArrayErrorStr);
    SetLastError(ERROR_GEN_FAILURE);

    return EXCEPTION_EXECUTE_HANDLER;
}


// LogNTEvent -- Log NT event to local application event sink
//
static VOID
LogNTEvent(
    DWORD           eventID,
    WORD            strcnt,
    TCHAR           **strarray
    )
{
    HANDLE          hEventSource;
    WORD            type;

    // Log NT events locally
    hEventSource = RegisterEventSource(NULL, _T("CFGPKG"));
    if (hEventSource != NULL)
    {
        switch (((eventID >> 30) & 0x3))
        {
        case 1:
            type = EVENTLOG_INFORMATION_TYPE;
            break;
        case 2:
            type = EVENTLOG_WARNING_TYPE;
            break;
        default: 
            type = EVENTLOG_ERROR_TYPE;
        };

        ReportEvent(hEventSource,       // handle of event source
                    type,               // event type
                    0,                  // event category
                    eventID,            // event ID
                    NULL,               // current user's SID
                    strcnt,             // strings in lpszStrings
                    0,                  // no bytes of raw data
                    (LPCTSTR*)strarray, // array of error strings
                    NULL);              // no raw data

        DeregisterEventSource(hEventSource);
    }
}


/*
*  CFG_FREE
*
*    Routine used to free memory, just calls C RTL routine free().
*    Required to provide MFC based MANPKG a way to deallocate
*    memory allocated by a service's complex variable routine.
*/
void
CFG_FREE(
    TCHAR           *buf
    )
{
    free( buf );
}
