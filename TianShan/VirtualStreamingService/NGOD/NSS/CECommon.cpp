
#include "CECommon.h"
#include "wininet.h"


#define Common			"Common"
#define MOLOG					glog

///////////////////////////////////////////////////////////////////////////////
// Get error message from the given error code
//
std::string getErrMsg(DWORD dwErrCode)
{
    LPVOID lpMsgBuf;
    FormatMessageA(     FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwErrCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR) &lpMsgBuf,
            0,
            NULL 
            );
    if (lpMsgBuf != NULL)
    {
		std::string strErr = (LPCTSTR) lpMsgBuf;
        LocalFree( lpMsgBuf );
        return strErr;
    }
    else 
    {
		std::string strErrMsg;

        switch (dwErrCode)
        {
        default: // Not all error code can be formated
           {
                char wszErrMsg [256];
                sprintf(wszErrMsg, "Error Code = %d", dwErrCode);
                strErrMsg = wszErrMsg;
                break;
            }

        case ERROR_INTERNET_OUT_OF_HANDLES:
            strErrMsg = "ERROR_INTERNET_OUT_OF_HANDLES"; break;

        case ERROR_INTERNET_TIMEOUT:
            strErrMsg = "ERROR_INTERNET_TIMEOUT"; break;

        case ERROR_INTERNET_EXTENDED_ERROR:
            strErrMsg = "ERROR_INTERNET_EXTENDED_ERROR"; break;
            /*{
                WCHAR buf[256];
                DWORD dwErr, dwLen = 256;
                if(TRUE == InternetGetLastResponseInfo(&dwErr, buf, &dwLen))
                {
                    strErrMsg = buf;
                }
                else
                {
                    strErrMsg = L"ERROR_INTERNET_EXTENDED_ERROR"; 
                }
                break;
            }*/

        case ERROR_INTERNET_INTERNAL_ERROR:
            strErrMsg = "ERROR_INTERNET_INTERNAL_ERROR"; break;

        case ERROR_INTERNET_INVALID_URL:
            strErrMsg = "ERROR_INTERNET_INVALID_URL"; break;

        case ERROR_INTERNET_UNRECOGNIZED_SCHEME:
            strErrMsg = "ERROR_INTERNET_UNRECOGNIZED_SCHEME"; break;

        case ERROR_INTERNET_NAME_NOT_RESOLVED:
            strErrMsg = "ERROR_INTERNET_NAME_NOT_RESOLVED"; break;

        case ERROR_INTERNET_PROTOCOL_NOT_FOUND:
            strErrMsg = "ERROR_INTERNET_PROTOCOL_NOT_FOUND"; break;

        case ERROR_INTERNET_INVALID_OPTION:
            strErrMsg = "ERROR_INTERNET_INVALID_OPTION"; break;

        case ERROR_INTERNET_BAD_OPTION_LENGTH:
            strErrMsg = "ERROR_INTERNET_BAD_OPTION_LENGTH"; break;

        case ERROR_INTERNET_OPTION_NOT_SETTABLE:
            strErrMsg = "ERROR_INTERNET_OPTION_NOT_SETTABLE"; break;

        case ERROR_INTERNET_SHUTDOWN:
            strErrMsg = "ERROR_INTERNET_SHUTDOWN"; break;

        case ERROR_INTERNET_INCORRECT_USER_NAME:
            strErrMsg = "ERROR_INTERNET_INCORRECT_USER_NAME"; break;

        case ERROR_INTERNET_INCORRECT_PASSWORD:
            strErrMsg = "ERROR_INTERNET_INCORRECT_PASSWORD"; break;

        case ERROR_INTERNET_LOGIN_FAILURE:
            strErrMsg = "ERROR_INTERNET_LOGIN_FAILURE"; break;

        case ERROR_INTERNET_INVALID_OPERATION:
            strErrMsg = "ERROR_INTERNET_INVALID_OPERATION"; break;

        case ERROR_INTERNET_OPERATION_CANCELLED:
            strErrMsg = "ERROR_INTERNET_OPERATION_CANCELLED"; break;

        case ERROR_INTERNET_INCORRECT_HANDLE_TYPE:
            strErrMsg = "ERROR_INTERNET_INCORRECT_HANDLE_TYPE"; break;

        case ERROR_INTERNET_INCORRECT_HANDLE_STATE:
            strErrMsg = "ERROR_INTERNET_INCORRECT_HANDLE_STATE"; break;

        case ERROR_INTERNET_NOT_PROXY_REQUEST:
            strErrMsg = "ERROR_INTERNET_NOT_PROXY_REQUEST"; break;

        case ERROR_INTERNET_REGISTRY_VALUE_NOT_FOUND:
            strErrMsg = "ERROR_INTERNET_REGISTRY_VALUE_NOT_FOUND"; break;

        case ERROR_INTERNET_BAD_REGISTRY_PARAMETER:
            strErrMsg = "ERROR_INTERNET_BAD_REGISTRY_PARAMETER"; break;

        case ERROR_INTERNET_NO_DIRECT_ACCESS:
            strErrMsg = "ERROR_INTERNET_NO_DIRECT_ACCESS"; break;

        case ERROR_INTERNET_NO_CONTEXT:
            strErrMsg = "ERROR_INTERNET_NO_CONTEXT"; break;

        case ERROR_INTERNET_NO_CALLBACK:
            strErrMsg = "ERROR_INTERNET_NO_CALLBACK"; break;

        case ERROR_INTERNET_REQUEST_PENDING:
            strErrMsg = "ERROR_INTERNET_REQUEST_PENDING"; break;

        case ERROR_INTERNET_INCORRECT_FORMAT:
            strErrMsg = "ERROR_INTERNET_INCORRECT_FORMAT"; break;

        case ERROR_INTERNET_ITEM_NOT_FOUND:
            strErrMsg = "ERROR_INTERNET_ITEM_NOT_FOUND"; break;

        case ERROR_INTERNET_CANNOT_CONNECT:
            strErrMsg = "ERROR_INTERNET_CANNOT_CONNECT"; break;

        case ERROR_INTERNET_CONNECTION_ABORTED:
            strErrMsg = "ERROR_INTERNET_CONNECTION_ABORTED"; break;

        case ERROR_INTERNET_CONNECTION_RESET:
            strErrMsg = "ERROR_INTERNET_CONNECTION_RESET"; break;

        case ERROR_INTERNET_FORCE_RETRY:
            strErrMsg = "ERROR_INTERNET_FORCE_RETRY"; break;

        case ERROR_INTERNET_INVALID_PROXY_REQUEST:
            strErrMsg = "ERROR_INTERNET_INVALID_PROXY_REQUEST"; break;

        case ERROR_INTERNET_NEED_UI:
            strErrMsg = "ERROR_INTERNET_NEED_UI"; break;

        case ERROR_INTERNET_HANDLE_EXISTS:
            strErrMsg = "ERROR_INTERNET_HANDLE_EXISTS"; break;

        case ERROR_INTERNET_SEC_CERT_DATE_INVALID:
            strErrMsg = "ERROR_INTERNET_SEC_CERT_DATE_INVALID"; break;

        case ERROR_INTERNET_SEC_CERT_CN_INVALID:
            strErrMsg = "ERROR_INTERNET_SEC_CERT_CN_INVALID"; break;

        case ERROR_INTERNET_HTTP_TO_HTTPS_ON_REDIR:
            strErrMsg = "ERROR_INTERNET_HTTP_TO_HTTPS_ON_REDIR"; break;

        case ERROR_INTERNET_HTTPS_TO_HTTP_ON_REDIR:
            strErrMsg = "ERROR_INTERNET_HTTPS_TO_HTTP_ON_REDIR"; break;

        case ERROR_INTERNET_MIXED_SECURITY:
            strErrMsg = "ERROR_INTERNET_MIXED_SECURITY"; break;

        case ERROR_INTERNET_CHG_POST_IS_NON_SECURE:
            strErrMsg = "ERROR_INTERNET_CHG_POST_IS_NON_SECURE"; break;

        case ERROR_INTERNET_POST_IS_NON_SECURE:
            strErrMsg = "ERROR_INTERNET_POST_IS_NON_SECURE"; break;

        case ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED:
            strErrMsg = "ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED"; break;

        case ERROR_INTERNET_INVALID_CA:
            strErrMsg = "ERROR_INTERNET_INVALID_CA"; break;

        case ERROR_INTERNET_CLIENT_AUTH_NOT_SETUP:
            strErrMsg = "ERROR_INTERNET_CLIENT_AUTH_NOT_SETUP"; break;

        case ERROR_INTERNET_ASYNC_THREAD_FAILED:
            strErrMsg = "ERROR_INTERNET_ASYNC_THREAD_FAILED"; break;

        case ERROR_INTERNET_REDIRECT_SCHEME_CHANGE:
            strErrMsg = "ERROR_INTERNET_REDIRECT_SCHEME_CHANGE"; break;

        case ERROR_INTERNET_DIALOG_PENDING:
            strErrMsg = "ERROR_INTERNET_DIALOG_PENDING"; break;

        case ERROR_INTERNET_RETRY_DIALOG:
            strErrMsg = "ERROR_INTERNET_RETRY_DIALOG"; break;

        case ERROR_INTERNET_HTTPS_HTTP_SUBMIT_REDIR:
            strErrMsg = "ERROR_INTERNET_HTTPS_HTTP_SUBMIT_REDIR"; break;

        case ERROR_INTERNET_INSERT_CDROM:
            strErrMsg = "ERROR_INTERNET_INSERT_CDROM"; break;

        case ERROR_FTP_TRANSFER_IN_PROGRESS:
            strErrMsg = "ERROR_FTP_TRANSFER_IN_PROGRESS"; break;

        case ERROR_FTP_DROPPED:
            strErrMsg = "ERROR_FTP_DROPPED"; break;

        case ERROR_FTP_NO_PASSIVE_MODE:
            strErrMsg = "ERROR_FTP_NO_PASSIVE_MODE"; break;
        }

        return strErrMsg;
    }
}

// ------------------------------------------------------------------------- //
std::string getErrMsg()
{
    DWORD dwErrCode = ::GetLastError();
    return getErrMsg(dwErrCode);
}

bool validatePath  ( const char * wszPath )
{
    if (-1 != ::GetFileAttributes(wszPath))
        return true;

    DWORD dwErr = ::GetLastError();
    if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
    {
        if (!::CreateDirectory(wszPath, NULL))
        {
            dwErr = ::GetLastError();
            if ( dwErr != ERROR_ALREADY_EXISTS)
            {
               MOLOG(Log::L_ERROR, CLOGFMT(Common, "Failed to create directory %s Reason : %s"),
                        wszPath, getErrMsg(dwErr).c_str() );
               return false;
            }
        }
    }
    else
    {
        MOLOG(Log::L_ERROR, CLOGFMT(Common,"Failed to access directory %s Reason : %s"), 
                  wszPath, getErrMsg(dwErr).c_str());
        return false;
    }

    return true;
}

VOID LogServiceEvent(DWORD eventID, DWORD eventType, WORD strcnt, LPCSTR *strarray, DWORD bdatacnt,
	LPVOID bdata, char *servname)
{
	HANDLE  hEventSource;

	/* Register ourselves . Always log locally. */    
	hEventSource = RegisterEventSourceA(NULL, servname);
	
    /* If we don't have an eventlog handle, we can't log locally. */  		
	if(hEventSource != NULL) {

		/* Log the event. */
		ReportEventA(hEventSource, // handle of event source
				    (WORD)eventType,				  // event type
				    0,                    // event category
				    (WORD)eventID,              // event ID
				    NULL,                 // current user's SID
				    strcnt,               // strings in lpszStrings
				    bdatacnt,             // no. of bytes of raw data
				    strarray, 		        // array of error strings
				    bdata);               // raw data

		(VOID) DeregisterEventSource(hEventSource);
    }

	return;
}

BOOL dirExist(LPCTSTR sDir)
{
	WIN32_FIND_DATA fd;

	HANDLE hFind = FindFirstFile(sDir, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	FindClose(hFind);

	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{		
		return true;
	}
	else
		return false;
}

BOOL fileExist(LPCSTR sDir)
{
	WIN32_FIND_DATA fd;

	HANDLE hFind = FindFirstFile(sDir, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	FindClose(hFind);

	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{		
		return false;
	}
	else
		return true;
}


//delete the end ' ' && '\\'
// for example str: "c:\RDS\home\   "  -> "c:\RDS\home"
void deleteDirEndSlash(LPSTR str)
{
	int nLen = strlen(str);

	LPSTR ptr = &str[nLen - 1];
	
	while(*ptr && *ptr == (' '))
		ptr--;

	if (*ptr == '\\')
	{
		*ptr = '\0';		
	}
	else
		*(ptr + 1) = '\0';	
}