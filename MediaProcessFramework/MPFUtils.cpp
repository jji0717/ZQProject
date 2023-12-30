#include "MPFCommon.h"
extern "C"
{
#include <time.h>
}

#include <locale>
#include "xmlrpc.h"

//#include "ZqSafeMem.h"

//using ZQ::comextra::sfstrncpy;

MPF_UTILITY_NAMESPACE_BEGIN

BOOL GetNodeGUID(char *szNodeID,DWORD nSize)
{
	HKEY hKey=NULL;
	try
	{
		DWORD dispo;
		//query reg
		long nRet=RegCreateKeyExA(HKEY_LOCAL_MACHINE,"SoftWare\\ZQ\\MPF",0, NULL, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE|KEY_SET_VALUE, NULL, &hKey, &dispo);
		
		if(hKey==NULL)
		{
			return FALSE;
		}
		
		if(nRet==ERROR_SUCCESS)
		{
			DWORD dwSize=nSize;
			nRet=RegQueryValueExA(hKey,"NodeID",NULL,NULL,(LPBYTE)szNodeID,&dwSize);
			szNodeID[dwSize]='\0';
		}
		
		if(nRet!=ERROR_SUCCESS || szNodeID[0]==0)
		{
			GUID guid;
			if(FAILED(::CoCreateGuid(&guid)))
			{
				return FALSE;
			}
			
			_snprintf(szNodeID, nSize-1
				, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"
				, guid.Data1
				, guid.Data2
				, guid.Data3
				, guid.Data4[0], guid.Data4[1]
				, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
				, guid.Data4[6], guid.Data4[7]
				);
			
			RegSetValueExA(hKey,"NodeID",0,REG_SZ,(BYTE*)szNodeID,nSize);
			
		}
		
		if(hKey)
			RegCloseKey(hKey);
	}
	catch(...)
	{
		if(hKey)
			RegCloseKey(hKey);
		return FALSE;
	}
	
	return TRUE;
}
	
UniqueId::UniqueId()
{
	static uint64 lastid  = 0;

	if (lastid ==0)
	{
		time_t tm;
		time(&tm);
		lastid = tm;
		lastid <<=32;
		lastid |= (uint64) rand();
	}

	_id = ++lastid;
	_idstr = convertToStr(_id);
}

UniqueId::~UniqueId()
{
}

std::string UniqueId::convertToStr(const uint64& id)
{
	const static char* ID_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	std::string strResult;
	int nIDCharsLen = strlen(ID_CHARS);
	for (unsigned __int64 left = id; left !=0; left /= nIDCharsLen)
	{
		strResult.insert(strResult.begin(), ID_CHARS[left % strlen(ID_CHARS)]);
	}
	return strResult;
}

MPF_UTILITY_NAMESPACE_END

void print_screen(const char* message)
{
	printf("%s\n", message);
}

std::string getTimeStr(time_t tml)
{
	char tmp[48];
	struct tm *timeData = localtime(&tml);
	if (timeData)
	{ 
		sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d", timeData->tm_year + 1900,
		timeData->tm_mon +1, timeData->tm_mday, timeData->tm_hour, timeData->tm_min, timeData->tm_sec); 
	}
	else
	{
		sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d", 0,
		0, 0, 0, 0, 0); 
	}
	
	return tmp;
}
