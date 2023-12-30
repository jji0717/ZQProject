// HttpYeoman.cpp: implementation of the HttpYeoman class.
//
//////////////////////////////////////////////////////////////////////

#include "HttpYeoman.h"
#include <log.h>
#include "ZQResource.h"
#include <boost/algorithm/string.hpp>

#define SERVER_VER "Sentry " __N2S__(ZQ_PRODUCT_VER_MAJOR) "." __N2S__(ZQ_PRODUCT_VER_MINOR)

#ifdef envlog
#undef envlog
#define envlog (*_env._pHttpSvcLog)
#endif //

static struct _MethodType 
{
	char*	pMethod;		// method names
	int		iMethod;		// numeric method type
} MethodType[] = 
{
	{"OPTIONS",		M_OPTIONS},
	{"GET",			M_GET},
	{"HEAD",		M_HEAD},
	{"POST",		M_POST},
	{"PUT",			M_PUT},
	{"DELETE",		M_DELETE},
	{"TRACE",		M_TRACE},
	{"CONNECT",		M_CONNECT}
};


static struct _ResourceTypeString {
	char*	pExt;			// file extension(s)
	int		iResourceType;	// numeric resource type
} ResourceTypeString[] = 
{
	{".htm.html",		RT_HYPERTEXT},
	{".css",			RT_STYLE_SHEET},
	{".gif.jpg.jpeg",	RT_IMAGE},
	{".pl",				RT_PERL_SCRIPT},
	{".dll.tswl",		RT_DYNAMIC_LINK_LIB},
	{".exe",			RT_EXECUTABLE},
	{".mpg.rm.asf.avi",	RT_VIDEO},
	{".ra.mp3",			RT_AUDIO},
	{".bat",			RT_BATCH},
	{".sysinvoke",		RT_SYSTEMFUNCTION}
};

static struct _HttpStatus
{
	int		iHttpStatus;	// http status code
	char*	pHttpStatus;	// http status string
} HttpStatus[] =
{
	// Informational 100 - 1nn
	{HSC_CONTINUE,						HS_CONTINUE},
	{HSC_SWITCHING_PROTOCOLS,			HS_SWITCHING_PROTOCOLS},
	// Successful 200 - 2nn
	{HSC_OK,							HS_OK},
	{HSC_CREATED,						HS_CREATED},
	{HSC_ACCEPTED,						HS_ACCEPTED},
	{HSC_NON_AUTHORITATIVE_INFORMATION,	HS_NON_AUTHORITATIVE_INFORMATION},
	{HSC_NO_CONTENT,					HS_NO_CONTENT},
	{HSC_RESET_CONTENT,					HS_RESET_CONTENT},
	{HSC_PARTIAL_CONTENT,				HS_PARTIAL_CONTENT},
	// Redirection 300 - 3nn
	{HSC_MULTIPLE_CHOICES,				HS_MULTIPLE_CHOICES},
	{HSC_MOVED_PERMANENTLY,				HS_MOVED_PERMANENTLY},
	{HSC_FOUND,							HS_FOUND},
	{HSC_SEE_OTHER,						HS_SEE_OTHER},
	{HSC_NOT_MODIFIED,					HS_NOT_MODIFIED},
	{HSC_USE_PROXY,						HS_USE_PROXY},
	{HSC_TEMPORARY_REDIRECT,			HS_TEMPORARY_REDIRECT},
	// Client Error 400 - 4nn
	{HSC_BAD_REQUEST,					HS_BAD_REQUEST},
	{HSC_UNAUTHORIZED,					HS_UNAUTHORIZED},
	{HSC_PAYMENT_REQUIRED,				HS_PAYMENT_REQUIRED},
	{HSC_FORBIDDEN,						HS_FORBIDDEN},
	{HSC_NOT_FOUND,						HS_NOT_FOUND},
	{HSC_METHOD_NOT_ALLOWED,			HS_METHOD_NOT_ALLOWED},
	{HSC_NOT_ACCEPTABLE,				HS_NOT_ACCEPTABLE},
	{HSC_PROXY_AUTHENTICATION_REQUIRED,	HS_PROXY_AUTHENTICATION_REQUIRED},
	{HSC_REQUEST_TIMEOUT,				HS_REQUEST_TIMEOUT},
	{HSC_CONFLICT,						HS_CONFLICT},
	{HSC_GONE,							HS_GONE},
	{HSC_LENGTH_REQUIRED,				HS_LENGTH_REQUIRED},
	{HSC_PRECONDITION_FAILED,			HS_PRECONDITION_FAILED},
	{HSC_REQUEST_ENTITY_TOO_LARGE,		HS_REQUEST_ENTITY_TOO_LARGE},
	{HSC_REQUEST_URI_TOO_LARGE,			HS_REQUEST_URI_TOO_LARGE},
	{HSC_UNSUPPORTED_MEDIA_TYPE,		HS_UNSUPPORTED_MEDIA_TYPE},
	{HSC_REQUEST_RANGE_NOT_SATISFIABLE,	HS_REQUEST_RANGE_NOT_SATISFIABLE},
	{HSC_EXPECTATION_FAILED,			HS_EXPECTATION_FAILED},
	// Server Error 500 - 5nn
	{HSC_INTERNAL_SERVER_ERROR,			HS_INTERNAL_SERVER_ERROR},
	{HSC_NOT_IMPLEMENTED,				HS_NOT_IMPLEMENTED},
	{HSC_BAD_GATEWAY,					HS_BAD_GATEWAY},
	{HSC_SERVICE_UNAVAILABLE,			HS_SERVICE_UNAVAILABLE},
	{HSC_GATEWAY_TIMEOUT,				HS_GATEWAY_TIMEOUT},
	{HSC_HTTP_VERSION_NOT_SUPPORTED,	HS_HTTP_VERSION_NOT_SUPPORTED}
};
static struct _ContentType
{
	char*	pExt;			// file extension
	int		iContentType;	// numeric content type
	char*	pContentType;	// string content type
} ContentType[] =
{
	{".htm.html",	CTI_TEXTHTML,				CT_TEXTHTML},
    {".js",         CTI_TEXTJAVASCRIPT,         CT_TEXTJAVASCRIPT},
	{".css",		CTI_TEXTCSS,				CT_TEXTCSS},
	{".gif",		CTI_IMAGEGIF,				CT_IMAGEGIF},
	{".txt.log",	CTI_TEXTPLAIN,				CT_TEXTPLAIN},
	{".jpg.jpeg",	CTI_IMAGEJPEG,				CT_IMAGEJPEG},
	{".asf",		CTI_APPLICATIONOCTETSTREAM,	CT_APPLICATIONOCTETSTREAM},
	{".mpg",		CTI_VIDEOMPEG,				CT_VIDEOMPEG},
	{".avi",		CTI_VIDEOMSVIDEO,			CT_VIDEOMSVIDEO},
	{".ra.rm.ram",	CTI_VIDEOREALMEDIA,			CT_VIDEOREALMEDIA},
	{".exe",		CTI_APPLICATIONOCTETSTREAM,	CT_APPLICATIONOCTETSTREAM},
//	{".xml",		CTI_TEXTXML,	CT_TEXTXML}
};

static int GetMethodType(char* pMethod)
{
    if(NULL == pMethod || '\0' == (*pMethod))
        return M_UNKNOWN;

	int		iMethodType	= M_UNKNOWN;
	int		i;

	for (i=0 ; i<(sizeof(MethodType) / sizeof(MethodType[0])) ; i++) 
	{
		if (strcmp(pMethod, MethodType[i].pMethod) == 0)
		{
			iMethodType = MethodType[i].iMethod;
			break;
		}
	}
	return iMethodType;
}

/****************************************************************************
 * HexToChar
 *		Convert hexified characters in http url
 */
static char HexToChar(char c1, char c2)
{
    // Some browsers/servers send uppercase values.
    c1 = tolower(c1);
    c2 = tolower(c2);
    return ((c2 < 'a')? c2 - '0' : c2 - 'a' + 10) + 
	    ((c1 < 'a')? c1 - '0' : c1 - 'a' + 10) * 16;
}

/****************************************************************************
 * UrlDecode
 *		Decodes a URL encoded string to standard ascii in place
 *		returns:
 *		  char*	pointer to same url string
 */
static char* UrlDecode(char* pBuffer)
{
	char*	ibuf;
	char*	obuf;

	ibuf = obuf = pBuffer;
	
	while(*ibuf)
	{
		switch(*ibuf) 
		{
		default:
			*obuf++ = *ibuf++;
			break;
		case '+':			// plus signs are converted to spaces
			*obuf++ = ' ';
			ibuf++;
			break;
		case '%':			// %xx hex values
			*obuf++ = HexToChar(*(ibuf+1), *(ibuf+2));
			ibuf += 3;
			break;
		}
	}

	// Terminate string
	*obuf=0;

	return pBuffer;
}
static char* UnixToNtFileSpec(char* pFileSpec)
{
    register int i;

    for(i=0; pFileSpec[i]; i++) 
		if (pFileSpec[i] == '/') 
			pFileSpec[i] = '\\';

	return pFileSpec;
}
static int GetResourceType(char* pFileName)
{
	char*	pFileExt	= strrchr(pFileName, '.');
	char*	pExt;
	char*	pDot;
	int		iResourceType	= RT_UNKNOWN;
	int		i;

	if (pFileExt)
	{
		for (i=0; i<(sizeof(ResourceTypeString) / sizeof(ResourceTypeString[0])); i++) 
		{
			pExt = ResourceTypeString[i].pExt;
			while(*pExt) 
			{
				pDot = strchr(pExt+1, '.');
				if (pDot == 0)
					pDot = pExt+strlen(pExt);
				if (strnicmp(pFileExt, pExt, pDot-pExt) == 0) 
				{
					iResourceType = ResourceTypeString[i].iResourceType;
					break;
				}
				pExt = pDot;		// move to next extension
			}
		}
	}
	return iResourceType;
}


static char* GetHttpStatusString(int iHttpStatus)
{
	char*	pHttpStatus = NULL;
	int		i;

	do {
		for (i=0; i<(sizeof(HttpStatus) / sizeof(HttpStatus[0])); i++)
		{
			if (HttpStatus[i].iHttpStatus == iHttpStatus) 
			{
				pHttpStatus = HttpStatus[i].pHttpStatus;
				break;
			}
		}

		if (pHttpStatus == NULL)						// found?
			iHttpStatus = HSC_INTERNAL_SERVER_ERROR;	// no, default to internal error
	} while (pHttpStatus == NULL);

	return pHttpStatus;
}
static int GetContentType(char* pFileName)
{
    char*	pFileExt	= strrchr(pFileName, '.');
    char*	pExt;
    char*	pDot;
    int		iContentType	= CTI_UNKNOWN;
    int		i;

    if (pFileExt)
    {
        for (i=0; i<(sizeof(ContentType) / sizeof(ContentType[0])); i++)
        {
            pExt = ContentType[i].pExt;
            while(*pExt) 
            {
                pDot = strchr(pExt+1, '.');
                if (pDot == 0)
                    pDot = pExt+strlen(pExt);
                if (strnicmp(pFileExt, pExt, pDot-pExt) == 0) 
                {
                    iContentType = ContentType[i].iContentType;
                    break;
                }
                pExt = pDot;		// move to next extension
            }
        }
    }
    return iContentType;
}
static char* GetContentTypeString(int iContentType)
{
	char*	pContentType = NULL;
	int		i;

	do {
		for (i=0; i<(sizeof(ContentType) / sizeof(ContentType[0])); i++)
		{
			if (ContentType[i].iContentType == iContentType)
			{
				pContentType = ContentType[i].pContentType;
				break;
			}
		}

		if (pContentType == NULL)			// found?
			iContentType = CTI_APPLICATIONOCTETSTREAM;	// no, default to octet stream
	} while (pContentType == NULL);

	return pContentType;
}


#define YEOMAN(x)	"HttpYeoman[%12s]"##x,m_strDialogIdentity.c_str()

HttpYeoman::HttpYeoman(ZQTianShan::Sentry::SentryEnv& env):_env(env),HttpDlg(env)
{	
}

HttpYeoman::~HttpYeoman()
{	
}

void HttpYeoman::appendCgiEnv(const char* pName , const char* pValue)
{
	
}
bool HttpYeoman::ErrorResponse( int statuscode)
{	
	char szBuf[4096];
	ZeroMemory(szBuf,sizeof(szBuf));
	int iLens = buildResponseHeader(szBuf,4095,statuscode,0);
	SendData(szBuf,iLens);
	return true;
}
bool HttpYeoman::ProcessHttpReqeust()
{
    DWORD procStart = GetTickCount();
	//parse the first line,find out the request method,resource type
	ParseStartLine();

	if ( m_MethodType == M_UNKNOWN  )
	{
		envlog(ZQ::common::Log::L_ERROR,YEOMAN("Unknown http request type,return bad request"));
		ErrorResponse(HSC_BAD_REQUEST);
		return false;
	}

    // resolve resource reference
    const char *resolved= _env.FindReference(m_strResourceFile);
    if(resolved)
    {
        envlog(Log::L_DEBUG,YEOMAN("ProcessHttpReqeust() resolve resource %s -> %s"), m_strResourceFile.c_str(), resolved);
        m_strResourceFile = resolved;
		m_ResourceType = (ResourceType)GetResourceType((char*)m_strResourceFile.c_str());
        m_ContentType = (ContentTypes)GetContentType((char*)m_strResourceFile.c_str());
    }
	//redirect empty resource to default page
	if ( m_strResourceFile.empty() ) 
	{
		m_strResourceFile ="index.html";
		m_ResourceType  = RT_HYPERTEXT;
        SetContentType("Content-Type: text/html");
	}

	OpenResource();
    DWORD procStop = GetTickCount();
    envlog(ZQ::common::Log::L_DEBUG,YEOMAN("Request cost [%d] msec"), procStop - procStart);

	return true;

}
bool HttpYeoman::ParseStartLine()
{
	envlog(Log::L_DEBUG,YEOMAN("ParseStartLine() enter"));
	
	char	szEndOfLine[]	= "\r\n";
	char	szDelimit[]		= " \r\n";
	char*	pURI= NULL;
	
	int iLineLen = m_vecHeaderField[0].length();
	char *pStartLineBuf = new char[iLineLen+1];
    char *pStartLine    = pStartLineBuf;
	ZeroMemory(pStartLine,iLineLen+1);
	strcpy(pStartLine,m_vecHeaderField[0].c_str());
	
	//find method
	pStartLine = strtok(pStartLine,szDelimit);
	m_MethodType = (HTTPMETHOD)GetMethodType(pStartLine);
	
	//get uri string
	pURI = strtok(NULL, szDelimit);	

	//find out resource type and query string
	if(pURI)
	{
		//find the question mark
		char* pQtMark = strchr(pURI,'?');
		if(pQtMark)
		{
			m_strURIQueryString = (char*)(pQtMark+1) ==NULL ? "" : (char*)(pQtMark+1);
			*pQtMark = '\0';
		}
		else
		{
			m_strURIQueryString = "";
		}		

		//detect request resource type and content type 
        UrlDecode(pURI);
		m_ResourceType = (ResourceType)GetResourceType(pURI);
        m_ContentType = (ContentTypes)GetContentType(pURI);

		if( *pURI == '/' )
		{
			pURI++;
		}
		m_strResourceFile = pURI;
	}
    delete []pStartLineBuf;
	return true;
}

bool HttpYeoman::OpenResource()
{
	switch( m_ResourceType ) 
	{
	case RT_DYNAMIC_LINK_LIB:
		{
			OpenDllFile();
		}
		break;
    case RT_SYSTEMFUNCTION:
        {
            InvokeProcedure();
        }
        break;
	case RT_EXECUTABLE:		// Allows .exe files (StreamingClient.exe) to be downloaded
	case RT_HYPERTEXT:		// HTML - Hypertext Markup Language
	case RT_STYLE_SHEET:	// CSS - Cascading Style Sheets
	case RT_IMAGE:			// GIF, JPEG, etc - graphical image	
	default:
		{
			OpenWebFile();
		}
		break;
	}
	return true;
}

void HttpYeoman::InvokeProcedure()
{
    SetContentType("Content-Type: text/html");
    try
    {
        HttpRequestCtx rqstctx(this);
        if(!rqstctx.gatherVars())
        {
            envlog(ZQ::common::Log::L_ERROR,YEOMAN("Failed to gather request variables during processing [%s]."), m_strResourceFile.c_str());
            ErrorResponse(HSC_BAD_REQUEST);
        }
        _env._pages.SystemPage(&rqstctx);
    }catch(...)
    {
        envlog(ZQ::common::Log::L_ERROR,YEOMAN("catch exception during process [%s]."), m_strResourceFile.c_str());
        ErrorResponse(HSC_INTERNAL_SERVER_ERROR);
    }
}

#define LAYOUT_VAR_TEMPLATE     "#template"
bool HttpYeoman::OpenDllFile()
{
    SetContentType("Content-Type: text/html"); // the default content type
	envlog(Log::L_DEBUG,YEOMAN("OpenDllFile() enter with file:%s"),m_strResourceFile.c_str());
	std::string strFile = m_strResourceFile;	
    std::string proc = "LibProc";
    //resolve the dll file and the procedure
    {
        std::string::size_type  pos_ext = strFile.find_first_of('.');
        std::string             filetype= strFile.substr(pos_ext);
        if(filetype == ".dll")
        {
            const char *filename = _env.FindReference(strFile);
            if(NULL != filename)
                strFile = filename;
        }
        else
        {
            proc = strFile.substr(0, pos_ext);
            const char *filename = _env.FindReference(filetype);
            strFile = (NULL == filename) ? "" : filename;
        }
    }

    if(strFile.empty() || proc.empty())
    {
        envlog(ZQ::common::Log::L_ERROR,YEOMAN("unknown resource file.[%s]"), m_strResourceFile.c_str());
        ErrorResponse(HSC_BAD_REQUEST);
		return false;
    }
	HMODULE 	hDll = LoadLibraryA(strFile.c_str());
	if(hDll == NULL)
	{
		DWORD dwErr =  GetLastError();
		envlog(ZQ::common::Log::L_ERROR,YEOMAN("Load dll %s failed and error code is %u"),strFile.c_str(),dwErr);
        ErrorResponse(HSC_BAD_REQUEST);
		return false;
	}
	try
	{
		LibInitialize		init		= (LibInitialize)	GetProcAddress(hDll,"LibInit");
		LibUnInitialize		uninit		= (LibUnInitialize)	GetProcAddress(hDll,"LibUninit");
		LibProcess			process		= (LibProcess)		GetProcAddress(hDll, proc.c_str());
		if (  !(  init && uninit && process ) ) 
		{
			envlog(Log::L_ERROR,YEOMAN("Invalid lib file [%s],routinue LibInit LibUnit [%s] must be exist"),strFile.c_str(), proc.c_str());
			FreeLibrary(hDll);
            ErrorResponse(HSC_BAD_REQUEST);
			return false;
		}
        
        HttpRequestCtx rqstctx(this);
        if(!rqstctx.gatherVars())
        {
			FreeLibrary(hDll);
			envlog(Log::L_ERROR,YEOMAN("Failed to gather variables during processing [%s]."), m_strResourceFile.c_str());
            ErrorResponse(HSC_BAD_REQUEST);
            return false;
        }

		if(!init(_env._pHttpSvcLog))
		{
			FreeLibrary(hDll);
			envlog(Log::L_ERROR,YEOMAN("lib [%s] initialize failed"), strFile.c_str());
            ErrorResponse(HSC_BAD_REQUEST);
            return false;
		}
        // init ok now

        const char* tmpl = rqstctx.GetRequestVar(LAYOUT_VAR_TEMPLATE);

        if(tmpl)
        {
            HttpResponse &out = rqstctx.GetResponseObject();
            _env._pages.pushHeader(out, tmpl);
            out.flush();
            if(!process(&rqstctx))
            {
                // discard suspect content
                out.clear();
                out << "<h3>Error occured during the request:</h3><blockquote><p>"
                    << (out.GetLastError() ? out.GetLastError() : "(no explanation)")
                    << "</p></blockquote>";
            }
            _env._pages.pushFooter(out, tmpl);
        }
        else // no template name, all content is provided by the plug-in
        {
            envlog(Log::L_DEBUG,YEOMAN("template name missed, all the content is provided by the dll"));
            HttpResponse &out = rqstctx.GetResponseObject();
            if(!process(&rqstctx))
            {
                // discard suspect content
                out.clear();
                _env._pages.pushHeader(out, NULL);
                out << "<h3>Error occured during the request:</h3><blockquote><p>"
                    << (out.GetLastError() ? out.GetLastError() : "(no explanation)")
                    << "</p></blockquote>";
                _env._pages.pushFooter(out, NULL);
            }
        }

        uninit();
        envlog(Log::L_DEBUG,YEOMAN("lib [%s] uninit ok"), strFile.c_str());
		FreeLibrary(hDll);
	}
	catch (...) 
	{
		envlog(Log::L_ERROR,YEOMAN("OpenDllFile() catch a exception when invoke dll"));
        ErrorResponse(HSC_INTERNAL_SERVER_ERROR);
		FreeLibrary(hDll);
	}
	envlog(Log::L_DEBUG,YEOMAN("OpenDllFile() leave"));
	return true;
}
bool HttpYeoman::OpenWebFile()
{
	envlog(Log::L_DEBUG,YEOMAN("OpenWebFile() enter with file:%s"),m_strResourceFile.c_str());


	std::string	strFile = _env._strWebRoot+m_strResourceFile;

	envlog(Log::L_INFO,YEOMAN("OpenWebFile() after bind with webroot ,file path is :%s"),strFile.c_str());

	//step 1 . open the file
	HANDLE hFile  = CreateFileA(	strFile.c_str(),				// file specification
									GENERIC_READ,	// access mode 
									FILE_SHARE_READ,	// share mode
									0,							// security desc 
									OPEN_EXISTING,	// how to create
									0,							// file attributes
									0);
	if( hFile == INVALID_HANDLE_VALUE)
	{
		envlog(Log::L_ERROR,YEOMAN("OpenWebFile() can't open webfile %s"),strFile.c_str());
		ErrorResponse(HSC_NOT_FOUND);
		return false;
	}
	//step 2 .get file size
	BY_HANDLE_FILE_INFORMATION fileInfo;
	GetFileInformationByHandle(hFile,&fileInfo);
	m_CreationTime = fileInfo.ftCreationTime;
	char	szBuf[4096];
	ZeroMemory(szBuf,sizeof(szBuf));
	int iDataLen  = buildResponseHeader(szBuf,4095,HSC_OK,fileInfo.nFileSizeLow);
	SendData( szBuf , iDataLen );
	
	DWORD	dwRead = 0;
	do 
	{
		ReadFile(hFile,szBuf,4096,&dwRead,NULL);
		if(dwRead > 0)
		{
			SendData(szBuf,dwRead);
		}

	} while(dwRead == 4096);
	CloseHandle(hFile);
	envlog(Log::L_DEBUG,YEOMAN("OpenWebFile() leave"));
	return true;
}


static PCHAR monthText[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static PCHAR dayText[]   = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
int HttpYeoman::buildResponseHeader(char* buffer , int bufLen ,int statusCode , __int64 contentLength /* = 0 */)
{
    return buildResponseHeader(buffer, bufLen, statusCode, Fields(), contentLength);
}
int HttpYeoman::buildResponseHeader(char* buffer , int bufLen ,int statusCode , const Fields& fields, __int64 contentLength)
{		
	char*			pHead = buffer;
	char			szServerVersion[64];
	char			szDate[64];

	SYSTEMTIME		tSystemTime;
	// Get current time 
	GetSystemTime(&tSystemTime);

	// Build current time string into date buffer
	sprintf(szDate, "Date: %3s, %02d %3s %4d %02d:%02d:%02d GMT",
					dayText[tSystemTime.wDayOfWeek],					// Thu,
					tSystemTime.wDay,									// 07
					monthText[tSystemTime.wMonth -1],					// Oct
					tSystemTime.wYear,									// 1999
					tSystemTime.wHour,									// 22:
					tSystemTime.wMinute,								// 47:
					tSystemTime.wSecond);								// 52

	// Get server version
	sprintf (szServerVersion, "Server: %s\r\n", SERVER_VER);

	// Was HttpOpenResource() successful?
	if (statusCode == HSC_OK)
	{

		// Build "file exists" response
		if ( !m_strstatus.empty() )			// status from script or application?
			pHead += sprintf(pHead, "%s %s\r\n", HTTP_VER, m_strstatus.c_str());
		else 
			pHead += sprintf(pHead, "%s %d %s\r\n", HTTP_VER, HSC_OK, GetHttpStatusString(HSC_OK));

		pHead += sprintf(pHead, "Server: %s\r\n", SERVER_VER);
		pHead += sprintf(pHead, "%s \r\n", szDate);

		// Servers that accept byte-range requests
		pHead += sprintf(pHead, "Cache-Control: no-cache\r\n");		// no cache
		pHead += sprintf(pHead, "Accept-Ranges: bytes\r\n");		// byte range requests

		// File creation time, when known, is set by open
		// Convert "FILETIME" TO "SYSTEMTIME"
		if (( m_ResourceType == RT_PERL_SCRIPT) ||						// perl application? or
			( m_ResourceType == RT_DYNAMIC_LINK_LIB) ||					// DLL application? or
			((m_CreationTime.dwLowDateTime == 0) &&						// no date? or
			 (m_CreationTime.dwHighDateTime == 0)) ||
			(FileTimeToSystemTime(&m_CreationTime, &tSystemTime) == 0))	// date conversion failed
			GetSystemTime(&tSystemTime);											// then use current time

		// Format: "Last-Modified: Fri, 01 Oct 1999 21:11:54 GMT"
		pHead += sprintf(pHead, "Last-Modified: %3s, %02d %3s %4d %02d:%02d:%02d GMT\r\n",
							dayText[tSystemTime.wDayOfWeek],		// Thu,
							tSystemTime.wDay,						// 07
							monthText[tSystemTime.wMonth -1],		// Oct
							tSystemTime.wYear,						// 1999
							tSystemTime.wHour,						// 22:
							tSystemTime.wMinute,					// 47:
							tSystemTime.wSecond);					// 52

		// Often times a perl script redirecting to another script
		if (m_strlocation.empty()) 
		{
			// Report message length 
			if (m_MethodType != M_HEAD) 
			{			
				// File size, when known, is set by open
				if (contentLength > 0)				// file size known?
					pHead += sprintf(pHead, "Content-Length: %lld\r\n", contentLength );
				else
					pHead += sprintf(pHead, "Transfer-Encoding: chunked\r\n");
			}
			
			// CGI scripts and applications are required to supply the "Content-Type".
			// Otherwise we use the content type determined from the target
			if ( !m_strcontentType.empty())					// supplied by script/application
				pHead += sprintf(pHead, "%s\r\n", m_strcontentType.c_str() );
			else 
				pHead += sprintf(pHead, "Content-Type: %s\r\n", GetContentTypeString(m_ContentType));
		} 
		else
		{
			// Most likely a 302 Redirect
			pHead += sprintf(pHead, "%s\r\n", m_strlocation.c_str() );
			pHead += sprintf(pHead, "Content-Length: 0\r\n");
		}

        // append the additional fields
        for(Fields::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
            pHead += sprintf(pHead, "%s: %s\r\n", it->first.c_str(), it->second.c_str());
        }

		pHead += sprintf(pHead, "\r\n");						// terminate header lines

	}
	else
	{
		// An error occured
		char	szContent[512];		// content buffer
		char*	pText = szContent;	// content buffer addr
		char*	pStatusString;

		// Begin response with HTTP version and status
		pStatusString = GetHttpStatusString(statusCode);

		if ((statusCode == HSC_INTERNAL_SERVER_ERROR) && (buffer != NULL))
			pHead += sprintf(pHead, "%s %d %s\r\n", HTTP_VER, HSC_OK, GetHttpStatusString(HSC_OK));
		else
			pHead += sprintf(pHead, "%s %d %s\r\n", HTTP_VER, statusCode, pStatusString);

		// Unauthorized access requires a "WWW-Authenticate" response header
		if (statusCode == HSC_UNAUTHORIZED)
			pHead += sprintf(pHead, "WWW-Authenticate: Basic\r\n");

		pHead += sprintf(pHead, "Server: %s\r\n", SERVER_VER);
		pHead += sprintf(pHead, "%s\r\n", szDate);
		pHead += sprintf(pHead, "Connection: close\r\n");

		if ((statusCode == HSC_INTERNAL_SERVER_ERROR) && (buffer != NULL)) 
		{
			// Locate beginning of headers
			char*	pData	 = buffer;
			ULONG	iDataLen = bufLen;
			iDataLen = iDataLen<256 ? iDataLen :256;		// just 1st part
			pData[iDataLen] = '\0';							// terminate at reasonable length
			pText += sprintf(pText, "<html>\r\n");
			pText += sprintf(pText, "<head><title>Error in Application</title></head>\r\n");
			pText += sprintf(pText, "<body><h1>Application Error</h1>\r\n");
			pText += sprintf(pText, "The specified application misbehaved by not returning a complete set of HTTP headers.  ");
			pText += sprintf(pText, "The header it did return is:\r\n");
			pText += sprintf(pText, "<p><p><pre>");
			pText += sprintf(pText, pData);
			pText += sprintf(pText, "</pre></body>\r\n");
			pText += sprintf(pText, "</html>\r\n");
		} 
		else 
		{
			// Define error content
			pText += sprintf(pText, "<html>\r\n");
			pText += sprintf(pText, "<head>\r\n<title>%d %s</title>\r\n</head>\r\n",			statusCode, pStatusString);
			pText += sprintf(pText, "<body>\r\n<p><strong>%d %s</strong></p>\r\n</body>\r\n",	statusCode, pStatusString);
			pText += sprintf(pText, "</html>\r\n");
		}

		// Set error content length
		pHead += sprintf(pHead, "Content-Length: %d\r\n", strlen(szContent));

		// Append content type and crlf ending header
		pHead += sprintf(pHead, "Content-Type: text/html\r\n\r\n");

		// Append html error content
		pHead += sprintf(pHead, szContent);
	}
	return pHead-buffer;
	
}
void HttpYeoman::ClearResource()
{
	m_strlocation		=	"";
	m_strcontentType	=	"";
	m_strlocation		=	"";
	m_strstatus			=	"";
	
	HttpDlg::ClearResource();
}
const char*	HttpYeoman::GetRequestQueryString( )
{
	return m_strURIQueryString.c_str();
}


void HttpYeoman::SetCreationTime(FILETIME& fTime )
{
	m_CreationTime = fTime;
}

void HttpYeoman::SetContentType(const std::string& strContentType)
{
	m_strcontentType =strContentType;
}

void HttpYeoman::SetStatusString(const std::string& strStatusString)
{
	m_strstatus = strStatusString;
}

void HttpYeoman::SetLocationString( const std::string& strLocationString )
{
	m_strlocation = strLocationString;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

HttpDialogCreator::HttpDialogCreator(ZQTianShan::Sentry::SentryEnv& env):_env(env)
{
	
}
HttpDialogCreator::~HttpDialogCreator()
{
}
IDialogue* HttpDialogCreator::createDialogue()
{
	HttpYeoman* dlg = new HttpYeoman(_env);
	return (IDialogue*)dlg;
}
void HttpDialogCreator::releaseDialogue(IN IDialogue* dlg) 
{
	if(!dlg)
		return;

    delete dlg;
//	ZQ::common::MutexGuard gd(_mutex);
//	DialogMap::iterator it = _map.find(dlg);
//	if( it != _map.end())
//	{
//		delete (HttpYeoman*)dlg;
//		_map.erase(it);
//	}
}

//////////////////////////////////////////////////////////////////////////
HttpResponse::HttpResponse(HttpYeoman* pHttpYeoman)
:_pHttpYeoman(pHttpYeoman), _statusCode(HSC_OK)
{
}
HttpResponse::~HttpResponse()
{
    flush();
    send();
    _pHttpYeoman = NULL;
}
void HttpResponse::flush()
{
    _buf_stable << _buf.str();
    _buf.str("");//clear buffer
}
void HttpResponse::send()
{
    if(NULL == _pHttpYeoman)
        return;

    if(HSC_OK == _statusCode)
    {
        std::string msg;
        _buf_stable.str().swap(msg);
        if(!msg.empty())
        {
            char hdrbuf[4096] = {0};
            int hdrlen = _pHttpYeoman->buildResponseHeader(hdrbuf, 4095, HSC_OK, _fields, msg.size());
            //send header
            _pHttpYeoman->SendData(hdrbuf, hdrlen);
            //send msg
            _pHttpYeoman->SendData(msg.data(), msg.size());
        }
    }
    else
    {
        _pHttpYeoman->ErrorResponse(_statusCode);
    }

    // clear buffer
    _buf_stable.str("");
}

HttpRequestCtx::HttpRequestCtx(HttpYeoman* pHttpYeoman)
:_pHttpYeoman(pHttpYeoman), _response(pHttpYeoman)
{
    _rootDir = _pHttpYeoman->_env._strWebRoot;
    _rootURL = _pHttpYeoman->_env._selfInfo.adminRootUrl;
    _uri = _pHttpYeoman->m_strResourceFile;
    _method = _pHttpYeoman->m_MethodType;
}
// decode the urlencoded query string
static IHttpRequestCtx::RequestVars getUrlVars(const char* urlstr)
{
    IHttpRequestCtx::RequestVars vars;

    if(NULL == urlstr)
        return vars;
    ZQ::common::URLStr url(urlstr, true);//case sensitive
    int i = 0;
    const char* varname = NULL;
    while(varname = url.getVarname(i++))
    {
        const char* varval = url.getVar(varname);
        if(varval)
            vars[varname] = varval;
    }

    return vars;
}

#define CTXLOG (*_pHttpYeoman->_env._pHttpSvcLog)
bool HttpRequestCtx::gatherVars()
{
    _vars = getUrlVars(_pHttpYeoman->GetRequestQueryString());
    const char* postData = _pHttpYeoman->GetRequestPostData();
    const char* type = _pHttpYeoman->getHeaderValue("Content-Type");
    if(postData && type)
    { // with post data
        // check the binary data
        if(strlen(postData) != _pHttpYeoman->GetPostDataSize())
        { // bindary data with '\0' embedded.
            CTXLOG(ZQ::common::Log::L_WARNING, CLOGFMT(HttpRequestCtx, "Binary post data! Reject!"));
            return false;
        }
        // there are two kind of post data:
        // application/x-www-form-urlencoded and multipart/form-data
        if(0 == strnicmp(type, "application/x-www-form-urlencoded", strlen("application/x-www-form-urlencoded")))
        { // urlencoded
            IHttpRequestCtx::RequestVars vars = getUrlVars(postData);
            // merge into the query variables
            std::copy(vars.begin(), vars.end(), std::inserter(_vars, _vars.end()));
        }
        else if( 0 == strnicmp(type, "multipart/form-data", strlen("multipart/form-data")) )
        { // MIME multipart data
            // check the boundary
            const char* boundary = strstr(type, "boundary=");
            if(NULL == boundary || '\0' == *boundary)
            { // bad boundary
                CTXLOG(ZQ::common::Log::L_WARNING, CLOGFMT(HttpRequestCtx, "Bad format of post data. No valid boundary with type multipart/formdata."));
                return false;
            }
            std::string boundaryValue = (boundary + strlen("boundary="));
            boost::trim(boundaryValue);
#pragma message(__MSGLOC__"Should we care the quote-string boundary value?")
            // processing the multipart content
            std::string dashBoundary = std::string("--") + boundaryValue;
            std::string delimeter = std::string("\r\n") + dashBoundary;
            const char* p = strstr(postData, dashBoundary.c_str()); // the current position
            while(p)
            {
                if(0 == strncmp(p + dashBoundary.size(), "--", 2)) // end
                { // end of the multipart, normal quit
                    break;
                }

                const char* begin = strstr(p + dashBoundary.size(), "\r\n"); // begin of the body part
                if(begin)
                {
                    begin += 2; // skip the CRLF
                }
                else
                { // invalid format
                    CTXLOG(ZQ::common::Log::L_WARNING, CLOGFMT(HttpRequestCtx, "Bad format of post data. Can't position the begin of the body part."));
                    return false;
                }
                const char* end = strstr(begin, delimeter.c_str());
                if(end)
                {
                    p = end + 2; // skip the CRLF
                }
                else
                {
                    CTXLOG(ZQ::common::Log::L_WARNING, CLOGFMT(HttpRequestCtx, "Bad format of post data.. Can't position the end of the body part."));
                    return false;
                }

                // we got the body part now

                // split the header and the body
                const char* headerEnd = strstr(begin, "\r\n\r\n");
                if(!(headerEnd && (headerEnd + 4) <= end))
                { // invalid header! ignore or abort?
                    return false;
                }

                // parse the header for the name
#pragma message(__MSGLOC__"Need we process the Content-Type and Content-Transfer-Encoding ?")
                std::string name;
                const char* lineBegin = begin;
                while(lineBegin < headerEnd)
                {
#pragma message(__MSGLOC__"Should care the case of multilines header.")
                    const char* lineEnd = strstr(lineBegin, "\r\n"); // must exist
                    if(0 == strnicmp(lineBegin, "Content-disposition:", strlen("Content-disposition:")))
                    { // get the target line
                        std::vector<std::string> params;
                        boost::split(params, std::string(lineBegin + strlen("Content-disposition:"), lineEnd), boost::is_any_of(";"));
                        for(size_t i = 0; i < params.size(); ++i)
                        {
                            std::string& param = params[i];
                            boost::trim(param);
                            if( (param.size() > strlen("name=\"\""))
                                && (0 == strnicmp(param.c_str(), "name=\"", strlen("name=\"")))
                                && ('"' == param[param.size() - 1]))
                            {
                                name.assign(param, strlen("name=\""), param.size() - strlen("name=\"\""));
                                break;
                            }
                        } // for end (parameter)
                        if(!name.empty())
                            break;
                    }
                    lineBegin = lineEnd + 2;
                } // while end (header)
                if(!name.empty())
                {
                    _vars[name].assign(headerEnd + 4, end);
                }
                else
                { // no variable name, ignore or abort
#pragma message(__MSGLOC__"no variable name, ignore or abort?")
                }
            } // while end (multi-part)
        }
        else
        { // don't care
            CTXLOG(ZQ::common::Log::L_WARNING, CLOGFMT(HttpRequestCtx, "ignore post data of type [%s]"), type);
        }
    }
    return true;
}
