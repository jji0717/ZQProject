// ZQSNMPExtensionAgent.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "MibDescriptor.h"
#include "ZQSNMPExtensionAgent.h"
#include "MibDataOper.h"


extern UINT ProcessVariableBinding(BYTE	RequestType,RFC1157VarBind*		pVarBind);

/////////MIB Table ////////////////////////////////////////////////
SnmpVariable gItvOidServicesPrefix;
//FileReport   gFileReport(_T("SNMPExtension"));

/// This is the MIB table and its related variable store
//    Actualy it should be loaded from the registry or from some file
CMibDataOper    gMIBData; 
#ifdef FILELOGMODE
	ZQ::common::FileLog * m_pReporter;
#else
	MCB_t * hMClog;                        // Handle for mclog
#endif

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			#ifdef FILELOGMODE
				m_pReporter = NULL;
			#endif
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			#ifdef FILELOGMODE
				if(m_pReporter)
				{
					delete m_pReporter;
					m_pReporter = NULL;
				}
			#else
					MClogTerm(hMClog);			// Terminate MClog, releasing resources.
			#endif
			break;
    }
    return TRUE;
}

// When exported funtion will be called during DLL loading and initialization
BOOL SNMP_FUNC_TYPE SnmpExtensionInit(DWORD dwUptimeReference,
									  HANDLE *phSubagentTrapEvent,
									  AsnObjectIdentifier *pFirstSupportedRegion)
{
	// ... or No trap event
    *phSubagentTrapEvent = NULL;
	// init the MIB descriptors
	if (gMIBData.InitializeMibDescriptors())
	{

		#ifdef FILELOGMODE
			char strFile[MAL_LENTH] ={0};
			#ifdef _UNICODE
				WideCharToMultiByte(CP_ACP,NULL,gMIBData.m_strFile,-1,strFile,sizeof(strFile),NULL,NULL);
			#else
				sprintf(strFile,"%s",gMIBData.m_strFile);
			#endif
				m_pReporter = new ZQ::common::FileLog(strFile,ZQ::common::Log::L_INFO);
				(*m_pReporter).setVerbosity(ZQ::common::Log::L_INFO);
				(*m_pReporter)(ZQ::common::Log::L_INFO,_T("===================== SNMPExtensionInit Successful ======================"));
		#else
			hMClog = MClogInit(gMIBData.m_strFile,10,500000);
			MClog(hMClog,1,_T("===================== SNMPExtensionInit Successful ======================"));
		#endif
		
//		// Return the supported view.
		*pFirstSupportedRegion = gItvOidServicesPrefix.name;
		
	}
    else
	{
		#ifdef FILELOGMODE
			(*m_pReporter)(ZQ::common::Log::L_INFO,_T("===================== SNMPExtensionInit Failed   ======================"));
		#else
			MClog(hMClog,1,_T("===================== SNMPExtensionInit Failed   ======================"));
		#endif
	
		return FALSE;
	}

	return SNMPAPI_NOERROR;
}

// this export is to query the MIB table and fields
BOOL SNMP_FUNC_TYPE SnmpExtensionQuery(BYTE bPduType, 
									   SnmpVarBindList *pVarBindList, 
									   AsnInteger32 *pErrorStatus, AsnInteger32 *pErrorIndex)
{

	// Query counter.
	static unsigned long QueryNumber = 0;
	QueryNumber++;
	TCHAR				cMessage[5000];
	TCHAR*				pMsg;
	pMsg = cMessage;
	TCHAR *szPageType = NULL;
	if ( bPduType == 160 )
	{
		szPageType =_T("Snmp's Get Package");
	}
	else if ( bPduType == 161 )
	{
		szPageType =_T("Snmp's Get All Package");
	}
	else
	{
		szPageType =_T("Snmp's Set Package");
	}
	pMsg += _stprintf(pMsg, _T("TRACE : SnmpExtensionQuery(%d) requesttype=%d, PageType =%s, Number of variables=%d"), 
											QueryNumber, bPduType, szPageType,pVarBindList->len);

	#ifdef FILELOGMODE
			(*m_pReporter)(ZQ::common::Log::L_INFO,cMessage);
	#else
			MClog(hMClog,1,cMessage);
	#endif

			// below is the MultiExtensionAgent's Operation
	unsigned int		i;
    BOOL				QueryResult = SNMPAPI_NOERROR;

	 for (i = 0; i < pVarBindList->len; i++)
    {
		 // On a Get or GetNext retrieve a value from the MIB

		 // On a Get or GetNext retrieve a value from the MIB
		pMsg = cMessage;
		pMsg += _stprintf(pMsg, _T("Variable %d: list length=%d, OID="), i, pVarBindList->list[i].name.idLength);
		for (unsigned long j=0; j<pVarBindList->list[i].name.idLength; j++)
		{
			pMsg += _stprintf(pMsg, _T("%d."), pVarBindList->list[i].name.ids[j]);
		}
		#ifdef FILELOGMODE
			(*m_pReporter)(ZQ::common::Log::L_INFO,cMessage);
		#else
			MClog(hMClog,1,cMessage);
		#endif
		
//		gFileReport.writeMessage(cMessage,Log::L_INFO);
		
		// do a get/getnext, or set up to do a set
         *pErrorStatus = ProcessVariableBinding(bPduType, &pVarBindList->list[i]); 

 
        // Test and handle case where Get Next past end of MIB view supported
        // by this Extension Agent occurs.  Special processing is required to 
        // communicate this situation to the Extendible Agent so it can take 
        // appropriate action, possibly querying other Extension Agents.

		 
        if ((SNMP_ERRORSTATUS_NOSUCHNAME == *pErrorStatus) &&
             (MIB_ACTION_GETNEXT == bPduType))
        {
			*pErrorStatus = SNMP_ERRORSTATUS_NOERROR;


			// Modify variable binding of such variables so the OID points
			// just outside the MIB view supported by this Extension Agent.
			// The Extendible Agent tests for this, and takes appropriate
			// action.

			SnmpUtilOidFree( &pVarBindList->list[i].name);
			SnmpUtilOidCpy( &pVarBindList->list[i].name, gItvOidServicesPrefix.GetOID());
			pVarBindList->list[i].name.ids[gItvOidServicesPrefix.name.idLength-1] ++;
			
        }

        // Return any indication of error to the Extendible Agent 
        if (SNMP_ERRORSTATUS_NOERROR != *pErrorStatus)
        {
            *pErrorIndex = i + 1;	// Point to the varbind that cause the error 
			QueryResult = SNMPAPI_ERROR;
			break; // An error occurred. Stop processing the remaining varbinds 
	    }
	 }

	 
	if (bPduType == MIB_ACTION_SET &&
		SNMP_ERRORSTATUS_NOERROR == *pErrorStatus)
    {
	    for (i = 0; i < pVarBindList->len; i++)
		{
			// 
//			*pErrorStatus = SetNewValueInManPkg(&pVariableBindings->list[i]);
		}

		if (*pErrorStatus == SNMP_ERRORSTATUS_NOERROR)
		{
			//QueryResult = FlushMibVarCache();
			if (!QueryResult)
			{
				*pErrorStatus = SNMP_ERRORSTATUS_GENERR;
				*pErrorIndex = 1;
			}
		}
		else
		{
            //ClearMibVarCache();
		}
    }
	return(QueryResult);
}

// this function just simulate traps
//   Traps just a 2 variables value from MIB
//  Trap is kind of event from server to client
///         When ever the event is set service will call this function and gets the parameters filled.
//       After filling the parameters service willsend the trap to all the client connected
BOOL SNMP_FUNC_TYPE SnmpExtensionTrap(AsnObjectIdentifier *pEnterpriseOid, AsnInteger32 *pGenericTrapId, AsnInteger32 *pSpecificTrapId, AsnTimeticks *pTimeStamp, SnmpVarBindList *pVarBindList)
{
	return FALSE;
}






