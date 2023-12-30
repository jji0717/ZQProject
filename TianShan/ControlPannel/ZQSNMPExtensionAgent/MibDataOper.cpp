// MibDataOper.cpp: implementation of the CMibDataOper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MibDataOper.h"
extern CMibDataOper    gMIBData; 
extern SnmpVariable    gItvOidServicesPrefix;

#ifdef FILELOGMODE
	extern  ZQ::common::FileLog * m_pReporter;
#else
	extern MCB_t *         hMClog; 
#endif

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
#pragma comment(lib, "ZQSnmpManPkg" VERLIBEXT)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
UINT ProcessLeafNode(
	BYTE				bRequestType,	// IN     SNMP operation to perform 
	MibDescriptor*		pMibPtr,		// IN     Pointer to the MIB variable in the MibVars array 
	RFC1157VarBind*		pVarBind );		// IN OUT Variable binding that references the MIB variable 

UINT ProcessVariableBinding(
	BYTE				RequestType,		// IN     SNMP operation to perform 
	RFC1157VarBind*		pVarBind);			// IN OUT Variable binding to process 

UINT ProcessIntegerFunc(
	BYTE				bRequestType,		// IN     SNMP operation to perform  
	MibDescriptor*		pMibPtr,			// IN     Pointer to the MIB variable in the MibVars array  
	RFC1157VarBind*		pVarBind );			// IN OUT Variable binding that references the MIB variable 

UINT ProcessStringFunc(
	BYTE			  bRequestType,		// IN     SNMP operation to perform 
	MibDescriptor*  pMibPtr,			// IN OUT Pointer to the MIB variable in the MibVars array 
	RFC1157VarBind* pVarBind );	    	// IN     Variable binding that references the MIB variable 

char * SNMP_AnyToStr(AsnObjectSyntax *sAny);

////////////////////////////////////////////////////////////////////////////////////////////
//
// Process a Leaf Node (MIB variable)
//
// This function performs a Get, GetNExt, or Set operation on a specific MIB
// variable defined in the MibVars array. The ACCESS mode of the variable is
// checked, and if allowable, the operation is performed by calling the
// variable's processing function. The resulting value--or error code--is then
// returned in the variable binding.
//
// Returns the SNMP ErrorStatus of the operation.
//
UINT ProcessLeafNode(
	BYTE				bRequestType,	// IN     SNMP operation to perform 
	MibDescriptor*		pMibPtr,		// IN     Pointer to the MIB variable in the MibVars array 
	RFC1157VarBind*		pVarBind )		// IN OUT Variable binding that references the MIB variable 
{
 
	TCHAR szMsg[MAL_LENTH]={0};
    UINT nErrorStatus = SNMP_ERRORSTATUS_NOERROR;
    switch( bRequestType )
    {
        case MIB_ACTION_GET:
	    case MIB_ACTION_GETNEXT:
            // Check if the MIB variable has a readable ACCESS mode 
            if ( pMibPtr->Access != MIB_ACCESS_READONLY )
            {
                if ( pMibPtr->Access != MIB_ACCESS_READWRITE )
                {
                    nErrorStatus = SNMP_ERRORSTATUS_NOACCESS;
                    break;
                }
            }
            // Call the variable's processing function to GET the value 
			if ( pMibPtr->pMibFunc != NULL )
			{
				nErrorStatus = (*pMibPtr->pMibFunc)( MIB_ACTION_GET, pMibPtr, pVarBind );
				if ( nErrorStatus != SNMP_ERRORSTATUS_NOERROR )
					break;
			}
			else
			{
				nErrorStatus = SNMP_ERRORSTATUS_GENERR;
				break;
			}

			// Check if there were any problems in GETting the value 
            // Get the value from the MibVars array and stuff it into the variable binding 
            pVarBind->value.asnType = pMibPtr->GetType();
			SME_STATUS	Result;
			
			
            switch (pVarBind->value.asnType)
            {
                case ASN_INTEGER:
					Result = pMibPtr->GetValue(&pVarBind->value.asnValue.number);
					break;

				case ASN_UNSIGNED32:
					Result = pMibPtr->GetValue(&pVarBind->value.asnValue.unsigned32);
					pVarBind->value.asnType = ASN_COUNTER32;
					break;

				case ASN_RFC1155_COUNTER:
					Result = pMibPtr->GetValue(&pVarBind->value.asnValue.counter);
					break;

                case ASN_RFC1155_GAUGE:
					Result = pMibPtr->GetValue(&pVarBind->value.asnValue.gauge);
					break;

                case ASN_RFC1155_TIMETICKS:
					Result = pMibPtr->GetValue(&pVarBind->value.asnValue.ticks);
					break;

                case ASN_OCTETSTRING:
					{
						// (re)allocate memory for the OCTET STRING 
						AsnOctetString*		pValue=NULL;

						Result = pMibPtr->GetValue(&pValue);
						if (pValue)
						{
							pVarBind->value.asnValue.string.length = pValue->length;
 
							if ( ( pVarBind->value.asnValue.string.stream = (BYTE*)
							  SnmpUtilMemAlloc(  
								pVarBind->value.asnValue.string.length * sizeof( BYTE ))) == NULL )
							{
								// Can't allocate memory for the string? Then it's a genErr 
								nErrorStatus = SNMP_ERRORSTATUS_GENERR;
								break;
							}
							// Copy the OCTET STRING value from pValue which came from the input
							// pMibPtr variable.
							memcpy(pVarBind->value.asnValue.string.stream, 
								  pValue->stream, 
								  pVarBind->value.asnValue.string.length );
							pVarBind->value.asnValue.string.dynamic = TRUE;
						}
						else
						{
							pVarBind->value.asnValue.string.length = 0;
							pVarBind->value.asnValue.string.stream = 0;
							pVarBind->value.asnValue.string.dynamic = FALSE;
						}
						break;
					}
				case ASN_BITS:
					{
						// (re)allocate memory for the OCTET STRING 
						AsnBits*		pValue=NULL;

						Result = pMibPtr->GetValue(&pValue);
						if (pValue)
						{
							pVarBind->value.asnValue.bits.length = pValue->length;
 
							if ( ( pVarBind->value.asnValue.bits.stream = (BYTE*)
							  SnmpUtilMemAlloc(  
								pVarBind->value.asnValue.bits.length * sizeof( BYTE ))) == NULL )
							{
								// Can't allocate memory for the string? Then it's a genErr 
								nErrorStatus = SNMP_ERRORSTATUS_GENERR;
								break;
							}
							// Copy the OCTET STRING value from pValue which came from the input
							// pMibPtr variable.
							memcpy(pVarBind->value.asnValue.bits.stream, 
								  pValue->stream, 
								  pVarBind->value.asnValue.bits.length );
							pVarBind->value.asnValue.bits.dynamic = TRUE;
						}
						else
						{
							pVarBind->value.asnValue.bits.length = 0;
							pVarBind->value.asnValue.bits.stream = 0;
							pVarBind->value.asnValue.bits.dynamic = FALSE;
						}
						break;
					}


				case ASN_OBJECTIDENTIFIER:
					nErrorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
					break;

                default:

                    nErrorStatus = SNMP_ERRORSTATUS_GENERR;
            }
			_stprintf(szMsg,_T("Get Package of Var  name =%s"),pMibPtr->pszStorageName);
			
			#ifdef FILELOGMODE
				(*m_pReporter)(ZQ::common::Log::L_INFO,szMsg);
			#else		
				MClog(hMClog, 1,szMsg);
			#endif
			

			// add by dony 20070122 for add variable's name in the variable's Data
			break;
        case MIB_ACTION_SET:
            // Check if the MIB variable has a writeable ACCESS mode 
            if ( pMibPtr->Access != MIB_ACCESS_WRITEONLY )
            {
                if ( pMibPtr->Access != MIB_ACCESS_READWRITE )
                {
					// Variable is not writeable. SNMP_ERRORSTATUS_NOSUCHNAME is also a possible ErrorStatus value for this error 
                    nErrorStatus = SNMP_ERRORSTATUS_READONLY;
                    break;
                }
			}

            // Check if the value is the proper data type 
            if (pMibPtr->GetType() != pVarBind->value.asnType) 
			{
				if (pMibPtr->GetType() != ASN_UNSIGNED32 && pVarBind->value.asnType != ASN_INTEGER)
				{
					// VarBind value type does not match the type of this variable specified the MIB 
	                nErrorStatus = SNMP_ERRORSTATUS_BADVALUE;	
		            break;
				}
            }

			// switch on type in order to check ranges
            switch ( pMibPtr->GetType() )
	        {
				case ASN_OCTETSTRING:
					// Check that the length of the OCTET STRING is within range 
					if (pMibPtr->MaxVal != 0)
					{
						// Is the new string too long or too short? 
						if (pVarBind->value.asnValue.string.length > (UINT) (pMibPtr->MaxVal) ||
							pVarBind->value.asnValue.string.length < (UINT) (pMibPtr->MinVal) )
						{
							nErrorStatus = SNMP_ERRORSTATUS_BADVALUE;
    					}
					}
					break;
				case ASN_BITS:
					if (pMibPtr->MaxVal != 0)
					{
						// Is the new string too long or too short? 
						if (pVarBind->value.asnValue.bits.length > (UINT) (pMibPtr->MaxVal) ||
							pVarBind->value.asnValue.bits.length < (UINT) (pMibPtr->MinVal) )
						{
							nErrorStatus = SNMP_ERRORSTATUS_BADVALUE;
    					}
					}
					break;
				case ASN_INTEGER:
					if ( pMibPtr->MaxVal != 0 )
					{	// Check the range of the new value 
						if (pVarBind->value.asnValue.number > (AsnInteger) pMibPtr->MaxVal ||
							pVarBind->value.asnValue.number < (AsnInteger) pMibPtr->MinVal )
						{
							nErrorStatus = SNMP_ERRORSTATUS_BADVALUE;
						}
					}
					break;

				case ASN_UNSIGNED32:
					if ( pMibPtr->MaxVal != 0 )
					{	// Check the range of the new value 
						if ( pVarBind->value.asnValue.unsigned32 > (AsnUnsigned32) pMibPtr->MaxVal ||
							 pVarBind->value.asnValue.unsigned32 < (AsnUnsigned32) pMibPtr->MinVal )
						{
							nErrorStatus = SNMP_ERRORSTATUS_BADVALUE;
						}
					}
					break;

			    case ASN_RFC1155_COUNTER:
					if ( pMibPtr->MaxVal != 0 )
					{
						if ( pVarBind->value.asnValue.counter > (AsnCounter) pMibPtr->MaxVal ||
							 pVarBind->value.asnValue.counter < (AsnCounter) pMibPtr->MinVal )
						{
							nErrorStatus = SNMP_ERRORSTATUS_BADVALUE;
						}
					}
					break;

				case ASN_RFC1155_GAUGE:
   	  				if ( pMibPtr->MaxVal != 0 )
					{
						if ( pVarBind->value.asnValue.gauge > (AsnGauge) pMibPtr->MaxVal ||
							 pVarBind->value.asnValue.gauge < (AsnGauge) pMibPtr->MinVal )
						{
							nErrorStatus = SNMP_ERRORSTATUS_BADVALUE;
						}
					}
					break;

				case ASN_RFC1155_TIMETICKS:
			 		if ( pMibPtr->MaxVal != 0 )
					{
						if ( pVarBind->value.asnValue.ticks > (AsnTimeticks) pMibPtr->MaxVal ||
		 					 pVarBind->value.asnValue.ticks < (AsnTimeticks) pMibPtr->MinVal )
						{
							nErrorStatus = SNMP_ERRORSTATUS_BADVALUE;
						}
					}
					break;
			}

			// Call the variable's processing function to SET the value  add by dony 20061116
			if ( pMibPtr->pMibFunc != NULL )
			{
				nErrorStatus = (*pMibPtr->pMibFunc)( MIB_ACTION_SET, pMibPtr, pVarBind );
				if ( nErrorStatus != SNMP_ERRORSTATUS_NOERROR )
					break;
			}
			else
			{

				nErrorStatus = SNMP_ERRORSTATUS_GENERR;
				break;
			}
			// Call the variable's processing function to SET the value  add by dony 20061116

            
			break;
        default:
            nErrorStatus = SNMP_ERRORSTATUS_GENERR;
			break;
    }
    return( nErrorStatus );
}

//
//
// Process and resolve a single variable binding.
//
//
UINT ProcessVariableBinding(
	BYTE				RequestType,		// IN     SNMP operation to perform 
	RFC1157VarBind*		pVarBind)			// IN OUT Variable binding to process 
{
    UINT                uResult = SNMP_ERRORSTATUS_NOERROR;
    MibDescriptor*		pMibPtr = NULL;
  
	
	// search for the MIB in the map
	pMibPtr = gMIBData.FindMibDescriptorByOID(&pVarBind->name);

	// if it's a GETNEXT and we couldn't find an exact match, then
	// try and get the previous MIB if one exists
	if (! pMibPtr && MIB_ACTION_GETNEXT == RequestType)
	{
		pMibPtr = gMIBData.FindPreviousMibDescriptor(&pVarBind->name);
	}

	// an exact match was found in the map
	if (pMibPtr && MIB_ACTION_GETNEXT == RequestType)
	{
		
		SME_STATUS RefreshStatus = SME_SUCCESS;
		BOOL foundValidMIB = FALSE;
		while (pMibPtr && ! foundValidMIB)
		{
			// if the OID is for a table column MIB and there are no 'rows' (MIBs that are not 
			// table columns) in memory, then try to go fetch the table data
			if (RefreshStatus != SME_SOCKET_ERROR)
			{
				// try to get the next non-tablecol MIB
				do 
				{
						pMibPtr = pMibPtr->pMibNext;
			//	} while ( pMibPtr && ( pMibPtr->GetType() == ASN_OBJECTIDENTIFIER || pMibPtr->m_IsTableCol));
				} while ( pMibPtr && ( pMibPtr->GetType() == ASN_OBJECTIDENTIFIER ));
			}

			if (pMibPtr != NULL)
			{
				time_t currentTime = time(NULL);
				
				// ...Construct the OID of the "next" MIB variable and return it
				SnmpUtilOidCpy(&pVarBind->name, gItvOidServicesPrefix.GetOID());
				SnmpUtilOidAppend(&pVarBind->name, pMibPtr->GetOID());
				uResult = ProcessLeafNode(RequestType, pMibPtr, pVarBind); 
				foundValidMIB = TRUE;
			}
			else
			{
				uResult = SNMP_ERRORSTATUS_NOSUCHNAME;
			}

		} // end while()
	}
	// we have nothing to process
    if (pMibPtr == NULL)
    {
	    // 
		// The OID specified in the GET or SET varbind was not found. 
		// And there was no "next" OID for a GETNEXT operation.
		//
        uResult = SNMP_ERRORSTATUS_NOSUCHNAME;
    }
    else 
	{

		// get current time
		time_t currentTime = time(NULL);
		// if we've never retrieved this value yet or if the refresh interval
		// has been exceeded then get the variable again
		if (pMibPtr->RetrievalCount == 0 || 
			((unsigned)currentTime - pMibPtr->LastModifiedTime > pMibPtr->RefreshInterval))
		{
			// Process the OID in pMibPtr
			
			/*
			RefreshStatus = RefreshVariable(&pMibPtr);
			if (RefreshStatus != SME_SUCCESS)
			{
				return SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
			}
			*/
		}

		// add by dony y by dony 20070122 becauset the get all package procedure will can ProcessLeafNode twice,so add the check conditicon
		if ( RequestType != MIB_ACTION_GETNEXT )
		{
			uResult = ProcessLeafNode(RequestType, pMibPtr, pVarBind);
		}
//		uResult = ProcessLeafNode(RequestType, pMibPtr, pVarBind); // This is the old code modify by dong 20070122 
	}

    return(uResult);
}

void GetProductName(const TCHAR* lpszServiceName, TCHAR lpszProductName[DISPLAY_STRING+1])
{
    LONG lRet;
    HKEY hKey;
    TCHAR szKey[MAX_PATH+1] = _T("SYSTEM\\CurrentControlSet\\Services\\");
    TCHAR szValue[] = _T("ProductName");
    DWORD dwType;
    DWORD dwSize = MAX_PATH;

    _tcscat(szKey, lpszServiceName);

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);

    if (ERROR_SUCCESS == lRet)
    {
        dwSize = MAX_PATH*(sizeof(TCHAR)); 
		lRet = RegQueryValueEx(hKey, szValue, NULL, &dwType, (LPBYTE)lpszProductName, &dwSize);
        RegCloseKey(hKey);

        if (ERROR_SUCCESS == lRet && REG_EXPAND_SZ == dwType)
        {
            TCHAR szProductName[MAX_PATH + 1]={0};
            ExpandEnvironmentStrings(lpszProductName, szProductName, MAX_PATH);
            _tcsncpy(lpszProductName, szProductName, MAX_PATH+1);
        }
    }

    if (ERROR_SUCCESS != lRet)
    {
        //Assume ITV
        _tcscpy(lpszProductName, _T("ITV"));
    }
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
				char szBuf[17];

				uLen = sAny->asnValue.address.length;
				puData = sAny->asnValue.address.stream;

				pString = (char *) SnmpUtilMemAlloc( uLen * 4 );
				if(pString)
				{
					pString[0] = '\0';

					for (i = 0; i < uLen; i++)
					{

						strcat( pString, itoa(puData[i], szBuf,10 ));    
						if( i < uLen-1 )
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
					char szBuf[17];
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
////////////////////////////////////////////////////////////////////////////////////////////
//
// Function to process scalar INTEGER variables.
// Here, process means to check for legal values
//
// Returns the SNMP ErrorStatus of the operation.
//
UINT ProcessIntegerFunc(
	BYTE				bRequestType,		// IN     SNMP operation to perform  
	MibDescriptor*		pMibPtr,			// IN     Pointer to the MIB variable in the MibVars array  
	RFC1157VarBind*		pVarBind )			// IN OUT Variable binding that references the MIB variable 
{

	HANDLE hCfg;
    UINT nErrorStatus = SNMP_ERRORSTATUS_NOERROR;
	TCHAR *szVarName = NULL;
	DWORD dwVar;
	DWORD dError;
	ZQSNMPSTATUS  manStatus;
	CFGSTATUS     cfgStatus;
	BOOL bReadOnly;
	TCHAR szMsg[MAL_LENTH]={0};

	DWORD d1;
	TCHAR *pos = NULL;
	TCHAR szServiceName[SERVICE_NAME_LEN];
	TCHAR szBuf[OID_LENTH];
	memset(szBuf,0,sizeof(szBuf));
	memset(szServiceName,0,sizeof(szServiceName));
	int ilen;
	
				
	switch ( bRequestType )
    {
		// Write the new variable data to the temporary storage in the MibVars 
        case MIB_ACTION_SET:
		{
			pos  = _tcsrchr(pMibPtr->pszStorageName, _T('.'));
			ilen = _tcslen(pMibPtr->pszStorageName) -_tcslen(pos);
			_tcsncpy(szServiceName,pMibPtr->pszStorageName,ilen);
			*(szServiceName+ilen) =0;

			TCHAR szProductName[FILENAME_LEN];
			memset(szProductName,0,sizeof(szProductName));

			GetProductName(szServiceName,szProductName);
			// Check if the new value is within the proper SIZE 
            switch ( pVarBind->value.asnType )
	        {
			case ASN_INTEGER:
				// Store the new value in the temporary MibVars variable 
				pMibPtr->TempVar.asnValue.number = pVarBind->value.asnValue.number;
			    pMibPtr->Modified = TRUE;

				if ( pMibPtr->Access == MIB_ACCESS_READWRITE ) //  可读写
				{
					bReadOnly = FALSE;
				}
				else // 只可读
				{
					bReadOnly = TRUE;
				}
				if ( !bReadOnly)
				{

					pMibPtr->SetValue(pVarBind->value.asnValue.number); // add by dony to set the var's value
				
					// add by dony 20061202 to set share memory
					szVarName = pMibPtr->pszStorageName;
					dwVar = (DWORD)pVarBind->value.asnValue.number;
				
								
					manStatus =SNMPOpenSession(szServiceName,szProductName,&d1,FALSE);
					if ( manStatus == ZQSNMP_SUCCESS )
					{
						manStatus =SNMPManagerSetVar(szVarName,(DWORD)&dwVar,ZQSNMP_INT,bReadOnly,&dError);
						if ( manStatus == ZQSNMP_SUCCESS)
						{
							_stprintf(szMsg,_T("Set Package of Var of name =%s,Var of Data=%d,%s"),pMibPtr->pszStorageName,dwVar,_T("successful\n"));

							hCfg = CFG_INITEx(szServiceName, &d1, szProductName,TRUE);
							if ( hCfg)
							{
								cfgStatus = CFG_VALUE_EXISTS(hCfg,pos+1,TRUE);
								if ( cfgStatus == CFG_SUCCESS)
								{
									d1 = sizeof(DWORD);
									cfgStatus = CFG_SET_VALUE(hCfg,pos+1,(BYTE*)&dwVar,d1,REG_DWORD,TRUE);
								}
								CFG_TERM(hCfg);
							}
							
							_stprintf(szBuf,_T("%s%s"),szServiceName,_T("_shell"));
							hCfg = CFG_INITEx(szBuf, &d1, szProductName,TRUE);
							if ( hCfg)
							{
								cfgStatus = CFG_VALUE_EXISTS(hCfg,pos+1,TRUE);
								if ( cfgStatus == CFG_SUCCESS)
								{
									d1 = sizeof(DWORD);
									cfgStatus = CFG_SET_VALUE(hCfg,pos+1,(BYTE*)&dwVar,d1,REG_DWORD,TRUE);
								}
								CFG_TERM(hCfg);
							}
							
						}
						else
						{
							_stprintf(szMsg,_T("Set Package of Var of name =%s,Var of Data=%d,%s"),pMibPtr->pszStorageName,dwVar,_T("failed\n"));
						}
						#ifdef FILELOGMODE
							(*m_pReporter)(ZQ::common::Log::L_INFO,szMsg);
						#else		
							MClog(hMClog, 1,szMsg);
						#endif
						
					}
				}
				
				// add by dony 20061202 to set share memory
			    break;

			case ASN_UNSIGNED32:
		        // Store the new value in the temporary MibVars variable 
				pMibPtr->TempVar.asnValue.unsigned32 = pVarBind->value.asnValue.unsigned32;
			    pMibPtr->Modified = TRUE;

				pMibPtr->SetValue(pVarBind->value.asnValue.unsigned32); // add by dony to set the var's value

				

				// add by dony 20061202 to set share memory
			    break;

		     case ASN_RFC1155_COUNTER:
			 	// Store the new value in the temporary MibVars variable 
				pMibPtr->TempVar.asnValue.counter64 = pVarBind->value.asnValue.counter64;
				pMibPtr->Modified = TRUE;

				pMibPtr->SetValue(pVarBind->value.asnValue.counter64); // add by dony to set the var's value

				
				// add by dony 20061202 to set share memory

				break;

			 case ASN_RFC1155_GAUGE:
			  	 // Store the new value in the temporary MibVars variable 
				 pMibPtr->TempVar.asnValue.gauge = pVarBind->value.asnValue.gauge;
				 pMibPtr->Modified = TRUE;

				 pMibPtr->SetValue(pVarBind->value.asnValue.gauge); // add by dony to set the var's value

				

				 break;

             case ASN_RFC1155_TIMETICKS:
		         // Store the new value in the temporary MibVars variable 
				 pMibPtr->TempVar.asnValue.ticks = pVarBind->value.asnValue.ticks;
				 pMibPtr->Modified = TRUE;
				 
				 break;
			 default:
                 nErrorStatus = SNMP_ERRORSTATUS_GENERR;
 			     break;
			}

			if (nErrorStatus == SNMP_ERRORSTATUS_NOERROR)
			{
				
				/*
				if (SetInManPkg(pMibPtr, &pMibPtr->TempVar) != SME_SUCCESS)
				{
					nErrorStatus = SNMP_ERRORSTATUS_NOTWRITABLE;
				}
				*/
				// remark the old code modify by dony
			}
            break;
		}

        case MIB_ACTION_GETNEXT:
        case MIB_ACTION_GET:
		{
			// Nothing to do. 

			break;
		}

		default:
		{
			nErrorStatus = SNMP_ERRORSTATUS_GENERR;
			break;
		}
    }

	szVarName = NULL;
	return( nErrorStatus );
}
////////////////////////////////////////////////////////////////////////////////////////////
//
// Function to process scalar OCTET STRING variables.
// Here, process means to check for legal values
//
// Returns the SNMP ErrorStatus of the operation.
//
UINT ProcessStringFunc(
  BYTE			  bRequestType,		// IN     SNMP operation to perform 
  MibDescriptor*  pMibPtr,			// IN OUT Pointer to the MIB variable in the MibVars array 
  RFC1157VarBind* pVarBind )		// IN     Variable binding that references the MIB variable 
{
 
    UINT nErrorStatus = SNMP_ERRORSTATUS_NOERROR;
	
	TCHAR *szVarName = NULL;
	TCHAR *szVarValue = NULL;
	char *strTmp = NULL;
	int iLen;
	ZQSNMPSTATUS  manStatus;
	CFGSTATUS     cfgStatus;
	BOOL bReadOnly;
	DWORD dwVar,dError;
	float fValue;
	TCHAR szMsg[MAL_LENTH]={0};


	HANDLE hCfg;
	DWORD d1;
	TCHAR *pos = NULL;
	TCHAR szServiceName[SERVICE_NAME_LEN];
	TCHAR szBuf[OID_LENTH];
	memset(szBuf,0,sizeof(szBuf));
	memset(szServiceName,0,sizeof(szServiceName));
	TCHAR szNewValue[MAL_LENTH];
	memset(szNewValue,0,sizeof(szNewValue));
		
	
	switch ( bRequestType )
    {
        case MIB_ACTION_SET:
		{
			pos  = _tcsrchr(pMibPtr->pszStorageName, _T('.'));
			iLen = _tcslen(pMibPtr->pszStorageName) -_tcslen(pos);
			_tcsncpy(szServiceName,pMibPtr->pszStorageName,iLen);
			*(szServiceName+iLen) =0;

			TCHAR szProductName[FILENAME_LEN];
			memset(szProductName,0,sizeof(szProductName));
			GetProductName(szServiceName,szProductName);
	
			switch ( pVarBind->value.asnType )
	        {
			case ASN_OCTETSTRING:
				// Copy the varbind OCTET STRING to the temporary storage variable 
				if (pMibPtr->TempVar.asnValue.string.stream && pMibPtr->Modified)
				{
					delete pMibPtr->TempVar.asnValue.string.stream;
					pMibPtr->TempVar.asnValue.string.stream = 0;
				}
				pMibPtr->TempVar.asnValue.string.stream = 
					new BYTE[pVarBind->value.asnValue.string.length];
				memcpy(pMibPtr->TempVar.asnValue.string.stream,
					   pVarBind->value.asnValue.string.stream,
					   pVarBind->value.asnValue.string.length);
				pMibPtr->TempVar.asnValue.string.length = pVarBind->value.asnValue.string.length;
				pMibPtr->Modified = TRUE;

				// (re)allocate memory for the OCTET STRING  add by dony to set the String style's var value 

				// add by dony 20061202 to set share memory
				szVarName = pMibPtr->pszStorageName;
				
				strTmp = (char*)malloc(VAR_DATA_LENGTH);
				strTmp = SNMP_AnyToStr(&(pVarBind->value));
				szVarValue = (TCHAR*)malloc(VAR_DATA_LENGTH);

	#if defined _UNICODE || defined UNICODE
				MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 strTmp,        // address of string to map
						strlen(strTmp),      // number of bytes in string
						szVarValue,       // address of wide-character buffer
						VAR_DATA_LENGTH);             // size of buffer);
				szVarValue[strlen(strTmp)] ='\0';
	#else 
				sprintf(szVarValue,"%s",strTmp);
	#endif
				*(szVarValue+_tcslen(szVarValue))=0;

				// modify by dony 20070122 for add variable's name in the variable data
				if ( pMibPtr->Access == MIB_ACCESS_READWRITE ) //  可读写
				{
					bReadOnly = FALSE;
				}
				else // 只可读
				{
					bReadOnly = TRUE;
				}
				if ( !bReadOnly)
				{
				
		#ifdef VARNAMEIN_MODE // 包含变量名模式
					_stprintf(szNewValue,_T("%s%s%d%s%d%s%s"),szVarName,_T("@"),pMibPtr->chType,_T("+"),pMibPtr->Access,_T(":"),szVarValue);
					pMibPtr->SetValue(szNewValue); // This is the new code
		#else  // OID 模式，原来的处理方式
					pMibPtr->SetValue(pVarBind->value.asnValue.string); // This is the old code 		
		#endif 
				
					
					manStatus =SNMPOpenSession(szServiceName,szProductName,&d1,FALSE);
					if ( manStatus == ZQSNMP_SUCCESS )
					{
					
						// modify by dony 20070122 for add variable's name in the variable data
			#ifdef VARNAMEIN_MODE // 包含变量名模式
						if ( pMibPtr->chType == 1 ) // 整型变量
						{
							dwVar = _ttol(szVarValue);
							manStatus =SNMPManagerSetVar(szVarName,(DWORD)&dwVar,ZQSNMP_INT,bReadOnly,&dError);
						}
						else if ( pMibPtr->chType == 2 ) //字符串变量
						{				
							manStatus =SNMPManagerSetVar(szVarName,(DWORD)(void*)szVarValue,ZQSNMP_STR,bReadOnly,&dError);
						}
						else if ( pMibPtr->chType == 3 ) // 浮点数类型
						{
							_stscanf(szVarValue,_T("%f"),&fValue);
							manStatus =SNMPManagerSetVar(szVarName,(DWORD)&fValue,ZQSNMP_FLOAT,bReadOnly,&dError);
						}
			#else  // OID 模式，原来的处理方式
						manStatus =SNMPManagerSetVar(szVarName,(DWORD)(void*)szVarValue,ZQSNMP_STR,bReadOnly,&dError);
			#endif 

						if ( manStatus == ZQSNMP_SUCCESS)
						{
							_stprintf(szMsg,_T("Set Package of Var of name =%s,Var of Data=%s,%s"),pMibPtr->pszStorageName,szVarValue,_T("Successful\n"));
							hCfg = CFG_INITEx(szServiceName, &d1, szProductName,TRUE);
							if ( hCfg)
							{
								cfgStatus = CFG_VALUE_EXISTS(hCfg,pos+1,TRUE);
								if ( cfgStatus == CFG_SUCCESS)
								{
									if ( pMibPtr->chType == 1 ) // 整型变量
									{
										dwVar = _ttol(szVarValue);
										cfgStatus = CFG_SET_VALUE(hCfg,pos+1,(BYTE*)&dwVar,sizeof(dwVar),REG_DWORD,TRUE);
									}
									else // 字符串型和浮动点数型
									{
										d1 = _tcsclen(szVarValue)*2;
										cfgStatus = CFG_SET_VALUE(hCfg,pos+1,(BYTE*)szVarValue,d1,REG_SZ,TRUE);
									}
								}
								CFG_TERM(hCfg);
							}
							_stprintf(szBuf,_T("%s%s"),szServiceName,_T("_shell"));
							hCfg = CFG_INITEx(szBuf, &d1, szProductName,TRUE);
							if ( hCfg)
							{
								cfgStatus = CFG_VALUE_EXISTS(hCfg,pos+1,TRUE);
								if ( cfgStatus == CFG_SUCCESS)
								{
									if ( pMibPtr->chType == 1 ) // 整型变量
									{
										dwVar = _ttol(szVarValue);
										cfgStatus = CFG_SET_VALUE(hCfg,pos+1,(BYTE*)&dwVar,sizeof(dwVar),REG_DWORD,TRUE);
									}
									else // 字符串型和浮动点数型
									{
										d1 = _tcsclen(szVarValue)*2;
										cfgStatus = CFG_SET_VALUE(hCfg,pos+1,(BYTE*)szVarValue,d1,REG_SZ,TRUE);
									}
								}
								CFG_TERM(hCfg);
							}
						}
						else
						{
							_stprintf(szMsg,_T("Set Package of Var of name =%s,Var of Data=%s,%s"),pMibPtr->pszStorageName,szVarValue,_T("Failed\n"));
						}
						#ifdef FILELOGMODE
							(*m_pReporter)(ZQ::common::Log::L_INFO,szMsg);
						#else		
							MClog(hMClog, 1,szMsg);
						#endif
						
					}
				}
				break;
			case ASN_BITS:
				// Copy the varbind OCTET STRING to the temporary storage variable 
				if (pMibPtr->TempVar.asnValue.bits.stream && pMibPtr->Modified)
				{
					delete pMibPtr->TempVar.asnValue.bits.stream;
					pMibPtr->TempVar.asnValue.bits.stream = 0;
				}
				pMibPtr->TempVar.asnValue.bits.stream = 
					new BYTE[pVarBind->value.asnValue.bits.length];
				memcpy(pMibPtr->TempVar.asnValue.bits.stream,
					   pVarBind->value.asnValue.bits.stream,
					   pVarBind->value.asnValue.bits.length);
				pMibPtr->TempVar.asnValue.bits.length = pVarBind->value.asnValue.bits.length;
				pMibPtr->Modified = TRUE;

				
				// (re)allocate memory for the OCTET STRING  add by dony to set the String style's var value 
				pMibPtr->SetValue(pVarBind->value.asnValue.bits);
				break;
			default:
				nErrorStatus = SNMP_ERRORSTATUS_GENERR;
 			    break;
			}
			if (nErrorStatus == SNMP_ERRORSTATUS_NOERROR)
			{				
				/*
				if (SetInManPkg(pMibPtr, &pMibPtr->TempVar) != SME_SUCCESS)
				{
					nErrorStatus = SNMP_ERRORSTATUS_NOTWRITABLE;
				}
				*/ // remark the old code modify by dony 20061115
			}
			break;
		}

        case MIB_ACTION_GETNEXT:
        case MIB_ACTION_GET:
			// Nothing to do
			break;
        default:
            nErrorStatus = SNMP_ERRORSTATUS_GENERR;
            break;
    }

	if ( szVarValue )
	{
		free(szVarValue);
	}

	szVarValue = NULL;

	if ( strTmp )
	{
		free(strTmp);
	}

	strTmp = NULL;
    return( nErrorStatus );
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMibDataOper::CMibDataOper()
{
	m_pXMLFile = new CXMLFileOper();
//	m_FileMap.clear();
	m_FileCount = 0;
	m_MibMap.clear();
	m_dwRefreshInterval = 0;
	m_iCurCount = 0;
	m_iCompanyNumber = 9;
	m_pLastMib = NULL;
}

CMibDataOper::~CMibDataOper()
{
	// empty the MibMap -  does so we only need to free the mib objects up once
	if (!m_MibMap.empty())
	{
		MIBMAPITERTMP itorTmp;
		for ( itorTmp = m_MibMap.begin(); itorTmp != m_MibMap.end();  itorTmp++)
		{
			// add by dony 20070122 for OID/Varname Mode's switch
#ifdef   OIDSTDSTRING_MODE

#else
			if (( *itorTmp).first )
			{
				free((*itorTmp).first);
			}
#endif
			MibDescriptor *ptr = (*itorTmp).second;
			if ( ptr )
			{
				delete ptr;
				ptr = NULL;
			}
		}
		m_MibMap.clear();
	}
	
	/*
	if ( !m_FileMap.empty())
	{
		FILEMAPITER itorTmp;
		for ( itorTmp = m_FileMap.begin(); itorTmp != m_FileMap.end();  itorTmp++)
		{
			
			if ( itorTmp->second )
				free(itorTmp->second);
		}
		m_FileMap.clear();
	}
	*/

	if ( m_pXMLFile )
	{
		delete m_pXMLFile;
	}
	m_pXMLFile = NULL;

}

// add by dony 20070122 for add varialbe's name in the variable
// iVarType ==1  is as to ASN_INT Type，整型
// iVarType ==2  is as to ASN_STR Type，字符串型
// iVarType ==3  is as to ASN_FLOAT Type,浮动数型,
void CMibDataOper::CreateAndStoreNewMibObject(TCHAR *currentOID, 
								TCHAR *name,
								BYTE currentType,
								DWORD RefreshInterval,
								UINT access,
								TCHAR *value,
								bool IsTable,
								bool IsTableCol,
								INT  iVarType 
								)
{
	TCHAR *newOID;
	newOID = (TCHAR*)malloc(OID_LENTH);
	MibDescriptor *pMib = new MibDescriptor;

	// set Mib vars
	_tcscpy(newOID,m_sCurrentServiceOID);
	if (currentType != ASN_OBJECTIDENTIFIER)
	{
		if ( _tcslen(m_sCurrentOID) != 0 ) 
		{
			_tcscat(newOID,_T("."));
			_tcscat(newOID,m_sCurrentOID);
		}
		if (currentOID )
		{
			_tcscat(newOID,_T("."));
			_tcscat(newOID,currentOID);
		}
		// change the old operator style, not to append each OID with a '.0'
		/*
		if (! IsTableCol)
		{
			_tcscat(newOID,_T(".0")); // append each OID with a '.0'
		}
		*/
	}
	unsigned len = _tcsclen(newOID);
	*(newOID+len)=0; 
	TCHAR Buffer[OID_LENTH]={0};
	_tcsnccpy(Buffer, newOID, len);
	*(Buffer+len) = 0;
	// set the OID
	pMib->SetOID(Buffer);
	// set the type
	pMib->SetType(currentType);
	// set value and processing func based on type
	if (currentType == ASN_INTEGER)
	{
		pMib->SetValue(_ttol(value));
		pMib->pMibFunc = ProcessIntegerFunc;
		pMib->MaxVal = 2147483647;
		pMib->MinVal = -2147483647;
	}
	else if (currentType == ASN_UNSIGNED32) 
	{
		pMib->SetValue(_ttol(value));
		pMib->pMibFunc = ProcessIntegerFunc;
		pMib->MaxVal = 4294967295;
		pMib->MinVal = 0;
	}
	else if (currentType == ASN_OCTETSTRING ||
			 currentType == ASN_OBJECTIDENTIFIER)
	{
	// modify by dony 20070122 for add variable's name in the value, below is the old code,vardata=变量名@变量类型(整型如1 or 2 or  3):变量值
#ifdef VARNAMEIN_MODE // 包含变量名模式
		if ( name && value ) // variable's value = name/vartype+varReadWriteProperty:vardata this is the new code
		{
			TCHAR szNewValue[MAL_LENTH]={0};
			_stprintf(szNewValue,_T("%s%s%d%s%d%s%s"),name,_T("@"),iVarType,_T("+"),access,_T(":"),value);
			pMib->SetValue(szNewValue);
		}
#else  // OID 模式，原来的处理方式
		if ( value ) 
		{
			pMib->SetValue(value);
		}
#endif 
		pMib->pMibFunc = ProcessStringFunc;
		pMib->MaxVal   = 4294967295;
		pMib->MinVal   = 0;
	}
	else if (currentType == ASN_BITS )
	{
		pMib->SetValue(value);
		pMib->pMibFunc = ProcessStringFunc;
		pMib->MaxVal   = 4294967295;
		pMib->MinVal   = 0;
	}
	// set the variant description
	if ( name )
	{
		pMib->pszStorageName = (TCHAR*)malloc(VAR_NAME_LENGTH);
		_tcscpy(pMib->pszStorageName,name);
	}
	
	// set other values as well
	pMib->RefreshInterval = RefreshInterval;
	pMib->m_IsTable = IsTable;
	pMib->m_IsTableCol = IsTableCol;
	pMib->LastModifiedTime = time(NULL);
	pMib->Access = access;

	// modify by dony 20070122 for add variable's name in the value, below is the old code 
	pMib->chType= iVarType; 

#ifdef OIDSTDSTRING_MODE
		string strOID;
	#if defined _UNICODE || defined UNICODE
		char cOidTemp[OID_LENTH];
		memset(cOidTemp,0,sizeof(cOidTemp));
		WideCharToMultiByte(CP_ACP,NULL,newOID,-1,cOidTemp,sizeof(cOidTemp),NULL,NULL);
		strOID = cOidTemp;
	#else
		strOID =  newOID;
	#endif
		m_MibMap[strOID] = pMib;
	//	m_MibMap.insert(MIBMAPTYPE(strOID,pMib));
#else
		m_MibMap[newOID] = pMib;
	//	m_MibMap.insert(MIBMAPTYPE(newOID,pMib));
#endif

	// set pointer to this MibDescriptor object into the Mib Map by OID

	// set Mib pointer; 这里的设置有待考虑
	if ( m_pLastMib)
	{
		m_pLastMib->pMibNext = pMib;
	}
	pMib->pMibNext = NULL;
	m_pLastMib = pMib;

	if ( newOID )
	{
		newOID = NULL;
	}
}

bool CMibDataOper::GetRegistryValues()
{
	DWORD dwNumber;
	HANDLE hCfg = CFG_INITEx(_T("ZQSnmpExtension"), &dwNumber, _T("ITV"),FALSE);
	if (NULL == hCfg)
	{
		return false;
	}
	DWORD dwSize = sizeof(DWORD);
	DWORD dwType, dwValue;

	// get refresh interval
    CFGSTATUS cStatus = CFG_GET_VALUE(hCfg, (TCHAR*)_T("RefreshInterval"), (BYTE*)&dwValue, &dwSize, &dwType,FALSE);
	m_dwRefreshInterval = (CFG_SUCCESS != cStatus) ? DEFAULT_REFRESH_INTERVAL: dwValue;

	TCHAR XMLConfigFile[MAL_LENTH];
	memset(XMLConfigFile,0,sizeof(XMLConfigFile));
	dwSize = MAL_LENTH * sizeof(TCHAR);
	// get .xml file path
    cStatus = CFG_GET_VALUE(hCfg, (TCHAR*)_T("XMLConfigFilePath"), (BYTE*)XMLConfigFile, &dwSize, &dwType,FALSE);
	if (CFG_SUCCESS != cStatus) 
	{
		// modify by dony 20070309 for set the default value
		_stprintf(XMLConfigFile,_T("%s"),_T("C:\\TianShan\\SNMPXMLFiles"));
/*
		CFG_TERM(hCfg);
		return FALSE;
*/
	}
	_tcscpy(m_szXMLFileDirectory,XMLConfigFile);
	*(m_szXMLFileDirectory+(_tcsclen(XMLConfigFile))) =0;

	memset(XMLConfigFile,0,sizeof(XMLConfigFile));
	dwSize = MAL_LENTH * sizeof(TCHAR);
	
	// get .log file path
	// modify by dony 20070309 for set the default value
	if( m_szXMLFileDirectory[_tcsclen(m_szXMLFileDirectory) -1] != '\\' )
	{
		_tcscat(m_szXMLFileDirectory,_T("\\"));
	}
	memset(m_strFile,0,sizeof(m_strFile));
	_stprintf(m_strFile,_T("%s%s"),m_szXMLFileDirectory,_T("Agent.log"));
	
	/* this is the old code 
    cStatus = CFG_GET_VALUE(hCfg, (TCHAR*)_T("AgentLogFilePath"), (BYTE*)XMLConfigFile, &dwSize, &dwType,FALSE);
	if (CFG_SUCCESS != cStatus) 
	{
		CFG_TERM(hCfg);
		return FALSE;
	}
	*/
	
	CFG_TERM(hCfg);
	return TRUE;
}

/*
void  CMibDataOper::GetFilesVector(TCHAR *rootDir)
{
	
	WIN32_FIND_DATA fd;
	ZeroMemory(&fd, sizeof(WIN32_FIND_DATA));
	
	TCHAR *szFileName;
	szFileName = (TCHAR*)malloc(MAL_LENTH);
	ZeroMemory(szFileName, MAL_LENTH);

	int iCount = 0;
	HANDLE hSearch;
	TCHAR filePathName[MAL_LENTH];
	TCHAR filePathTmp[MAL_LENTH];
	TCHAR *filePathName1;
	filePathName1 = (TCHAR*)malloc(MAL_LENTH);

	memset(filePathTmp, 0,MAL_LENTH);
	memset(filePathName,0,MAL_LENTH);
	memset(filePathName1,0,MAL_LENTH);
	
	_tcscpy(filePathName,rootDir);
	
	BOOL bSearchFinished = FALSE;

	if( filePathName[_tcsclen(filePathName) -1] != '\\' )
	{
		_tcscat(filePathName,_T("\\"));
	}
	_tcscpy(filePathTmp,filePathName);
	_tcscat(filePathName,_T("*"));
	
	hSearch = FindFirstFile(filePathName, &fd);

	//Is directory
	
	if ( fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
	{
		if( _tcscmp(fd.cFileName,_T(".")) && _tcscmp(fd.cFileName,_T("..")))
		{
			memset(szFileName,0,sizeof(MAL_LENTH));
			_tcscpy(szFileName,fd.cFileName);
			memset(filePathName1,0,sizeof(MAL_LENTH));

			if ( ( _tcstok(szFileName,_T(".xml") )!= NULL ) ||  (_tcstok(szFileName,_T(".XML")) != NULL ) )
			{		
				_tcscpy(filePathName1,filePathTmp);
				_tcscat(filePathName1,fd.cFileName);
				iCount ++;
				m_FileMap[iCount]=filePathName1;
			}
		}
	}

	while( !bSearchFinished )
	{
		if( FindNextFile(hSearch, &fd) )
		{
			if ( fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
			{
	
				if( _tcscmp(fd.cFileName,_T(".")) && _tcscmp(fd.cFileName,_T("..")))
				{
					memset(szFileName,0,sizeof(MAL_LENTH));
					_tcscpy(szFileName,fd.cFileName);
					memset(filePathName1,0,sizeof(MAL_LENTH));

					if ( (_tcstok(szFileName,_T(".xml")) != NULL ) ||  (_tcstok(szFileName,_T(".XML")) != NULL ) )
					{
						_tcscpy(filePathName1,filePathTmp);
						_tcscat(filePathName1,fd.cFileName);
						iCount ++;
						m_FileMap[iCount]=filePathName1;
					}
				}
			}
		}
		else
		{
			if( GetLastError() == ERROR_NO_MORE_FILES )			//Normal Finished
			{
				bSearchFinished = TRUE;
			}
			else
			{
				bSearchFinished = TRUE;		//Terminate Search
			}
		}
	}

	FindClose(hSearch);
}
*/


INT  CMibDataOper::GetFileCount(TCHAR *rootDir)
{
	
	WIN32_FIND_DATA fd;
	ZeroMemory(&fd, sizeof(WIN32_FIND_DATA));
	INT iCount = 0;

	TCHAR *szFileName;
	szFileName = (TCHAR*)malloc(MAL_LENTH);
	ZeroMemory(szFileName, MAL_LENTH);

	
	HANDLE hSearch;
	TCHAR filePathName[MAL_LENTH];
	TCHAR filePathTmp[MAL_LENTH];

	
	
	memset(filePathTmp, 0,MAL_LENTH);
	memset(filePathName,0,MAL_LENTH);
	
	_tcscpy(filePathName,rootDir);

	TCHAR *pos = NULL;
	
	BOOL bSearchFinished = FALSE;

	if( filePathName[_tcsclen(filePathName) -1] != '\\' )
	{
		_tcscat(filePathName,_T("\\"));
	}
	_tcscpy(filePathTmp,filePathName);
	_tcscat(filePathName,_T("*"));
	
	hSearch = FindFirstFile(filePathName, &fd);

	//Is directory
	
	if ( fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
	{
		if( _tcscmp(fd.cFileName,_T(".")) && _tcscmp(fd.cFileName,_T("..")))
		{
			memset(szFileName,0,sizeof(MAL_LENTH));
			_tcscpy(szFileName,fd.cFileName);

			pos  = _tcsrchr(szFileName, _T('.'));
			
			if ( _tcsclen(pos) == 4 )
			{
				if (  ( _tcsncmp(pos,_T(".xml"),4) ==  0 ) ||  ( _tcsncmp(pos,_T(".XML"),4) == 0 ) )
				
			//	if ( ( _tcstok(szFileName,_T(".xml") )!= NULL ) ||  (_tcstok(szFileName,_T(".XML")) != NULL ) )
				{		
					iCount ++;
				}
			}
		}
	}

	while( !bSearchFinished )
	{
		if( FindNextFile(hSearch, &fd) )
		{
			if ( fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
			{
	
				if( _tcscmp(fd.cFileName,_T(".")) && _tcscmp(fd.cFileName,_T("..")))
				{
					memset(szFileName,0,sizeof(MAL_LENTH));
					_tcscpy(szFileName,fd.cFileName);

					pos  = _tcsrchr(szFileName, _T('.'));
					
					if ( _tcsclen(pos) == 4 )
					{
						if (  ( _tcsncmp(pos,_T(".xml"),4) == 0 ) ||  ( _tcsncmp(pos,_T(".XML"),4) == 0 ) )
						
	//					if ( (_tcstok(szFileName,_T(".xml")) != NULL ) ||  (_tcstok(szFileName,_T(".XML")) != NULL ) )
						{
							iCount ++;
						}
					}
				}
			}
		}
		else
		{
			if( GetLastError() == ERROR_NO_MORE_FILES )			//Normal Finished
			{
				bSearchFinished = TRUE;
			}
			else
			{
				bSearchFinished = TRUE;		//Terminate Search
			}
		}
	}

	FindClose(hSearch);
	m_FileCount = iCount;
	if ( szFileName )
	{
		free(szFileName);
	}
	szFileName = NULL;
	return iCount;
}

void CMibDataOper::SetFileNameMemory(TCHAR *rootDir,FILEDATAS ** pFileData)
{
	
	WIN32_FIND_DATA fd;
	ZeroMemory(&fd, sizeof(WIN32_FIND_DATA));
	
	TCHAR *szFileName;
	szFileName = (TCHAR*)malloc(MAL_LENTH);
	ZeroMemory(szFileName, MAL_LENTH);

	
	TCHAR *pos = NULL;

	HANDLE hSearch;
	TCHAR filePathName[MAL_LENTH];
	TCHAR filePathTmp[MAL_LENTH];
	TCHAR *filePathName1;
	filePathName1 = (TCHAR*)malloc(MAL_LENTH);

	memset(filePathTmp, 0,MAL_LENTH);
	memset(filePathName,0,MAL_LENTH);
	memset(filePathName1,0,MAL_LENTH);
	
	_tcscpy(filePathName,rootDir);
	
	BOOL bSearchFinished = FALSE;

	if( filePathName[_tcsclen(filePathName) -1] != '\\' )
	{
		_tcscat(filePathName,_T("\\"));
	}
	_tcscpy(filePathTmp,filePathName);
	_tcscat(filePathName,_T("*"));
	
	hSearch = FindFirstFile(filePathName, &fd);

	//Is directory
	*pFileData = new FILEDATAS[m_FileCount];

	for ( int i = 0; i < m_FileCount; i ++ )
	{
		(*pFileData)[i].szFileName = (TCHAR*)malloc(MAL_LENTH);
	}
	
	int iCountTmp; 
	iCountTmp = 0;

	if ( fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
	{
		if( _tcscmp(fd.cFileName,_T(".")) && _tcscmp(fd.cFileName,_T("..")))
		{
			memset(szFileName,0,sizeof(MAL_LENTH));
			_tcscpy(szFileName,fd.cFileName);
			memset(filePathName1,0,sizeof(MAL_LENTH));

			
			pos  = _tcsrchr(szFileName, _T('.'));
			if ( _tcslen(pos) == 4 )
			{
				if (  ( _tcsncmp(pos,_T(".xml"),4) ==  0 ) ||  ( _tcsncmp(pos,_T(".XML"),4) == 0 ) )
				{		
					_tcscpy(filePathName1,filePathTmp);
					_tcscat(filePathName1,fd.cFileName);
					_tcscpy((*pFileData)[iCountTmp].szFileName,filePathName1);
					iCountTmp ++;
				}
			}
		}
	}

	while( !bSearchFinished )
	{
		if( FindNextFile(hSearch, &fd) )
		{
			if ( fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
			{
	
				if( _tcscmp(fd.cFileName,_T(".")) && _tcscmp(fd.cFileName,_T("..")))
				{
					memset(szFileName,0,sizeof(MAL_LENTH));
					_tcscpy(szFileName,fd.cFileName);
					memset(filePathName1,0,sizeof(MAL_LENTH));

					pos  = _tcsrchr(szFileName, _T('.'));
					if ( _tcslen(pos) == 4 )
					{
						if (  ( _tcsncmp(pos,_T(".xml"),4) ==  0 ) ||  ( _tcsncmp(pos,_T(".XML"),4) == 0 ) )
						{
							_tcscpy(filePathName1,filePathTmp);
							_tcscat(filePathName1,fd.cFileName);
							
							_tcscpy((*pFileData)[iCountTmp].szFileName,filePathName1);
							iCountTmp ++;
						}
					}
				}
			}
		}
		else
		{
			if( GetLastError() == ERROR_NO_MORE_FILES )			//Normal Finished
			{
				bSearchFinished = TRUE;
			}
			else
			{
				bSearchFinished = TRUE;		//Terminate Search
			}
		}
	}
	FindClose(hSearch);
	if ( szFileName )
	{
		free(szFileName);
	}
	szFileName = NULL;
	if ( filePathName1 )
	{
		free(filePathName1);
	}
	filePathName1 = NULL;
}

bool CMibDataOper::InitializeMibData(TCHAR *szXMLFileName)
{
	try
	{
		
		if ( szXMLFileName == NULL)
			return false;

		ZQ::common::XMLPrefDoc* m_XMLDoc;
		ZQ::common::ComInitializer* m_comInit;
		
		m_comInit = new ZQ::common::ComInitializer();
		m_XMLDoc = new ZQ::common::XMLPrefDoc (*m_comInit);
		char strFile[FILENAME_LEN] ={0};

	#if defined _UNICODE || defined UNICODE
		WideCharToMultiByte(CP_ACP,NULL,szXMLFileName,-1,strFile,sizeof(strFile),NULL,NULL);
	#else
		sprintf(strFile,"%s",szXMLFileName);
	#endif

		if ( m_XMLDoc )
		{
			bool bRes;
			bRes = m_XMLDoc->open(strFile);
			if ( !bRes )
			{
				return bRes;
			}
		}
		
		bool gotName, gotOID, gotType,gotValue;
		gotName = gotOID = gotType =  gotValue = false;
		
		TCHAR szName[VAR_NAME_LENGTH] ={0};
		TCHAR szOID[OID_LENTH]  ={0};
		TCHAR szType[OID_LENTH] ={0};
		TCHAR szAccessType[OID_LENTH] ={0};
		TCHAR szValue[MAL_LENTH] ={0};
		TCHAR szValueName[VAR_NAME_LENGTH] ={0};

		
		TCHAR szServiceName[VAR_NAME_LENGTH] ={0};
		TCHAR szServiceOID[OID_LENTH]={0};

		BYTE currentType;
		UINT access = MIB_ACCESS_READONLY;
			
		char szNodeName[OID_LENTH]={0};
		char szNodeValue[MAL_LENTH]={0};
		INT  iVarType = 1;

		ZQ::common::IPreference* rootIpref = m_XMLDoc->root();
		ZQ::common::IPreference* itemIpref = NULL;
		ZQ::common::IPreference* itemIpref1 = NULL;
		BOOL bFind = FALSE;
		

		itemIpref = rootIpref->firstChild(); 
		while(itemIpref != NULL)
		{
			itemIpref->name(szNodeName);
			
			m_iCurCount ++ ;
			if ( m_iCurCount == 1  )
			{

				// Info 
				if ( _stricmp(szNodeName,XML_SNMP_INFO) == 0 )
				{
					memset(szNodeValue,0,sizeof(szNodeValue));
					itemIpref->get(XML_SNMP_SUPPORTOID,szNodeValue);
					memset(szOID,0,sizeof(szOID));

		#if defined _UNICODE || defined UNICODE
					MultiByteToWideChar(
							 CP_ACP,         // code page
							 0,              // character-type options
							 szNodeValue,        // address of string to map
							strlen(szNodeValue),      // number of bytes in string
							szOID,       // address of wide-character buffer
							OID_LENTH);             // size of buffer);

		#else
					sprintf(szOID,"%s",szNodeValue);
		#endif
				
					extern SnmpVariable gItvOidServicesPrefix;
					unsigned len = _tcsclen(szOID);
					*(szOID+len) = 0;
					gItvOidServicesPrefix.SetOID(szOID);

					m_iCompanyNumber = gItvOidServicesPrefix.name.idLength; // add by dony 20070122 for comanyoid is dynamic value
					
				}
			}

			//service 
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			if ( _stricmp(szNodeName,XML_SNMP_SERVICE) == 0 )
			{
				itemIpref1 = itemIpref->firstChild();
				while(itemIpref1 != NULL)
				{
					memset(szNodeName,0,sizeof(szNodeName));
					itemIpref1->name(szNodeName);
					
					// GET THE SERVICE 
					if ( _stricmp(szNodeName,XML_SNMP_SERVICEDATA) == 0 )
					{
						// GET SERVICE NAME
						memset(szNodeName,0,sizeof(szNodeName));
						itemIpref1->get(XML_SNMP_NAME,szNodeName);
						memset(szServiceName,0,sizeof(szServiceName));
										

		#if defined _UNICODE || defined UNICODE
						MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 szNodeName,        // address of string to map
						strlen(szNodeName),      // number of bytes in string
						szServiceName,       // address of wide-character buffer
						OID_LENTH);             // size of buffer);
		#else
						sprintf(szServiceName,"%s",szNodeName);
		#endif
						*(szServiceName+_tcslen(szServiceName))=0;
						if ( strlen(szNodeName) > 0 )
						{
							gotName = true;
						}

						// GET SERVICE OID
						memset(szNodeName,0,sizeof(szNodeName));
						itemIpref1->get(XML_SNMP_OID,szNodeName);
						memset(szServiceOID,0,sizeof(szServiceOID));

						

		#if defined _UNICODE || defined UNICODE
						MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 szNodeName,        // address of string to map
						strlen(szNodeName),      // number of bytes in string
						szServiceOID,       // address of wide-character buffer
						OID_LENTH);             // size of buffer);
		#else
						sprintf(szServiceOID,"%s",szNodeName);
		#endif

						*(szServiceOID+_tcslen(szServiceOID))=0;
						memset(m_sCurrentOID,0,sizeof(m_sCurrentOID));
						_stprintf(m_sCurrentOID,_T("%s"),szServiceOID);
						

						unsigned len = _tcsclen(szServiceOID);
						_tcsnccpy(m_sCurrentServiceOID,szServiceOID,len);
						*(m_sCurrentServiceOID+len) = 0;
						memset(m_sCurrentOID,0,sizeof(m_sCurrentOID));
						memset(m_sCurrentName,0,sizeof(m_sCurrentName));

						if ( strlen(szNodeName) > 0 )
						{
							gotValue = true;
						}

						if ( gotName && gotValue )
						{

							// create and store new MibDescriptor object
							CreateAndStoreNewMibObject(szServiceOID,
										   szServiceName,						
										   ASN_OBJECTIDENTIFIER,
										   1000,
										   MIB_ACCESS_READONLY,
										   NULL,
										   false,
										   false);
						}

					}

					// GET THE VARIABLE 
					memset(szNodeName,0,sizeof(szNodeName));
					itemIpref1->name(szNodeName);
					gotName = gotOID = gotType =  gotValue = false;

					if ( _stricmp(szNodeName,XML_SNMP_VAR) == 0 )
					{
						// GET THE NAME 
						memset(szNodeName,0,sizeof(szNodeName));
						itemIpref1->get(XML_SNMP_NAME,szNodeName);
						memset(szName,0,sizeof(szName));
						
		#if defined _UNICODE || defined UNICODE
						MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 szNodeName,        // address of string to map
						strlen(szNodeName),      // number of bytes in string
						szName,       // address of wide-character buffer
						OID_LENTH);             // size of buffer);
		#else
						sprintf(szName,"%s",szNodeName);
		#endif
						_tcsupr(szServiceName);
						_tcscpy(szValueName,szServiceName);
						_tcscat(szValueName,_T("."));
						_tcscat(szValueName,szName);
								
						*(szValueName+(_tcsclen(szValueName))) = 0;

						if ( strlen(szNodeName ) > 0 )
						{
							gotName = true;
						}
						

						// GET THE OID
						memset(szNodeName,0,sizeof(szNodeName));
						itemIpref1->get(XML_SNMP_OID,szNodeName);
						memset(szOID,0,sizeof(szOID));

						

		#if defined _UNICODE || defined UNICODE
						MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 szNodeName,        // address of string to map
						strlen(szNodeName),      // number of bytes in string
						szOID,       // address of wide-character buffer
						OID_LENTH);             // size of buffer);
		#else
						sprintf(szOID,"%s",szNodeName);
		#endif
						*(szOID+(_tcsclen(szOID))) = 0;
						if ( strlen(szNodeName ) > 0 )
						{

							gotOID = true;
						}

						// GET THE TYPE
						memset(szNodeName,0,sizeof(szNodeName));
						itemIpref1->get(XML_SNMP_TYPE,szNodeName);
						memset(szType,0,sizeof(szType));
						
						
		#if defined _UNICODE || defined UNICODE
						MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 szNodeName,        // address of string to map
						strlen(szNodeName),      // number of bytes in string
						szType,       // address of wide-character buffer
						OID_LENTH);             // size of buffer);
		#else
						sprintf(szType,"%s",szNodeName);
		#endif
						*(szType+(_tcsclen(szType))) = 0;

						if ( _tcsicmp(szType,_T("ASN_INT")) == 0 )
						{
						
							// modify by dony 20070122 for add variable's name in the value, below is the old code 
		#ifdef VARNAMEIN_MODE // 包含变量名模式
							currentType = ASN_OCTETSTRING;
		#else  // OID 模式，原来的处理方式
							currentType = ASN_INTEGER; 
		#endif 
							iVarType = 1;
							gotType = true;
						}
						else if ( _tcsicmp(szType,_T("ASN_STR")) == 0 )
						{
							currentType = ASN_OCTETSTRING;
							iVarType = 2; // This is the new code add by dony 20070122
							gotType = true;
						}
						else if ( _tcsicmp(szType,_T("ASN_FLOAT"))== 0)
						{
							currentType = ASN_OCTETSTRING;
							iVarType = 3; 
							gotType = true;
						}
						else if ( _tcsicmp(szType,_T("ASN_UNSIGNED32")) == 0 )
						{
							currentType = ASN_UNSIGNED32;
							iVarType = 1;
							gotType = true;
						}
						else if ( _tcsicmp(szType,_T("ASN_BOOL")) == 0 )
						{
							currentType = ASN_BITS;
							iVarType = 2;
							gotType = true;
						}

						// GET THE ACCESS
						memset(szNodeName,0,sizeof(szNodeName));
						itemIpref1->get(XML_SNMP_ACCESS,szNodeName);
						memset(szAccessType,0,sizeof(szAccessType));
						

		#if defined _UNICODE || defined UNICODE
						MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 szNodeName,        // address of string to map
						strlen(szNodeName),      // number of bytes in string
						szAccessType,       // address of wide-character buffer
						OID_LENTH);             // size of buffer);
		#else
						sprintf(szAccessType,"%s",szNodeName);
		#endif
						*(szAccessType+(_tcsclen(szAccessType))) = 0;

						if ( _tcsicmp(szAccessType,_T("READWRITE")) == 0 )
						{
							access = MIB_ACCESS_READWRITE;
						}
						else if ( _tcsicmp(szAccessType,_T("WRITEONLY")) == 0 )
						{
							access = MIB_ACCESS_WRITEONLY;
						}
						else if ( _tcsicmp(szAccessType,_T("READONLY")) == 0 )
						{
							access = MIB_ACCESS_READONLY;
						}
						else if ( _tcsicmp(szAccessType,_T("NOTACCESSIBLE")) == 0 )
						{
							access = MIB_ACCESS_NOTACCESSIBLE;
						}
						
						// GET THE VALUE
						memset(szNodeValue,0,sizeof(szNodeValue));
						itemIpref1->get(XML_SNMP_VALUE,szNodeValue);
						memset(szValue,0,sizeof(szValue));

						
						
		#if defined _UNICODE || defined UNICODE
						MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 szNodeValue,        // address of string to map
						strlen(szNodeValue),      // number of bytes in string
						szValue,       // address of wide-character buffer
						MAL_LENTH);             // size of buffer);
		#else
						sprintf(szValue,"%s",szNodeValue);
		#endif
						*(szValue+(_tcsclen(szValue))) = 0;
						if ( strlen(szNodeValue ) > 0 )
						{
							gotValue = true;
						}
						
						if (gotName && gotOID && gotType & gotValue )  // only need these 4 as a minimum
						{
							// create and store new MibDescriptor object
							CreateAndStoreNewMibObject(szOID,
													   szValueName,						
													   currentType,
													   1000,
													   access,
													   szValue,
													   false,
													   false,
													   iVarType
													   );
						}
					}
					itemIpref1 = itemIpref->nextChild();
				}
			}
			itemIpref = rootIpref->nextChild();
		}
		if ( itemIpref )
		{
			itemIpref->free();
		}
		rootIpref->free();
			
		if(m_comInit != NULL)
			delete m_comInit;
		m_comInit = NULL;

		m_XMLDoc = NULL;
	}
	catch(...)
	{
		return false;
	}
	return false;
}

void CMibDataOper::TestResult()
{
	int iSize;
	iSize = this->m_MibMap.size();

	MIBMAPITERTMP itorTmp;
	MibDescriptor*		pMatch = NULL;
	
	for ( itorTmp = m_MibMap.begin(); itorTmp != m_MibMap.end();  itorTmp++)
	{
#ifdef OIDSTDSTRING_MODE
		string strOid;
		strOid = itorTmp->first;
		pMatch = itorTmp->second;
#else
		TCHAR *OID;
		OID = (TCHAR*)malloc(OID_LENTH);
		OID    = itorTmp->first;
		pMatch = itorTmp->second;
		if ( OID )
		{
			free(OID);
			OID = NULL;
		}
#endif
	}
}

bool CMibDataOper::InitializeMibDescriptors(void)
{
	// Get the refresh interval and full path to the .xml file from the registry. The file name
	// must exist but the refresh interval is optional (we'll use the hard coded default value
	// if it doesn't exist in the registry).
	try
	{
	
		if (! GetRegistryValues())
		{
			return false;
		}
	//	GetFilesVector(this->m_szXMLFileDirectory);
	//	int iSize = this->m_FileMap.size();

		GetFileCount(this->m_szXMLFileDirectory);
		FILEDATAS *pFileData = NULL;
		SetFileNameMemory(this->m_szXMLFileDirectory,&pFileData);
		
		int i ;
		for ( i = 0; i < m_FileCount; i ++ )
		{
			memset(m_szXMLFileName,0,sizeof(MAL_LENTH));
			_tcscpy(m_szXMLFileName,pFileData[i].szFileName );
			InitializeMibData(m_szXMLFileName);
		}
		
		for ( i = 0; i < m_FileCount; i ++ )
		{
			if ( pFileData[i].szFileName)
			{
				free(pFileData[i].szFileName);
			}
			pFileData[i].szFileName = NULL;
		}
		pFileData = NULL;

		/*
		FILEMAPITER itorTmp;
		
		for ( itorTmp = m_FileMap.begin(); itorTmp != m_FileMap.end();  itorTmp++)
		{
			memset(m_szXMLFileName,0,sizeof(MAL_LENTH));
			_tcscpy(m_szXMLFileName,(itorTmp->second));
	//		_tcscpy(m_szXMLFileName,m_FileVector[i]);
			this->InitializeMibData(m_szXMLFileName);
		}
		*/

	//	this->TestResult();
	}
	catch(...)
	{
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Function to find a Mib Descriptor by Key
//
// This routine may not be needed. It may be best to leave the search embeded in
// the variable processing code above as it is now.
//
MibDescriptor* CMibDataOper::FindMibDescriptorByKey(TCHAR *szKey)
{
	MibDescriptor*		pMatch = NULL;
	try
	{
	
		int iSize;
		iSize = this->m_MibMap.size();
		MIBMAPITERTMP itorTmp;

		// add by dony 20070122 for add variable's name in the variable's Data,below remark is the old code
#ifdef OIDSTDSTRING_MODE
		string strOid;
		#if defined _UNICODE || defined UNICODE
			char cOidTemp[OID_LENTH];
			memset(cOidTemp,0,sizeof(cOidTemp));
			WideCharToMultiByte(CP_ACP,NULL,szKey,-1,cOidTemp,sizeof(cOidTemp),NULL,NULL);
			strOid = cOidTemp;
		#else
			strOid = szKey;
		#endif
		
		itorTmp = m_MibMap.find(strOid);
		if ( itorTmp != m_MibMap.end())
		{
			pMatch = itorTmp->second;
		}
#else
		TCHAR *OID;
		OID = (TCHAR*)malloc(OID_LENTH);
		if ( !m_MibMap.empty() )
		{
			for ( itorTmp = m_MibMap.begin(); itorTmp != m_MibMap.end();  itorTmp++)
			{
				OID    = itorTmp->first;
				if (0 == _tcscmp(OID,szKey))
				{
					pMatch = itorTmp->second;
					break;
				}
			}
		}
		if ( OID )
		{
			free(OID);
			OID = NULL;
		}
#endif
	}
	catch(...)
	{
		return NULL;
	}
	return pMatch;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Function to find a Mib Descriptor by OID
//
// This routine may not be needed. It may be best to leave the search embeded in
// the variable processing code above as it is now.
//
MibDescriptor*	CMibDataOper::FindMibDescriptorByOID(AsnObjectIdentifier* pOID)
{
	MibDescriptor*		pMatch = NULL;
	try
	{
		MIBMAPITERTMP       itorTmp;

		// build the OID used to perform the search
		AsnObjectIdentifier tempObj;
		tempObj.idLength = pOID->idLength - m_iCompanyNumber;
		tempObj.ids = &pOID->ids[m_iCompanyNumber];

		// add by dony 20070122 for add variable's name in the variable's Data,below remark is the old code
	#if defined _UNICODE || defined UNICODE
		TCHAR sTemp[OID_LENTH];
		MultiByteToWideChar(
					CP_ACP,         // code page
					0,              // character-type options
					SnmpUtilOidToA(&tempObj),        // address of string to map
					strlen(SnmpUtilOidToA(&tempObj)),      // number of bytes in string
					sTemp,       // address of wide-character buffer
					OID_LENTH);  // size of
		*(sTemp+strlen(SnmpUtilOidToA(&tempObj))) =0;
	
//		TCHAR *sTemp = SnmpUtilOidToW(&tempObj); // This is new code add by dony 20070122 
	#else
		TCHAR *sTemp = SnmpUtilOidToA(&tempObj);
	#endif
		pMatch = FindMibDescriptorByKey(sTemp);
	}
	catch(...)
	{
		return NULL;
	}
	return (pMatch);
}

MibDescriptor* CMibDataOper::FindPreviousMibDescriptor(AsnObjectIdentifier* pOID)
{
	try
	{
		
		MibDescriptor*		pMib = NULL;
		MIBMAPITERTMP       itorTmp;

		// build the OID used to perform the search
		AsnObjectIdentifier tempObj;
		tempObj.idLength = 1;
		tempObj.ids = &pOID->ids[m_iCompanyNumber];

		// do the lookup for the first Mib object for this particular service

	#if defined _UNICODE || defined UNICODE
		 // add by dony 20070122 for add variable's name in the variable's Data,below ramarked is the old code
		TCHAR sTemp[OID_LENTH];
		MultiByteToWideChar(
					CP_ACP,         // code page
					0,              // character-type options
					SnmpUtilOidToA(&tempObj),        // address of string to map
					strlen (SnmpUtilOidToA(&tempObj)),      // number of bytes in string
					sTemp,       // address of wide-character buffer
					OID_LENTH);             // size of
		*(sTemp+strlen(SnmpUtilOidToA(&tempObj))) =0;
		
		//TCHAR *sTemp = SnmpUtilOidToW(&tempObj); // This is the new code 
	#else
		TCHAR *sTemp = SnmpUtilOidToA(&tempObj);
	#endif
		pMib = FindMibDescriptorByKey(sTemp);
		BOOL foundMib = FALSE;
		if ( pMib ) 
		{
			foundMib = TRUE;
		}
		MibDescriptor* pLastMib = pMib;

		if ( foundMib ) // we found a starting point for all Mibs for this service
		{
			AsnObjectIdentifier OrigObj;
			OrigObj.idLength = pOID->idLength - m_iCompanyNumber;
			OrigObj.ids = &pOID->ids[m_iCompanyNumber];
			foundMib = FALSE;
			for (int i=0; pMib && ! foundMib; i++)
			{
				SnmpUtilOidCpy(&tempObj, pMib->GetOID());
				SnmpUtilOidToA(&tempObj);
				SnmpUtilOidToA(&OrigObj);
				int nCompResult = SnmpUtilOidCmp(&OrigObj, &tempObj);
				if (nCompResult < 0)
				{
					foundMib = TRUE;
				}
				else
				{
					pLastMib = pMib;
					pMib = pMib->pMibNext;
				}
			}
		}
		return (pLastMib);
	}
	catch(...)
	{
		return NULL;
	}

}
