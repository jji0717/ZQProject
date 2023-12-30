// TSClient.cpp : Defines the entry point for the DLL application.
//

#ifdef ZQ_OS_MSWIN
#include "stdafx.h"
#pragma warning(disable:4786)
#endif

#include "TSClient.h"
#include "SNMPOper.h"
#include "FileLog.h"
//#include "GridHandler.h"

#define LOG_MODULE_NAME			"TSClient"


static ZQ::common::Log			_nullLog;

#ifdef ZQ_OS_MSWIN

DWORD _iLogSlot; //tls index
#define LOGPTR          ((ZQ::common::Log*)(TlsGetValue(_iLogSlot)))
#define	TSLOG           (*LOGPTR)

static void ResetLogPtr(ZQ::common::Log* pLog = NULL)
{
    //clean current log object
    ZQ::common::Log* pCurrentLog = LOGPTR;
    if(pCurrentLog != NULL && pCurrentLog != (&_nullLog))
    {
        delete pCurrentLog;       
    }
    //set new log pointer
    ZQ::common::Log* pNewLog = pLog ? pLog : (&_nullLog);
    TlsSetValue(_iLogSlot, pNewLog);
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
            _iLogSlot = TlsAlloc();
            break;
		case DLL_THREAD_ATTACH:
            ResetLogPtr();
            break;
		case DLL_THREAD_DETACH:
            ResetLogPtr();
            break;
		case DLL_PROCESS_DETACH:
            TlsFree(_iLogSlot);
			break;
    }
    return TRUE;
}

#else

static ZQ::common::Log* currentLog;
#define LOGPTR (currentLog)
#define	TSLOG  (*LOGPTR)

static void ResetLogPtr(ZQ::common::Log* pLog=0) {
    currentLog = pLog ? pLog : (&_nullLog);
}

#endif

extern "C" {

TSCLIENT_API int TCInitialize(const char* szLogFile)
{
	try
	{
		ZQ::common::Log* pFileLog = new ZQ::common::FileLog(szLogFile, Log::L_DEBUG);
        ResetLogPtr(pFileLog);
	}
	catch(FileLogException& ex)
	{	
		printf("Fail to open log file %s, %s", szLogFile, ex.getString());
		return -1;
	}

	TSLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "TSClient initialized"));

	return 0;
}

TSCLIENT_API void TCUninitialize()
{
	TSLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "TSClient uninitialized"));
    ResetLogPtr();
}


TSCLIENT_API int TCFillGrid(PLayoutCtx ctx)
{
    if(NULL == ctx)
    {
        TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "null layout context pointer."));
        return 0;
    }

    TSLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "TCFillGrid() Not implemented!"));
    return 0;
// 
//     //step 1: get grid name from context
//     ILayoutCtx::lvalue val = {0};
//     if(!ctx->get("grid#gridname", val))
//     {
//         TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "GridName missed."));
//         return 0;
//     }
//     //step 2: get grid handler base on the grid name
//     GridHandler gh = GetGridHandler(val.value);
//     if(NULL == gh)
//     {
//         TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "unknown GridName, [GridName = %s]"), val.value);
//         return 0;
//     }
//     //step 3: dispatch call to the handler
//     int nRows = 0;
//     try{
//         nRows = gh(ctx);
//     }catch(...)
//     {
//         TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "catch exception from grid handler."));
//         return 0;
//     }
//     return nRows;
}


TSCLIENT_API int TCPopulateSnmpVariables(PLayoutCtx ctx, const char* baseOid, bool bPost, const char* snmpServer, const char *community)
{
	if (snmpServer==NULL)
		snmpServer = "127.0.0.1";

    if(NULL == community)
        community = "public";

	TSLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Request populate snmp vars, oid[%s], post[%d], snmpserver[%s]"),
		baseOid, int(bPost), snmpServer);

    SNMPOper  snmpOp;

    bool bInitSuccess = snmpOp.init(snmpServer, LOGPTR, community);

    if (!bInitSuccess)
    {
        TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to init SNMP"));
        ctx->clear(NULL); //cleanup
        return 0;
    }
	if (bPost)
	{
	    std::vector<std::string> oids;
	    std::vector<std::string> values;
        std::vector< int > types;

		char* pBuf = new char[SNMPATTR_VARNAMES_MAXLEN];
		char** pAttrname=NULL;
 		int nCount = ctx->list(pBuf, SNMPATTR_VARNAMES_MAXLEN, &pAttrname);
		if (nCount>0 && pAttrname)
		{
			for (int k=0; k<nCount; k++)
			{
				// step 1. chop out the oid from pAttrname
				ILayoutCtx::lvalue val;				
				if (!ctx->get(pAttrname[k], val))
				{
					//need to log?
					TSLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "Fail to get value from [%s]"), pAttrname[k]);
					continue;
				}
				
				if (val.readonly ||val.modify==SNMPATTR_VARVALUE_NOCHANGE)		
					continue;

				oids.push_back(val.oid);
				values.push_back(val.value);
                types.push_back(val.type);
				TSLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Request to modify [%s] to [%s]"), pAttrname[k], val.value);
			}
		}

		if (pBuf)
		{
			delete []pBuf;
			pBuf = NULL;
		}

        //prepare setVarValues parameters
		int nSetCount = oids.size();
        std::vector< const char* > pOids(nSetCount);
        std::vector< const char* > pVals(nSetCount);
        for(int i=0;i<nSetCount;i++)
		{
			pOids[i] = oids[i].c_str();
			pVals[i] = values[i].c_str();
		}

        const char** ppOids = nSetCount ? &(pOids[0]) : NULL;
		const char** ppValues = nSetCount ? &(pVals[0]) : NULL;
        int * pTypes = nSetCount ? &(types[0]) : NULL;

        // save the new value to SNMP
        bool bSetSuccess = snmpOp.setVarValues(ppOids, ppValues, pTypes, nSetCount);
        if (!bSetSuccess)
        {
            TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to modify the values"));
        }
	}

	
	// get all the values for all attributes and set to 
	{
		ctx->clear(NULL); //cleanup

		std::vector<SNMPOper::SNMPValue> values;
		bool bGetSuccess = snmpOp.getAllVarValue(baseOid, values);
		if (!bGetSuccess)
		{
			TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Fail to get the values"));
            return 0;
		}

        //store variables
        int nGotVars = values.size();
		for ( int i =0; i < nGotVars; i ++ )
		{
			ILayoutCtx::lvalue val;	
			val.modify = SNMPATTR_VARVALUE_NOCHANGE;
			val.type = values[i].type;
			strcpy(val.value, values[i].value);
			strcpy(val.oid, values[i].oid);
			val.readonly = values[i].readonly;
			ctx->set(values[i].name, val);
		}
        return nGotVars;
	}
}

}
