#include "stdafx.h"
#include "ZQSNMPOperPkg.h"

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

TCHAR* varTypeData[] = 
{ 
	_T("Gauge32"),_T("Count32"),  _T("Int"),
	_T("Ticks"),  _T("String"),   _T("Float"),
	_T("IpAddr"), _T("Opaque"),   _T("Oid"),
	_T("Null")
};

#ifdef __cplusplus
extern "C" {
#endif

typedef map<string,string> stringMap;
typedef stringMap::iterator  mapitor;
typedef stringMap::value_type maptype;

string    m_strServiceName; //add by dony 20070122 for add variable's name in variable's data
stringMap m_OIDVarNameMap; // key = varName, data=varOID add by dony 20070122 for add variable's name in variable's data
HANDLE m_handle;
CRITICAL_SECTION m_csect;
TCHAR  m_szServiceName[SERVICENAMELEN];
TCHAR  m_szXMLFileName[VAR_DATALEN];
//bool   GetRegistryValues();
//bool   GetCompanyOID();
void   PrintStatusError(int nErrorStatus, TCHAR *&szErrorMsg);
WCHAR  * WSNMP_AnyToStr(AsnObjectSyntax *sAny);
char   * SNMP_AnyToStr(AsnObjectSyntax *sAny);
bool    GetRequest(HANDLE hSession,AsnObjectIdentifier &asnOid,TCHAR *ResultVarType,TCHAR *ResultReadWrite, TCHAR *ResultValue);
bool    SetRequest(HANDLE hSession,AsnObjectIdentifier &asnOid,DWORD dwAddress,INT wType);
bool    IsNumber(TCHAR *szInputData);
void    GetStringLeftAtChar(const string& contString,char vChar,string &strTemp);


BOOL APIENTRY DllMain( HANDLE hModule,DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			::InitializeCriticalSection(&m_csect);
			memset(m_szServiceName,0,sizeof(m_szServiceName));
			memset(m_szXMLFileName,0,sizeof(m_szXMLFileName));
			m_handle = NULL;
			m_OIDVarNameMap.clear();
	//		GetRegistryValues();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			if ( m_handle )
			{
				SNMPOperCloseSession(m_handle);
			}
			::DeleteCriticalSection(&m_csect);
			memset(m_szServiceName,0,sizeof(m_szServiceName));
			if ( !m_OIDVarNameMap.empty())
			{
				m_OIDVarNameMap.clear();
			}
			break;
    }
    return TRUE;
}

void GetStringLeftAtChar(const string& contString,char vChar,string &strTemp)
{
	int strLen = contString.size();
	int cur;
	for(cur = 0;cur < strLen;cur++)
	{
		if(contString[cur] == vChar)
			break;
	}
	if(cur >= strLen)
	{
		strTemp = "";
		return;
	}
	else
	{
		int tempCur=0;
		strTemp ="";
		while(tempCur<cur)
		{
			strTemp +=(char)contString[tempCur];
			tempCur++;
		}
	}
}


void PrintStatusError(int nErrorStatus, TCHAR *&szErrorMsg)
{
	szErrorMsg = (TCHAR*)malloc(sizeof(TCHAR)*64);
	switch(nErrorStatus)
	{
	case SNMP_ERRORSTATUS_NOERROR:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_NOERROR"));
		break;
	case SNMP_ERRORSTATUS_TOOBIG:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_TOOBIG"));
		break;
	case SNMP_ERRORSTATUS_NOSUCHNAME:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_NOSUCHNAME"));
		break;
	case SNMP_ERRORSTATUS_BADVALUE:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_BADVALUE"));
		break;
	case SNMP_ERRORSTATUS_READONLY:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_READONLY"));
		break;
	case SNMP_ERRORSTATUS_GENERR:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_GENERR"));
		break;
	case SNMP_ERRORSTATUS_NOACCESS:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_NOACCESS"));
		break;
	case SNMP_ERRORSTATUS_WRONGTYPE:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_WRONGTYPE"));
		break;
	case SNMP_ERRORSTATUS_WRONGLENGTH:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_WRONGLENGTH"));
		break;
	case SNMP_ERRORSTATUS_WRONGENCODING:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_WRONGENCODING"));
		break;
	case SNMP_ERRORSTATUS_WRONGVALUE:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_WRONGVALUE"));
		break;
	case SNMP_ERRORSTATUS_NOCREATION:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_NOCREATION"));
		break;
	case SNMP_ERRORSTATUS_INCONSISTENTVALUE:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_INCONSISTENTVALUE"));
		break;
	case SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE"));
		break;
	case SNMP_ERRORSTATUS_COMMITFAILED:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_COMMITFAILED"));
		break;
	case SNMP_ERRORSTATUS_UNDOFAILED:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_UNDOFAILED"));
		break;
	case SNMP_ERRORSTATUS_AUTHORIZATIONERROR:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_AUTHORIZATIONERROR"));
		break;
	case SNMP_ERRORSTATUS_NOTWRITABLE:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_NOTWRITABLE"));
		break;
	case SNMP_ERRORSTATUS_INCONSISTENTNAME:
		_tcscpy(szErrorMsg,_T("SNMP_ERRORSTATUS_INCONSISTENTNAME"));
		break;
	default:
		_tcscpy(szErrorMsg,_T("Unknown Error"));
	}
}

WCHAR    * WSNMP_AnyToStr(AsnObjectSyntax *sAny)
{
	DWORD dwValue = 0;
	UINT  uLen = 0;
	BYTE  *puData = 0;
	WCHAR *pString = NULL;
	
	switch ( sAny->asnType )
	{
		case ASN_INTEGER:    
			pString = (WCHAR *)SnmpUtilMemAlloc(VAR_TYPELEN);
			if(pString)
			{
				_ltow(sAny->asnValue.number,pString,10);
			}
			break;
		case ASN_RFC1155_COUNTER:
			dwValue = sAny->asnValue.counter;
			pString = (WCHAR*) SnmpUtilMemAlloc( VAR_TYPELEN );
			if(pString)
			{
				_ultow(dwValue,pString,10);
			}
			break;
		case ASN_RFC1155_GAUGE:
			dwValue = sAny->asnValue.gauge;
			pString = (WCHAR *) SnmpUtilMemAlloc(VAR_TYPELEN);
			if (pString)
			{
				_ultow(dwValue,pString,10);
			}
			break;
		case ASN_RFC1155_TIMETICKS:
			dwValue = sAny->asnValue.ticks;
			pString = (WCHAR *) SnmpUtilMemAlloc(VAR_TYPELEN);
			if (pString)
			{
				_ultow(dwValue,pString,10);
			}
			break;
		case ASN_OCTETSTRING:   
			uLen    = (sAny->asnValue.string.length+1)*2;
			puData  = sAny->asnValue.string.stream;
			pString = (WCHAR *)SnmpUtilMemAlloc(uLen + 2);
			if(pString)
			{
				MultiByteToWideChar(
				CP_ACP,         // code page
				0,              // character-type options
				(char*)puData,        // address of string to map
				sAny->asnValue.string.length,      // number of bytes in string
				pString,    // address of wide-character buffer
				VAR_DATALEN);             // size of buffer);
			}
			break;
			/*
		case ASN_SEQUENCE:      
			uLen = sAny->asnValue.sequence.length;
			puData = sAny->asnValue.sequence.stream;
			if(pString)
			{
				if (sAny->asnValue.arbitrary.length)
				{
				strncpy(pString, (const char*)puData, uLen -1);
				pString[uLen] = '\0';
				}
			}
			break;
		case ASN_RFC1155_IPADDRESS:
			if (sAny->asnValue.address.length )
			{
				UINT i;
				char szBuf[VAR_TYPELEN];

				uLen = sAny->asnValue.address.length;
				puData = sAny->asnValue.address.stream;

				pString = (char *) SnmpUtilMemAlloc( uLen * 4 );
				if(pString)
				{
					pString[0] = '\0';

					for (i = 0; i < uLen; i++)
					{
						
//						lstrcat( pString, itoa( puData[i], szBuf, 10 ) );    
						strcat( pString, itoa(puData[i], szBuf,10 ));    
						if( i < uLen-1 )
//							lstrcat( pString, "." );
							strcat(pString,".");

					}
				}
			}
			else
				pString = NULL;
			break;
		case ASN_RFC1155_OPAQUE:
			if( sAny->asnValue.arbitrary.length )
			{
				uLen = sAny->asnValue.arbitrary.length;
				puData = sAny->asnValue.arbitrary.stream;
				pString = (char *) SnmpUtilMemAlloc(uLen + 1);
				if (pString)
				{
					if (sAny->asnValue.arbitrary.length)
					{
						strncpy(pString, (const char*)puData, uLen -1);
						pString[uLen] = '\0';
					}
				}
			}
			else
				pString = NULL;
			break;
		case ASN_OBJECTIDENTIFIER:
			if( sAny->asnValue.object.idLength )
			{
				pString = (char *) SnmpUtilMemAlloc( sAny->asnValue.object.idLength * 5 );
				if(pString)
				{
					UINT i;
					char szBuf[VAR_TYPELEN];
					for( i = 0; i < sAny->asnValue.object.idLength; i++ )
					{
//						lstrcat( pString, itoa( sAny->asnValue.object.ids[i], szBuf, 10 ) );    
						strcat( pString, itoa( sAny->asnValue.object.ids[i], szBuf, 10 ) );    
						if(i < sAny->asnValue.object.idLength-1)
//							lstrcat( pString, "." );
							strcat( pString, "." );
					}
				}
			}
			else
				pString = NULL;
			break;
			*/
		default:             
			return( FALSE );
	}
    return(pString);
}

char * SNMP_AnyToStr(AsnObjectSyntax *sAny)
{
	DWORD dwValue = 0;
	UINT uLen = 0;
	BYTE *puData = 0;
	char *pString = NULL;
	
	switch ( sAny->asnType )
	{
		case ASN_INTEGER:    
			pString = (char *) SnmpUtilMemAlloc(VAR_TYPELEN);
			if(pString)
			{
				ltoa(sAny->asnValue.number, pString, 10 );
			}
			break;
		case ASN_RFC1155_COUNTER:
			dwValue = sAny->asnValue.counter;
			pString = (char *) SnmpUtilMemAlloc( VAR_TYPELEN );
			if(pString)
			{
				ultoa(dwValue, pString, 10);
			}
			break;
		case ASN_RFC1155_GAUGE:
			dwValue = sAny->asnValue.gauge;
			pString = (char *) SnmpUtilMemAlloc(VAR_TYPELEN);
			if (pString)
			{
				ultoa(dwValue, pString, 10);
			}
			break;
		case ASN_RFC1155_TIMETICKS:
			dwValue = sAny->asnValue.ticks;
			pString = (char *) SnmpUtilMemAlloc(VAR_TYPELEN);
			if (pString)
			{
				ultoa(dwValue, pString, 10);
			}
			break;
		case ASN_BITS:
			uLen    = sAny->asnValue.bits.length+1;
			puData  = sAny->asnValue.bits.stream;
			pString = (char *) SnmpUtilMemAlloc(uLen + 1);
			if(pString)
			{
				if (sAny->asnValue.arbitrary.length)
				{
					strncpy(pString, (const char*)puData, uLen -1);
					pString[uLen] = '\0';
				}
			}
			break;
		case ASN_OCTETSTRING:   /* Same as ASN_RFC1213_DISPSTRING */
			uLen    = sAny->asnValue.string.length+1;
			puData  = sAny->asnValue.string.stream;
			pString = (char *) SnmpUtilMemAlloc(uLen + 1);
			if(pString)
			{
				if (sAny->asnValue.arbitrary.length)
				{
					strncpy(pString, (const char*)puData, uLen -1);
					pString[uLen] = '\0';
				}
			}
			break;
		case ASN_SEQUENCE:      /* Same as ASN_SEQUENCEOF */
			uLen = sAny->asnValue.sequence.length;
			puData = sAny->asnValue.sequence.stream;
			if(pString)
			{
				if (sAny->asnValue.arbitrary.length)
				{
				strncpy(pString, (const char*)puData, uLen -1);
				pString[uLen] = '\0';
				}
			}
			break;
		case ASN_RFC1155_IPADDRESS:
			if (sAny->asnValue.address.length )
			{
				UINT i;
				char szBuf[VAR_TYPELEN];

				uLen = sAny->asnValue.address.length;
				puData = sAny->asnValue.address.stream;

				pString = (char *) SnmpUtilMemAlloc( uLen * 4 );
				if(pString)
				{
					pString[0] = '\0';

					for (i = 0; i < uLen; i++)
					{
						
//						lstrcat( pString, itoa( puData[i], szBuf, 10 ) );    
						strcat( pString, itoa(puData[i], szBuf,10 ));    
						if( i < uLen-1 )
//							lstrcat( pString, "." );
							strcat(pString,".");

					}
				}
			}
			else
			{
				pString = NULL;
			}
			break;
		case ASN_RFC1155_OPAQUE:
			if( sAny->asnValue.arbitrary.length )
			{
				uLen = sAny->asnValue.arbitrary.length;
				puData = sAny->asnValue.arbitrary.stream;
				pString = (char *) SnmpUtilMemAlloc(uLen + 1);
				if (pString)
				{
					if (sAny->asnValue.arbitrary.length)
					{
						strncpy(pString, (const char*)puData, uLen -1);
						pString[uLen] = '\0';
					}
				}
			}
			else
			{
				pString = NULL;
			}
			break;
		case ASN_OBJECTIDENTIFIER:
			if( sAny->asnValue.object.idLength )
			{
				pString = (char *) SnmpUtilMemAlloc( sAny->asnValue.object.idLength * 5 );
				if(pString)
				{
					UINT i;
					char szBuf[VAR_TYPELEN];
					for( i = 0; i < sAny->asnValue.object.idLength; i++ )
					{
						strcat( pString, itoa( sAny->asnValue.object.ids[i], szBuf, 10 ) );    
						if(i < sAny->asnValue.object.idLength-1)
							strcat( pString, "." );
					}
				}
			}
			else
			{
				pString = NULL;
			}
			break;
		default:             /* Unrecognised data type */
			return( FALSE );
	}
    return(pString);
}

/// get request
// this will retrieve the value of the given valiable
bool GetRequest(HANDLE hSession,AsnObjectIdentifier &asnOid,TCHAR *ResultVarType,TCHAR *ResultReadWrite,TCHAR *ResultValue)
{
	try
	{
		TCHAR *asciiStr, *tmpStr;
		TCHAR szDataTemp[VAR_DATALEN];
		TCHAR szDataTemp1[VAR_DATALEN];
		memset(szDataTemp1,0,sizeof(szDataTemp1));
		memset(szDataTemp,0,sizeof(szDataTemp));
		TCHAR *pos = NULL;
		int iLen ;

		AsnInteger	errorStatus=0;  	// Error type that is returned if encountered
		AsnInteger	errorIndex=0;		// Works with variable above

		SnmpVarBindList snmpVarList;
		snmpVarList.list = NULL;
		snmpVarList.len = 0;

		snmpVarList.len++;	
		snmpVarList.list = (SnmpVarBind *)SNMP_realloc(snmpVarList.list, sizeof(SnmpVarBind)*snmpVarList.len); 
		

		// Assigning OID to variable bindings list
		SnmpUtilOidCpy(&snmpVarList.list[0].name,&asnOid);
		snmpVarList.list[0].value.asnType = ASN_NULL;
        
		
		// initiates the GET request
		if(!SnmpMgrRequest(hSession,SNMP_PDU_GET,&snmpVarList,&errorStatus,&errorIndex))
		{		
			SnmpUtilVarBindListFree(&snmpVarList);
			SnmpUtilOidFree(&asnOid);
			
			asciiStr = (TCHAR*)malloc(sizeof(TCHAR)*VAR_DATALEN);
			PrintStatusError(errorStatus,tmpStr);
			_stprintf(asciiStr,_T("Snmp Request Failed\nErrorStatus: %s  ErrorIndex: %d"),tmpStr,errorIndex);
			OutputDebugString(asciiStr);
			free(asciiStr);
			free(tmpStr);		
			return false;
		}
		if(errorStatus > 0)
		{
			SnmpUtilVarBindListFree(&snmpVarList);
			SnmpUtilOidFree(&asnOid);

			asciiStr = (TCHAR*)malloc(sizeof(TCHAR)*VAR_DATALEN);
			PrintStatusError(errorStatus,tmpStr);
			_stprintf(asciiStr,_T("ErrorStatus: %s  ErrorIndex: %d"),tmpStr,errorIndex);
			OutputDebugString(asciiStr);
			free(asciiStr);
			free(tmpStr);
			return false;
		}
		memset(szDataTemp1,0,sizeof(szDataTemp1));
		memset(szDataTemp,0,sizeof(szDataTemp));
	#if defined _UNICODE || defined UNICODE
		char *sTemp;
		sTemp = (char*)malloc(sizeof(char)*VAR_DATALEN);
		sTemp = SNMP_AnyToStr(&snmpVarList.list[0].value);
		MultiByteToWideChar(
					CP_ACP,         // code page
					0,              // character-type options
					sTemp,        // address of string to map
					strlen (sTemp),      // number of bytes in string
					szDataTemp,    // address of wide-character buffer
					VAR_DATALEN);             // size of buffer);
	#else
		_tcscpy(szDataTemp,SNMP_AnyToStr(&snmpVarList.list[0].value));
	#endif
		*(szDataTemp+(_tcsclen(szDataTemp))) = 0;

		// add by dony 20070122 for add variable's name and type in variable's data
	#ifdef	VARNAMEIN_MODE
		pos  = _tcsrchr(szDataTemp, _T('@'));
		if ( pos )
		{
			if ( *(pos+1) =='1' ) //整数类型
			{
				_tcscpy(ResultVarType,varTypeData[2]);
			}
			else if ( *(pos+1) =='2' ) //字符串类型
			{
				_tcscpy(ResultVarType,varTypeData[4]);
			}
			else if ( *(pos+1) =='3') //浮点数类型
			{
				_tcscpy(ResultVarType,varTypeData[5]);
			}
			if ( *(pos+3) =='0') 
			{
				_tcscpy(ResultReadWrite,_T("ReadOnly"));
			}
			else if ( *(pos+3) =='1' )
			{
				_tcscpy(ResultReadWrite,_T("WriteOnly"));
			}
			else
			{
				_tcscpy(ResultReadWrite,_T("ReadWrite"));
			}
			iLen = _tcsclen(szDataTemp);
			for ( int i = 0; i < iLen; i ++ )
			{
				if ( *(szDataTemp+i) =='.') 
				{
					break;
				}
			}
			iLen = iLen - _tcslen(pos) - i;
			_tcsncpy(szDataTemp1,szDataTemp+i+1,iLen);
			_stprintf(ResultValue,_T("%s%s"),szDataTemp1,pos+5);
		}
	#else
		_tcscpy(ResultValue,szDataTemp);
		_tcscpy(ResultVarType,varTypeData[snmpVarList.list[0].value.asnType]);
	#endif

		if ( pos )
		{
			pos = NULL;
		}
		SnmpUtilVarBindListFree(&snmpVarList);	
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool  IsNumber(TCHAR *szInputData)
{
	bool bReturn = true;
	int iLen = _tcsclen(szInputData);
	
	for ( int i =0; i < iLen; i++)
	{
		if ( szInputData[i] >='0' && szInputData[i] <='9')
		{
		}
		else
		{
			bReturn = false;
			break;
		}
	}
	return bReturn;
}
//  set request
//  gets the value fro a cvariable from the user,
//  convert to the related variable type from string and assigns to the server variable
bool  SetRequest(HANDLE hSession,AsnObjectIdentifier &asnOid,DWORD dwAddress,INT wType)
{
	try
	{
	
		TCHAR *pBuff, *pBuff2;
		int i;
		int iLen = 0;
		char sTemp[VAR_DATALEN] ={0};
		
		
		SnmpVarBindList snmpVarList;
		
		AsnInteger	errorStatus=0;	// Error type that is returned if encountered
		AsnInteger	errorIndex=0;	// Works with variable above

		snmpVarList.list = NULL;
		snmpVarList.len = 0;

		snmpVarList.len++;
		snmpVarList.list = (SnmpVarBind *)SNMP_realloc(snmpVarList.list, sizeof(SnmpVarBind)*snmpVarList.len); 
				
		// Assigning OID to variable bindings list
		SnmpUtilOidCpy(&snmpVarList.list[0].name,&asnOid);
		snmpVarList.list[0].value.asnType = ASN_NULL;
    		
		snmpVarList.list[0].value.asnType = (INT)(wType);
				
		switch (snmpVarList.list[0].value.asnType)
		{
		case ASN_BITS:
			/*
			iLen = _tcslen((TCHAR*)dwAddress);
			snmpVarList.list[0].value.asnValue.bits.dynamic = TRUE;
			snmpVarList.list[0].value.asnValue.bits.length  = iLen;
			snmpVarList.list[0].value.asnValue.bits.stream = (BYTE*)SnmpUtilMemAlloc(iLen*sizeof(BYTE));
	#if defined _UNICODE || defined UNICODE
			memset(sTemp,0,sizeof(sTemp));
			WideCharToMultiByte(CP_ACP,NULL,(WCHAR*)dwAddress,-1,sTemp,sizeof(sTemp),NULL,NULL);
			memcpy((char*)snmpVarList.list[0].value.asnValue.bits.stream,sTemp,iLen);
			
	#else
			memcpy((TCHAR*)snmpVarList.list[0].value.asnValue.bits.stream,(TCHAR*)dwAddress,iLen);
	#endif
			*/
			memset(sTemp,0,sizeof(sTemp));
	#if defined _UNICODE || defined UNICODE
			WideCharToMultiByte (CP_ACP, 0,
						  (WCHAR*)dwAddress, -1, sTemp, sizeof(sTemp),
						  NULL, NULL);
	#else
			sprintf(sTemp,"%s",(char*)dwAddress);
	#endif
	     
			iLen = strlen(sTemp);
			if ((NULL != snmpVarList.list[0].value.asnValue.bits.stream) )
	//		    &&	(snmpVarList.list[0].value.asnValue.bits.dynamic))
			{
				delete[] snmpVarList.list[0].value.asnValue.bits.stream;
				snmpVarList.list[0].value.asnValue.bits.stream = 0;
				snmpVarList.list[0].value.asnValue.bits.length = 0;
			}
			if (iLen > 0)
			{
				snmpVarList.list[0].value.asnValue.bits.stream = new (BYTE[iLen]);
				snmpVarList.list[0].value.asnValue.bits.length = iLen;
				snmpVarList.list[0].value.asnValue.bits.dynamic = false;
				memcpy(snmpVarList.list[0].value.asnValue.bits.stream,sTemp,iLen);
			}
	 		break;
		case ASN_OCTETSTRING:
			/*
			iLen = _tcslen((TCHAR*)dwAddress);
			snmpVarList.list[0].value.asnValue.string.dynamic = TRUE;
			snmpVarList.list[0].value.asnValue.string.length  = iLen;
			snmpVarList.list[0].value.asnValue.string.stream = (BYTE*)SnmpUtilMemAlloc(iLen*sizeof(BYTE));
	#if defined _UNICODE || defined UNICODE
			memset(sTemp,0,sizeof(sTemp));
			WideCharToMultiByte(CP_ACP,NULL,(WCHAR*)dwAddress,-1,sTemp,sizeof(sTemp),NULL,NULL);
			memcpy((char*)snmpVarList.list[0].value.asnValue.string.stream,sTemp,iLen);
	#else
			memcpy((TCHAR*)snmpVarList.list[0].value.asnValue.string.stream,(TCHAR*)dwAddress,iLen);
     #endif
	 */
				
			// modify by dony 20070122 for add variable's name in the variable's data
			memset(sTemp,0,sizeof(sTemp));
	#if defined _UNICODE || defined UNICODE
			WideCharToMultiByte (CP_ACP, 0,
						  (WCHAR*)dwAddress, -1, sTemp, sizeof(sTemp),
						  NULL, NULL);
	#else
			sprintf(sTemp,"%s",(char*)dwAddress);
	#endif
	     
			iLen = strlen(sTemp);
			if ((NULL != snmpVarList.list[0].value.asnValue.string.stream) )
	//		    &&	(snmpVarList.list[0].value.asnValue.string.dynamic))
			{
				delete[] snmpVarList.list[0].value.asnValue.string.stream;
				snmpVarList.list[0].value.asnValue.string.stream = 0;
				snmpVarList.list[0].value.asnValue.string.length = 0;
			}
			if (iLen > 0)
			{
				snmpVarList.list[0].value.asnValue.string.stream = new (BYTE[iLen]);
				snmpVarList.list[0].value.asnValue.string.length = iLen;
				snmpVarList.list[0].value.asnValue.string.dynamic = false;
				memcpy(snmpVarList.list[0].value.asnValue.string.stream,sTemp,iLen);
			}
			break;
			
		case ASN_INTEGER32:
			snmpVarList.list[0].value.asnValue.number = *((LONG*)dwAddress);
			break;
		case ASN_TIMETICKS:
			snmpVarList.list[0].value.asnValue.ticks = *((LONG*)dwAddress);
			break;
		case ASN_GAUGE32:
			snmpVarList.list[0].value.asnValue.gauge = *((LONG*)dwAddress);
			break;
		case ASN_COUNTER32:
			snmpVarList.list[0].value.asnValue.counter = *((LONG*)dwAddress);
			break;

		case ASN_IPADDRESS:
			pBuff = (TCHAR*)dwAddress;
			pBuff2 = pBuff;

			snmpVarList.list[0].value.asnValue.address.dynamic = TRUE;
			snmpVarList.list[0].value.asnValue.address.length = 4;
			snmpVarList.list[0].value.asnValue.address.stream = (UCHAR*)SnmpUtilMemAlloc(4);

			for(i=0;i<4;i++)
			{
				pBuff2 = _tcschr(pBuff,_T('.'));

				if(pBuff2)
				{
					pBuff2[0]='\0';
				}
				else
				{
					pBuff = pBuff;
					pBuff[_tcslen(pBuff)] = '\0';
					
				}
				snmpVarList.list[0].value.asnValue.address.stream[i] =_ttoi(pBuff);
				pBuff = ++pBuff2;
			}
			break;
		case ASN_OBJECTIDENTIFIER:
	#if defined _UNICODE || defined UNICODE
			memset(sTemp,0,VAR_DATALEN);
			WideCharToMultiByte(CP_ACP,NULL,(WCHAR*)dwAddress,-1,sTemp,sizeof(sTemp),NULL,NULL);
			if(!SnmpMgrStrToOid(sTemp,&snmpVarList.list[0].value.asnValue.object))
	#else
			if(!SnmpMgrStrToOid((TCHAR*)dwAddress,&snmpVarList.list[0].value.asnValue.object))
	#endif
			{
				OutputDebugString(_T("Invalid value Snmp Value Error"));
				SnmpUtilVarBindListFree(&snmpVarList);
				SnmpUtilOidFree(&asnOid);
				return false;
			}
			break;
		case ASN_SEQUENCE:
			snmpVarList.list[0].value.asnValue.sequence.dynamic = TRUE;
			snmpVarList.list[0].value.asnValue.sequence.length = _tcslen((TCHAR*)dwAddress)+1;
			snmpVarList.list[0].value.asnValue.sequence.stream = (UCHAR*) SnmpUtilMemAlloc(_tcslen((TCHAR*)dwAddress) + 1);
	#if defined _UNICODE || defined UNICODE
			memset(sTemp,0,VAR_DATALEN);
			WideCharToMultiByte(CP_ACP,NULL,(WCHAR*)dwAddress,-1,sTemp,sizeof(sTemp),NULL,NULL);
			strncpy((char*)snmpVarList.list[0].value.asnValue.sequence.stream,sTemp,strlen(sTemp)+1);
	#else
			_tcsncpy((TCHAR*)snmpVarList.list[0].value.asnValue.sequence.stream,(TCHAR*)dwAddress,_tcslen((TCHAR*)dwAddress)+1);
	#endif
			break;
		case ASN_OPAQUE:
			snmpVarList.list[0].value.asnValue.arbitrary.dynamic = TRUE;
			snmpVarList.list[0].value.asnValue.arbitrary.length = _tcslen((TCHAR*)dwAddress)+1;
			snmpVarList.list[0].value.asnValue.arbitrary.stream =	 (UCHAR*) SnmpUtilMemAlloc(_tcslen((TCHAR*)dwAddress) + 1);
	#if defined _UNICODE || defined UNICODE
			memset(sTemp,0,VAR_DATALEN);
			WideCharToMultiByte(CP_ACP,NULL,(WCHAR*)dwAddress,-1,sTemp,sizeof(sTemp),NULL,NULL);
			strncpy((char*)snmpVarList.list[0].value.asnValue.arbitrary.stream,sTemp,strlen(sTemp)+1);
	#else
			_tcsncpy((TCHAR*)snmpVarList.list[0].value.asnValue.sequence.stream,(TCHAR*)dwAddress,_tcslen((TCHAR*)dwAddress)+1);
	#endif
			
		case ASN_NULL:
		default:
			_tcscpy((TCHAR*)dwAddress,_T(""));
			SnmpUtilVarBindListFree(&snmpVarList);
			SnmpUtilOidFree(&asnOid);
			OutputDebugString(_T("Error Unsupported Type,Error"));
			return false;
		}
		
		// initiates the set request		
		if(!SnmpMgrRequest(hSession,SNMP_PDU_SET,&snmpVarList,&errorStatus,&errorIndex))
		{
			int error = ::GetLastError();
			OutputDebugString(_T("Snmp Request Failed,Snmp Error"));
			SnmpUtilVarBindListFree(&snmpVarList);
			SnmpUtilOidFree(&asnOid);
			return false;
		}
		if(errorStatus > 0)
		{
			PrintStatusError(errorStatus,pBuff);
			TCHAR *strTemp;
			strTemp = (TCHAR*)malloc(VAR_DATALEN);
			_stprintf(strTemp,_T("Snmp Request Failed\nErrorStatus: %s  ErrorIndex: %d"),pBuff,errorIndex);
			OutputDebugString(strTemp);
			free(strTemp);
			free(pBuff);

			SnmpUtilVarBindListFree(&snmpVarList);
			SnmpUtilOidFree(&asnOid);		
			return false;
		}
		// free the list, to avoid memory leak
		SnmpUtilVarBindListFree(&snmpVarList);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

/* 
int SetRequest(HANDLE hSession,AsnObjectIdentifier &asnOid,TCHAR *asciiStr,INT wType)
{
	TCHAR *pBuff, *pBuff2;
	int i;
	char sTemp[VAR_DATALEN];
	

	SnmpVarBindList snmpVarList;
	
	AsnInteger	errorStatus=0;	// Error type that is returned if encountered
    AsnInteger	errorIndex=0;	// Works with variable above

	snmpVarList.list = NULL;
    snmpVarList.len = 0;

	snmpVarList.len++;
	snmpVarList.list = (SnmpVarBind *)SNMP_realloc(snmpVarList.list, sizeof(SnmpVarBind) *snmpVarList.len); 
	

	// Assigning OID to variable bindings list
	SnmpUtilOidCpy(&snmpVarList.list[0].name,&asnOid);
	snmpVarList.list[0].value.asnType = ASN_NULL;
    	
	snmpVarList.list[0].value.asnType = (INT)(wType);
			
	switch (snmpVarList.list[0].value.asnType)
	{
	case ASN_BITS:
		snmpVarList.list[0].value.asnValue.bits.dynamic = TRUE;
		snmpVarList.list[0].value.asnValue.bits.length = _tcslen(asciiStr);
		snmpVarList.list[0].value.asnValue.bits.stream = (BYTE*)malloc(snmpVarList.list[0].value.asnValue.bits.length*sizeof(TCHAR));
#if defined _UNICODE || defined UNICODE
		memset(sTemp,0,VAR_DATALEN);
		WideCharToMultiByte(CP_ACP,NULL,asciiStr,-1,sTemp,sizeof(sTemp),NULL,NULL);
		strcpy((char*)snmpVarList.list[0].value.asnValue.bits.stream,sTemp);
		//strcpy((char*)snmpVarList.list[0].value.asnValue.bits.stream,sTemp);
#else
		_tcscpy((TCHAR*)snmpVarList.list[0].value.asnValue.bits.stream,asciiStr);
#endif
		break;
	case ASN_OCTETSTRING:
		snmpVarList.list[0].value.asnValue.string.dynamic = TRUE;
		snmpVarList.list[0].value.asnValue.string.length = _tcslen(asciiStr);
		snmpVarList.list[0].value.asnValue.string.stream = (BYTE*)malloc(snmpVarList.list[0].value.asnValue.string.length*sizeof(TCHAR));
#if defined _UNICODE || defined UNICODE
		memset(sTemp,0,VAR_DATALEN);
		WideCharToMultiByte(CP_ACP,NULL,asciiStr,-1,sTemp,sizeof(sTemp),NULL,NULL);
		strcpy((char*)snmpVarList.list[0].value.asnValue.string.stream,sTemp);
		//strcpy((char*)snmpVarList.list[0].value.asnValue.string.stream,sTemp);
#else
		_tcscpy((TCHAR*)snmpVarList.list[0].value.asnValue.string.stream,asciiStr);
#endif
		break;
	case ASN_INTEGER32:
		snmpVarList.list[0].value.asnValue.number = _ttol(asciiStr);
		break;
	case ASN_TIMETICKS:
		snmpVarList.list[0].value.asnValue.ticks = _ttol(asciiStr);
		break;
	case ASN_GAUGE32:
		snmpVarList.list[0].value.asnValue.gauge = _ttol(asciiStr);
		break;
	case ASN_COUNTER32:
		snmpVarList.list[0].value.asnValue.counter = _ttol(asciiStr);
		break;

	case ASN_IPADDRESS:
		pBuff = asciiStr;
		pBuff2 = pBuff;

		snmpVarList.list[0].value.asnValue.address.dynamic = TRUE;
		snmpVarList.list[0].value.asnValue.address.length = 4;
		snmpVarList.list[0].value.asnValue.address.stream = (UCHAR*)SnmpUtilMemAlloc(4);

		for(i=0;i<4;i++)
		{
			pBuff2 = _tcschr(pBuff,_T('.'));

			if(pBuff2)
			{
				pBuff2[0]='\0';
			}
			else
			{
				pBuff = pBuff;
				pBuff[_tcslen(pBuff)] = '\0';
				
			}
			snmpVarList.list[0].value.asnValue.address.stream[i] =_ttoi(pBuff);
			pBuff = ++pBuff2;
		}
		break;
	case ASN_OBJECTIDENTIFIER:
#if defined _UNICODE || defined UNICODE
		memset(sTemp,0,VAR_DATALEN);
		WideCharToMultiByte(CP_ACP,NULL,asciiStr,-1,sTemp,sizeof(sTemp),NULL,NULL);
		if(!SnmpMgrStrToOid(sTemp,&snmpVarList.list[0].value.asnValue.object))
#else
		if(!SnmpMgrStrToOid(asciiStr,&snmpVarList.list[0].value.asnValue.object))
#endif
		{
			OutputDebugString(_T("Invalid value Snmp Value Error"));
			SnmpUtilVarBindListFree(&snmpVarList);
			SnmpUtilOidFree(&asnOid);
			return 1;
		}
		break;
	case ASN_SEQUENCE:
		snmpVarList.list[0].value.asnValue.sequence.dynamic = TRUE;
		snmpVarList.list[0].value.asnValue.sequence.length = _tcslen(asciiStr)+1;
		snmpVarList.list[0].value.asnValue.sequence.stream = (UCHAR*) SnmpUtilMemAlloc(_tcslen(asciiStr) + 1);
#if defined _UNICODE || defined UNICODE
		memset(sTemp,0,VAR_DATALEN);
		WideCharToMultiByte(CP_ACP,NULL,asciiStr,-1,sTemp,sizeof(sTemp),NULL,NULL);		
		strncpy((char*)snmpVarList.list[0].value.asnValue.sequence.stream,sTemp,strlen(sTemp)+1);
#else
		_tcsncpy((TCHAR*)snmpVarList.list[0].value.asnValue.sequence.stream,asciiStr,_tcslen(asciiStr)+1);
#endif
		break;
	case ASN_OPAQUE:
		snmpVarList.list[0].value.asnValue.arbitrary.dynamic = TRUE;
		snmpVarList.list[0].value.asnValue.arbitrary.length = _tcslen(asciiStr)+1;
		snmpVarList.list[0].value.asnValue.arbitrary.stream =	 (UCHAR*) SnmpUtilMemAlloc(_tcslen(asciiStr) + 1);
#if defined _UNICODE || defined UNICODE
		memset(sTemp,0,VAR_DATALEN);
		WideCharToMultiByte(CP_ACP,NULL,asciiStr,-1,sTemp,sizeof(sTemp),NULL,NULL);		
		strncpy((char*)snmpVarList.list[0].value.asnValue.arbitrary.stream,sTemp,strlen(sTemp)+1);
#else
		_tcsncpy((TCHAR*)snmpVarList.list[0].value.asnValue.arbitrary.stream,asciiStr,_tcslen(asciiStr)+1);
#endif
		
	case ASN_NULL:
	default:
		_tcscpy(asciiStr,_T(""));
		SnmpUtilVarBindListFree(&snmpVarList);
		SnmpUtilOidFree(&asnOid);
		OutputDebugString(_T("Error Unsupported Type,Error"));
		return 1;
	}
	
	// initiates the set request
	if(!SnmpMgrRequest(hSession,SNMP_PDU_SET,&snmpVarList,&errorStatus,&errorIndex))
	{
		OutputDebugString(_T("Snmp Request Failed,Snmp Error"));
		SnmpUtilVarBindListFree(&snmpVarList);
		SnmpUtilOidFree(&asnOid);
		return 1;
	}
	if(errorStatus > 0)
	{
		PrintStatusError(errorStatus,pBuff);
		TCHAR *strTemp;
		strTemp = (TCHAR*)malloc(VAR_DATALEN);
		_stprintf(strTemp,_T("Snmp Request Failed\nErrorStatus: %s  ErrorIndex: %d"),pBuff,errorIndex);
		OutputDebugString(strTemp);
		free(strTemp);
		free(pBuff);

		SnmpUtilVarBindListFree(&snmpVarList);
		SnmpUtilOidFree(&asnOid);		
		return 1;
	}

	// free the list, to avoid memory leak
	SnmpUtilVarBindListFree(&snmpVarList);
	return 0;
}
*/

// SNMPOpenSession
//  -- Initialize a session with ZQSNMPManPkg.  A process can initialize multiple sesions
//  with ZQSNMPManPkg.    Access  is thread-safe. 
ZQMANSTATUS SNMPOperOpenSession(TCHAR *pszIpAddress,TCHAR * pszCommunity, HANDLE *hSession,TCHAR *szServiceName)
{
	ZQMANSTATUS manRet = ZQMAN_SUCCESS;
	HANDLE lpMgrSession;
	try
	{
		::EnterCriticalSection(&m_csect);
		LPVOID lpMsgBuf;
		
#if defined _UNICODE || defined UNICODE
		CHAR sTemp[VAR_DATALEN] ={0};
		WideCharToMultiByte(CP_ACP,NULL,pszIpAddress,-1,sTemp,sizeof(sTemp),NULL,NULL);
		CHAR sTemp1[VAR_DATALEN] ={0};
		WideCharToMultiByte(CP_ACP,NULL,pszCommunity,-1,sTemp1,sizeof(sTemp),NULL,NULL);
		lpMgrSession = SnmpMgrOpen(sTemp,sTemp1,1000,3);
#else
		lpMgrSession = SnmpMgrOpen(pszIpAddress,pszCommunity,1000,3);
#endif
		
		if(lpMgrSession == NULL)
		{
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0,NULL );
			OutputDebugString((TCHAR*)lpMsgBuf);
			free(lpMsgBuf);
			manRet =ZQMAN_SESSION_ERROR;
			*hSession = NULL;
			m_handle  = *hSession;
			::LeaveCriticalSection(&m_csect);
			return manRet;
		}
	}
	catch(...)
	{
		manRet =ZQMAN_SESSION_ERROR;
		*hSession = NULL;
		m_handle  = *hSession;
		::LeaveCriticalSection(&m_csect);
		return manRet;
	}
	if ( szServiceName)
	{
		_tcscpy(m_szServiceName,szServiceName);
	    *(m_szServiceName+(_tcsclen(szServiceName))) =0;
	}
	*hSession = lpMgrSession;
	m_handle  = *hSession;
	::LeaveCriticalSection(&m_csect);
	return manRet;
}

// SNMPCloseSession
//  -- Terminate a session held with ZQSNMPManPkg.  This function will close the opened snmp session.

ZQMANSTATUS  SNMPOperCloseSession(HANDLE hSession)
{
	ZQMANSTATUS manRet = ZQMAN_SUCCESS;
	try
	{
		::EnterCriticalSection(&m_csect);
		LPVOID lpMsgBuf;
		BOOL bReturn = SnmpMgrClose(hSession);
		if(!bReturn)
		{
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0,NULL );
			OutputDebugString((TCHAR*)lpMsgBuf);
			free(lpMsgBuf);
			manRet =ZQMAN_SESSION_ERROR;
		}
		::LeaveCriticalSection(&m_csect);
	}
	catch(...)
	{
		manRet =ZQMAN_SESSION_ERROR;
		::LeaveCriticalSection(&m_csect);
    }
	return manRet;
}

// SNMPSetVarValue
//  -- Set the Variant Value by SNMP Protocol with ZQSNMPManPkg.  Access  is thread-safe. 
ZQMANSTATUS  SNMPOperSetVarValue(HANDLE hSession, TCHAR *szOid,DWORD dwAddress,INT wType)
{
	ZQMANSTATUS manRet = ZQMAN_SUCCESS;
	try
	{
		::EnterCriticalSection(&m_csect);
		TCHAR szNewOid[VAR_OIDLEN];
		memset(szNewOid,0,sizeof(szNewOid));
		{
		// add by dony 20070122 for add variable's name and type in variable's data
#ifdef	VARNAMEIN_MODE
		if (wType == ZQMAN_INT)
		{
			bool b = IsNumber((TCHAR*)dwAddress);
			if (  !b  ) //不是整数
			{
				manRet = ZQMAN_SETVAR_INVALID_DATATYPE;
				::LeaveCriticalSection(&m_csect);
				return manRet;
			}
			wType = ZQMAN_STR;
		}
		if ( wType == ZQMAN_FLOAT)
		{
			TCHAR *pos = NULL;
			pos  = _tcsrchr((TCHAR*)dwAddress, _T('.'));
			int ilen = _tcsclen((TCHAR*)dwAddress) - _tcsclen(pos);
			TCHAR szTemp[20]={0};
			_tcsncpy(szTemp,(TCHAR*)dwAddress,ilen);
			bool b = (IsNumber(szTemp) && ( IsNumber(pos+1)));
			if ( !b) // 不是浮点数
			{
				manRet = ZQMAN_SETVAR_INVALID_DATATYPE;
				::LeaveCriticalSection(&m_csect);
				return manRet;
			}
			wType = ZQMAN_STR;
		}
		
		string strOID,strVarName;
		bool b = ( *(szOid+1) >='0') && ( *(szOid+1) <='9' );
		if ( !b ) // 如果输入的变量名,则进行变量名到OID的转换
		{

#if defined _UNICODE || defined UNICODE
			  char sTemp[VAR_OIDLEN];
				memset(sTemp,0,sizeof(sTemp));
				WideCharToMultiByte(CP_ACP,NULL,szOid,-1,sTemp,sizeof(sTemp),NULL,NULL);
				strVarName = sTemp;
#else
				strVarName = szOid;
#endif
				if ( m_OIDVarNameMap.empty())
				{
					manRet = ZQMAN_SETVAR_INVALID_VARNAME;
					::LeaveCriticalSection(&m_csect);
					return manRet;
				}
				else
				{
					strVarName = m_strServiceName + "." + strVarName;
					strOID = m_OIDVarNameMap[strVarName];
#if defined _UNICODE || defined UNICODE
				WCHAR wszOid[VAR_OIDLEN];
				memset(wszOid,0,sizeof(wszOid));
				MultiByteToWideChar(
							CP_ACP,         // code page
							0,              // character-type options
							strOID.c_str(),        // address of string to map
							strlen(strOID.c_str()),      // number of bytes in string
							wszOid,
							VAR_OIDLEN);             // size of buffer);
				_stprintf(szNewOid,_T("%s"),wszOid);
#else
				sprintf(szNewOid,"%s",strOID.c_str());
#endif
				}
			}
			else
			{
				_stprintf(szNewOid,_T("%s"),szOid);
			}
#else
			_stprintf(szNewOid,_T("%s"),szOid);
#endif
			// add by dony 20070122 for add variable's name and type in variable's data
		

			AsnObjectIdentifier asnOid;
#if defined _UNICODE || defined UNICODE
			CHAR sTemp[VAR_DATALEN] ={0};
			WideCharToMultiByte(CP_ACP,NULL,szNewOid,-1,sTemp,sizeof(sTemp),NULL,NULL);
			if(!SnmpMgrStrToOid(sTemp, &asnOid))
#else
			if(!SnmpMgrStrToOid(szNewOid, &asnOid))
#endif			
			{
				manRet =ZQMAN_SETVAR_INVALIDOID;
				OutputDebugString(_T("Invalid Oid,Error"));
				SnmpUtilOidFree(&asnOid);
				::LeaveCriticalSection(&m_csect);
				return manRet;
			}
		//	if(!SetRequest(hSession,asnOid,szVarValue,wType))
			if(SetRequest(hSession,asnOid,dwAddress,wType))
			{
				manRet =ZQMAN_SUCCESS;
				OutputDebugString(_T("Successfully Set"));
			}
			else
			{
				SnmpUtilOidFree(&asnOid);
				manRet = ZQMAN_SETVAR_ERROR_SETREQUEST;
				::LeaveCriticalSection(&m_csect);
				return manRet;
			}
			SnmpUtilOidFree(&asnOid);
		}
		::LeaveCriticalSection(&m_csect);
	}

	catch(...)
	{
		manRet =ZQMAN_SETVAR_ERROR;
		::LeaveCriticalSection(&m_csect);
	}
	return manRet;
}
/*
ZQMANSTATUS  SNMPSetVarValue(HANDLE hSession, TCHAR *szOid,TCHAR *szVarValue,INT wType)
{
	ZQMANSTATUS manRet = ZQMAN_SUCCESS;
	try
	{
		::EnterCriticalSection(&m_csect);
		{
			AsnObjectIdentifier asnOid;
#if defined _UNICODE || defined UNICODE
			CHAR sTemp[VAR_DATALEN] ={0};
			WideCharToMultiByte(CP_ACP,NULL,szOid,-1,sTemp,sizeof(sTemp),NULL,NULL);
			if(!SnmpMgrStrToOid(sTemp, &asnOid))
#else
			if(!SnmpMgrStrToOid(szOid, &asnOid))
#endif			
			{
				manRet =ZQMAN_SETVAR_ERROR;
				OutputDebugString(_T("Invalid Oid,Error"));
				SnmpUtilOidFree(&asnOid);
				::LeaveCriticalSection(&m_csect);
				return manRet;
			}
			if(!SetRequest(hSession,asnOid,szVarValue,wType))
			{
				manRet =ZQMAN_SUCCESS;
				OutputDebugString(_T("Successfully Set"));
			}
			else
			{
				SnmpUtilOidFree(&asnOid);
				manRet = ZQMAN_SETVAR_ERROR;
				::LeaveCriticalSection(&m_csect);
				return manRet;
			}
			SnmpUtilOidFree(&asnOid);
		}
		::LeaveCriticalSection(&m_csect);
	}

	catch(...)
	{
		manRet =ZQMAN_SETVAR_ERROR;
		::LeaveCriticalSection(&m_csect);
	}
	return manRet;
}
*/

// SNMPGetVarValue
//  -- Get the Variant Value by SNMP Protocol with ZQSNMPManPkg.  Access  is thread-safe. 
ZQMANSTATUS  SNMPOperGetVarValue(HANDLE hSession,TCHAR *szOid,TCHAR *ResultVarType,TCHAR *ResultReadWrite,TCHAR *ResultValue)
{
	ZQMANSTATUS manRet = ZQMAN_SUCCESS;
	try
	{
		TCHAR szNewOid[VAR_OIDLEN];
		memset(szNewOid,0,sizeof(szNewOid));

		char sTemp[VAR_DATALEN] ={0};
		::EnterCriticalSection(&m_csect);
		{
		// add by dony 20070122 for add variable's name and type in variable's data
#ifdef	VARNAMEIN_MODE
			string strOID,strVarName;
			bool b = ( *(szOid+1) >='0') && ( *(szOid+1) <='9' );
			if ( !b ) // 如果输入的变量名,则进行变量名到OID的转换
			{
#if defined _UNICODE || defined UNICODE
				memset(sTemp,0,sizeof(sTemp));
				WideCharToMultiByte(CP_ACP,NULL,szOid,-1,sTemp,sizeof(sTemp),NULL,NULL);
				strVarName = sTemp;
#else
				strVarName = szOid;
#endif

				if ( m_OIDVarNameMap.empty())
				{
					_stprintf(ResultVarType,_T("No Find Var Type "));
					_stprintf(ResultValue,_T("szOid is Error,so return null"));
					manRet = ZQMAN_GETVAR_INVALID_VARNAME;
					::LeaveCriticalSection(&m_csect);
					return manRet;
				}
				else
				{
					int isize = m_OIDVarNameMap.size();
					strVarName = m_strServiceName + "." + strVarName;
					strOID = m_OIDVarNameMap[strVarName];
#if defined _UNICODE || defined UNICODE
				WCHAR wszOid[VAR_OIDLEN];
				memset(wszOid,0,sizeof(wszOid));
				MultiByteToWideChar(
							CP_ACP,         // code page
							0,              // character-type options
							strOID.c_str(),        // address of string to map
							strlen (strOID.c_str()),      // number of bytes in string
							wszOid,
							VAR_OIDLEN);             // size of buffer);
				
				_stprintf(szNewOid,_T("%s"),wszOid);
#else
				sprintf(szNewOid,"%s",strOID.c_str());
#endif
				}
			}
			else
			{
				_stprintf(szNewOid,_T("%s"),szOid);
			}
#else
			_stprintf(szNewOid,_T("%s"),szOid);
#endif
			// add by dony 20070122 for add variable's name and type in variable's data
			AsnObjectIdentifier asnOid;

#if defined _UNICODE || defined UNICODE
			memset(sTemp,0,sizeof(sTemp));
			WideCharToMultiByte(CP_ACP,NULL,szNewOid,-1,sTemp,sizeof(sTemp),NULL,NULL);
			if(!SnmpMgrStrToOid(sTemp, &asnOid))
#else
			if(!SnmpMgrStrToOid(szNewOid, &asnOid))
#endif
			{
				manRet = ZQMAN_GETVAR_INVALIDOID;
				_stprintf(ResultVarType,_T("No Find Var Type "));
				_stprintf(ResultValue,_T("Invalid Oid,Error"));
				OutputDebugString(_T("Invalid Oid,Error"));
				SnmpUtilOidFree(&asnOid);
				::LeaveCriticalSection(&m_csect);
				return manRet;
			}
			if(!GetRequest(hSession,asnOid,ResultVarType,ResultReadWrite,ResultValue))
			{
				SnmpUtilOidFree(&asnOid);
				_stprintf(ResultVarType,_T("No Find Var Type "));
				_stprintf(ResultValue,_T("Get Request Error"));
				manRet = ZQMAN_GETVAR_ERROR_GETREQUEST;
				::LeaveCriticalSection(&m_csect);
				return manRet;
			}
			SnmpUtilOidFree(&asnOid);
		}
		::LeaveCriticalSection(&m_csect);
	}
	catch(...)
	{
		int iErrorCode =::GetLastError();
		manRet =ZQMAN_GETVAR_ERROR;
		::LeaveCriticalSection(&m_csect);
	}
	return manRet;
}

// SNMPGetAllVarValue
//  -- Get the ALL Variant Value by the input oid by SNMP Protocol with ZQSNMPManPkg.  Access  is thread-safe. 
ZQMANSTATUS   SNMPOperGetAllVarValue(HANDLE hSession,TCHAR *szOid,int *iCount,VARDATAS** pReturnData)
{
	ZQMANSTATUS manRet = ZQMAN_SUCCESS;
	try
	{
		TCHAR szDataTemp[VAR_DATALEN];
		TCHAR szDataTemp1[VAR_DATALEN];
		WCHAR wszOidTemp[VAR_OIDLEN];
		int iLen;
		
		memset(szDataTemp1,0,sizeof(szDataTemp1));
		memset(wszOidTemp,0,sizeof(wszOidTemp));
		memset(szDataTemp,0,sizeof(szDataTemp));
		TCHAR *pos = NULL;
		TCHAR *pos1 = NULL;
		char *sTemp = NULL;
		string strVarOid,strVarName,strTemp;

		::EnterCriticalSection(&m_csect);
		{
			AsnObjectIdentifier asnOid,asnOidTemp;
			
#if defined _UNICODE || defined UNICODE
			CHAR sTemp1[VAR_DATALEN] ={0};
			WideCharToMultiByte(CP_ACP,NULL,szOid,-1,sTemp1,sizeof(sTemp1),NULL,NULL);
			if(!SnmpMgrStrToOid(sTemp1, &asnOid))
#else
			if(!SnmpMgrStrToOid(szOid, &asnOid))
#endif
			{
				manRet = ZQMAN_GETALLVAR_INVALIDOID;
				OutputDebugString(_T("Invalid Oid,Error"));
				SnmpUtilOidFree(&asnOid);
				::LeaveCriticalSection(&m_csect);
				return manRet;
			}
			TCHAR *asciiStr, *tmpStr;
			SnmpVarBindList snmpVarList;
			AsnInteger  errorStatus = 0;
			AsnInteger  errorIndex = 0;
			
			snmpVarList.list = NULL;
			snmpVarList.len  = 0;

			snmpVarList.len ++;
			snmpVarList.list = (SnmpVarBind*)SNMP_realloc(snmpVarList.list,sizeof(SnmpVarBind)*snmpVarList.len);

			// Assigning OID to variable bindings list
			SnmpUtilOidCpy(&snmpVarList.list[0].name,&asnOid);
			snmpVarList.list[0].value.asnType = NULL;

			// get the count;
			int iCountTmp =0;

			for (;;)
			{
				// one by one ge the variables
				if(!SnmpMgrRequest(hSession,SNMP_PDU_GETNEXT,&snmpVarList,&errorStatus,&errorIndex))
				{
					SnmpUtilVarBindListFree(&snmpVarList);
					SnmpUtilOidFree(&asnOid);
					::LeaveCriticalSection(&m_csect);
					manRet = ZQMAN_GETALLVAR_ERROR_REQUEST;
					return manRet;
				}
				if ( ( errorStatus == 0 ) && snmpVarList.list[0].value.asnType != ASN_OBJECTIDENTIFIER )
				{
					// Prepare for the next iteration.  Make sure returned oid is
					// preserved and the returned value is freed.
					if ( snmpVarList.list[0].value.asnValue.string.length>0)
					{
						iCountTmp ++;
					}
					SnmpUtilOidCpy(&asnOidTemp, &snmpVarList.list[0].name);
					SnmpUtilVarBindFree(&snmpVarList.list[0]);

					SnmpUtilOidCpy(&snmpVarList.list[0].name, &asnOidTemp);
					snmpVarList.list[0].value.asnType = ASN_NULL;
					SnmpUtilOidFree(&asnOidTemp);
					
				}
				if(errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME||SnmpUtilOidNCmp(&snmpVarList.list[0].name,&asnOid,asnOid.idLength))
					break;
			}

			if ( iCountTmp > 0 )
			{
				*iCount = iCountTmp;
				*pReturnData = new VARDATAS[*iCount];
	
				for ( int i = 0; i < (*iCount); i ++ )
				{
					(*pReturnData)[i].ResultVarType   = (TCHAR*)malloc(VAR_TYPELEN);
					(*pReturnData)[i].ResultVarOID    = (TCHAR*)malloc(VAR_OIDLEN);
					(*pReturnData)[i].ResultValue     = (TCHAR*)malloc(VAR_DATALEN);
					(*pReturnData)[i].ResultReadWrite = (TCHAR*)malloc(VAR_TYPELEN);
				}
			}
			else
			{
				manRet = ZQMAN_GETALLVAR_NOCOUNT;
				OutputDebugString(_T("Invalid Oid,Error"));
				SnmpUtilOidFree(&asnOid);
				::LeaveCriticalSection(&m_csect);
				return manRet;
			}
			
			// ReAssigning OID to variable bindings list
			snmpVarList.list = NULL;
			snmpVarList.len  = 0;

			snmpVarList.len ++;
			snmpVarList.list = (SnmpVarBind*)SNMP_realloc(snmpVarList.list,sizeof(SnmpVarBind)*snmpVarList.len);

			SnmpUtilOidCpy(&snmpVarList.list[0].name,&asnOid);
			snmpVarList.list[0].value.asnType = NULL;

			iCountTmp = 0;
			//
			for (;;)
			{
				// one by one ge the variables
				if (!SnmpMgrRequest(hSession,SNMP_PDU_GETNEXT,&snmpVarList,&errorStatus,&errorIndex))
				{
					asciiStr = (TCHAR*)malloc(sizeof(TCHAR)*VAR_DATALEN);
					PrintStatusError(errorStatus,tmpStr);

					_stprintf(asciiStr,_T("Snmp Request Failed\nErrorStatus: %s  ErrorIndex: %d"),tmpStr,errorIndex);
					OutputDebugString(asciiStr);
					free(asciiStr);
					free(tmpStr);		

					SnmpUtilVarBindListFree(&snmpVarList);
					SnmpUtilOidFree(&asnOid);
					::LeaveCriticalSection(&m_csect);
					manRet = ZQMAN_GETALLVAR_ERROR_REREQUEST;
					return manRet;
				}
				if(errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME||SnmpUtilOidNCmp(&snmpVarList.list[0].name,&asnOid,asnOid.idLength))
					break;

				if(errorStatus > 0)
				{
					asciiStr = (TCHAR*)malloc(sizeof(TCHAR)*VAR_DATALEN);
					PrintStatusError(errorStatus,tmpStr);
					_stprintf(asciiStr,_T("ErrorStatus: %s  ErrorIndex: %d"),tmpStr,errorIndex);
					OutputDebugString(asciiStr);
					free(asciiStr);
					free(tmpStr);
					break;
				}
				else
				{
					char *szOidString = NULL;
					if ( snmpVarList.list[0].name.idLength )
					{
						szOidString = (char *) SnmpUtilMemAlloc(snmpVarList.list[0].name.idLength * 5 );
						if(szOidString)
						{
							UINT i;
							char szBuf[VAR_TYPELEN];
							strcpy(szOidString,".");
							for(i = 0; i < snmpVarList.list[0].name.idLength; i++)
							{
								strcat(szOidString,_itoa(snmpVarList.list[0].name.ids[i],szBuf,10));
								if(i < snmpVarList.list[0].name.idLength-1)
									strcat(szOidString,("."));
							}
						}
					}
										
				
				   memset(szDataTemp,0,sizeof(szDataTemp));
				   memset(szDataTemp1,0,sizeof(szDataTemp1));
	#if defined _UNICODE || defined UNICODE
					MultiByteToWideChar(
								CP_ACP,         // code page
								0,              // character-type options
								szOidString,        // address of string to map
								strlen (szOidString),      // number of bytes in string
								wszOidTemp,
								VAR_OIDLEN);             // size of buffer);
					wcscpy((*pReturnData)[iCountTmp].ResultVarOID,wszOidTemp);
					
					
					sTemp = (char*)malloc(sizeof(char)*VAR_DATALEN);
					sTemp = SNMP_AnyToStr(&snmpVarList.list[0].value);
					MultiByteToWideChar(
								CP_ACP,         // code page
								0,              // character-type options
								sTemp,        // address of string to map
								strlen (sTemp),      // number of bytes in string
								szDataTemp,
								VAR_DATALEN);             // size of buffer);
	#else
					sprintf(szDataTemp,"%s",SNMP_AnyToStr(&snmpVarList.list[0].value));
					strcpy((*pReturnData)[iCountTmp].ResultVarOID,szOidString);
	#endif
					*(szDataTemp+(_tcsclen(szDataTemp))) = 0;
													
					// add by dony 20070122 for add variable's name and type in variable's data
	#ifdef	VARNAMEIN_MODE
					pos  = _tcsrchr(szDataTemp, _T('@'));
					if ( pos )
					{
						if ( *(pos+1) =='1' ) //整数类型
						{
							_tcscpy((*pReturnData)[iCountTmp].ResultVarType,varTypeData[2]);
						}
						else if ( *(pos+1) =='2' ) //字符串类型
						{
							_tcscpy((*pReturnData)[iCountTmp].ResultVarType,varTypeData[4]);
						}
						else if ( *(pos+1) =='3' ) //浮点数类型
						{
							_tcscpy((*pReturnData)[iCountTmp].ResultVarType,varTypeData[5]);
						}
						
						if ( *(pos+3) =='0') 
						{
							_tcscpy((*pReturnData)[iCountTmp].ResultReadWrite,_T("ReadOnly"));
						}
						else if ( *(pos+3) =='1' )
						{
							_tcscpy((*pReturnData)[iCountTmp].ResultReadWrite,_T("WriteOnly"));
						}
						else
						{
							_tcscpy((*pReturnData)[iCountTmp].ResultReadWrite,_T("ReadWrite"));
						}


						iLen = _tcsclen(szDataTemp);
						for ( int i = 0; i < iLen; i ++ )
						{
							if ( *(szDataTemp+i) =='.') 
							{
								break;
							}
						}
						
						iLen = iLen - _tcslen(pos) - i;
						_tcsncpy(szDataTemp1,szDataTemp+i+1,iLen);
						_stprintf((*pReturnData)[iCountTmp].ResultValue,_T("%s%s"),szDataTemp1,pos+5);
//						_tcscpy((*pReturnData)[iCountTmp].ResultValue,szDataTemp); // 因为数据的格式是:变量名@1(or2,or3)+变量的读写属性:变量数据
					}
					strVarOid = szOidString;
					strTemp = SNMP_AnyToStr(&snmpVarList.list[0].value);
					GetStringLeftAtChar(strTemp,'@',strVarName);
					GetStringLeftAtChar(strTemp,'.',m_strServiceName);
					m_OIDVarNameMap[strVarName] = strVarOid;
//					int isize = m_OIDVarNameMap.size();

	#else
					_tcscpy((*pReturnData)[iCountTmp].ResultValue,szDataTemp);
					_tcscpy((*pReturnData)[iCountTmp].ResultVarType,varTypeData[snmpVarList.list[0].value.asnType]);
	#endif
					
					if ( szOidString)
					{
						SnmpUtilMemFree(szOidString);
					}
					if ( sTemp)
					{
						free(sTemp);
					}
					sTemp = NULL;
					
				}
				// Prepare for the next iteration.  Make sure returned oid is
				// preserved and the returned value is freed.
				SnmpUtilOidCpy(&asnOidTemp, &snmpVarList.list[0].name);

				SnmpUtilVarBindFree(&snmpVarList.list[0]);

				SnmpUtilOidCpy(&snmpVarList.list[0].name, &asnOidTemp);
				snmpVarList.list[0].value.asnType = ASN_NULL;

				SnmpUtilOidFree(&asnOidTemp);
				iCountTmp ++;
			}
			*iCount = iCountTmp;

			SnmpUtilVarBindListFree(&snmpVarList);
			SnmpUtilOidFree(&asnOid);
		}
		::LeaveCriticalSection(&m_csect);
	}
	catch(...)
	{
		manRet =ZQMAN_GETALLVAR_ERROR;
		::LeaveCriticalSection(&m_csect);
	}
	return manRet;

}

/*
// get the company OID
bool GetCompanyOID()
{
	ZQ::common::ComInitializer comInier;
	ZQ::common::XMLPrefDoc xmlDoc(comInier);

	// open xml doc
	bool ret;
#if defined _UNICODE || defined UNICODE
	char sTemp[VAR_DATALEN] ={0};
	WideCharToMultiByte(CP_ACP,NULL,m_szXMLFileName,-1,sTemp,sizeof(sTemp),NULL,NULL);
	ret = xmlDoc.open(sTemp);
 #else
	ret = xmlDoc.open(m_szXMLFileName);
#endif
	if(!ret)
	{
		return false;
	}

	ZQ::common::PrefGuard root = xmlDoc.root();

	ZQ::common::PrefGuard pAgentPref;

	pAgentPref.pref(root.pref()->firstChild());
	while(pAgentPref.valid())
	{
		// get node name 
		char szNodeName[VAR_DATALEN]={0};
		pAgentPref.pref()->name(szNodeName);
		
		if (_stricmp(szNodeName,INFODATA) != 0 )
		{
			pAgentPref.pref(root.pref()->nextChild());
			continue;
		}
		char szName[VAR_DATALEN]={0};
		pAgentPref.pref()->get(SUPPORDOID,szName);
		
		// turn to next loop
		pAgentPref.pref(root.pref()->nextChild());
	}
	return true;
}

// get the XML File Name from Register.
bool GetRegistryValues()
{
	DWORD dwNumber;
	HANDLE hCfg = CFG_INITEx(_T("ZQSnmpExtension"), &dwNumber, _T("ITV"),FALSE);
	if (NULL == hCfg)
	{
		return false;
	}
	DWORD dwSize = sizeof(DWORD);
	DWORD dwType;
	
	TCHAR XMLConfigFile[VAR_DATALEN] ={0};
	dwSize = VAR_DATALEN * sizeof(TCHAR);
	// get .xml file path
    CFGSTATUS cStatus = CFG_GET_VALUE(hCfg, (TCHAR*)_T("XMLConfigFilePath"), (BYTE*)XMLConfigFile, &dwSize, &dwType,FALSE);
	if (CFG_SUCCESS != cStatus) 
	{
		CFG_TERM(hCfg);
		return false;
	}
	ExpandEnvironmentStrings(XMLConfigFile, XMLConfigFile, VAR_DATALEN * sizeof(TCHAR));
	_tcscpy(m_szXMLFileName,XMLConfigFile);
	*(m_szXMLFileName+(_tcsclen(XMLConfigFile))) =0;
	CFG_TERM(hCfg);
	return TRUE;
}
*/


#ifdef __cplusplus
} // end extern "C"
#endif
