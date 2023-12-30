// MODPlugIn.cpp : Defines the entry point for the DLL application.
//

#include "StdAfx.h"
#ifdef ZQ_OS_MSWIN
#include "ZQResource.h"
#endif
#include "LAMPlayListQuery.h"
#include "LAMPlayListQuery3.h"
#include "LSMSForMoDAuthorization.h"
#include "OTEAuthorization.h"
#include "ADMPlacement.h"
#include "AAAQuery.h"
#include "HeNanAAAQuery.h"
#include "Log.h"
#include "FileLog.h"
#include "FileSystemOp.h"
#include "SystemUtils.h"
#include "ModCfgLoader.h"
#define  MODPLUGINLOGFILE "GBMODPlugIn.log"

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
	}
	
//	_TRACE("DllMain(): reason = %d\n", ul_reason_for_call);
	
    return TRUE;
}
#endif

ZQAPPMOD::OTEAuthorization*  OteAuthorization  = NULL;
ZQAPPMOD::LAMPlayListQuery*  lamplaylistQuery  = NULL;
ZQAPPMOD::LAMPlayListQuery3*  lamplaylistQuery3 = NULL;
ZQAPPMOD::LSMSForMODAuthorization* LsmsAuthorization = NULL;
ZQAPPMOD::ADMPlacement* admpacement = NULL;
ZQAPPMOD::AAAQuery* aaaquery = NULL;
ZQAPPMOD::HeNanAAAQuery* henamaaaquery = NULL;
ZQAPPMOD::IMHOManager* mhoMgr = NULL;
ZQ::common::Log * PMHOlog = NULL;
::Ice::CommunicatorPtr _iceComm;

extern "C" {
void InitMHO(ZQAPPMOD::IMHOManager& mgr, void* pCtx, 
			      const char* configfile,const char* logFolder, const char * modinstanceID, Ice::CommunicatorPtr& ic)
{
	_TRACE("Entry InitializeMHO()..., NetID = %s\n",modinstanceID);
	
	ZQ::common::Config::Loader<MODCFGGROUP> gPlugInCfgGroup("MovieOnDemand.xml");
	int32 LogLevel = 7;
	int32 LogSize = 10240000;
	int32 LogCount = 5;

	if(gPlugInCfgGroup.load(configfile))
	{
		std::map< std::string, Config::Holder<ModCfg> >::iterator itor = gPlugInCfgGroup.modcfg.find(modinstanceID);
		if(itor != gPlugInCfgGroup.modcfg.end())
		{
			ZQ::common::Config::Holder<ModCfg>& gPlugInCfg = itor->second;
			LogLevel = gPlugInCfg.modPlugIn.LogLevel;
			LogSize = gPlugInCfg.modPlugIn.LogSize;
			LogCount = gPlugInCfg.modPlugIn.LogCount;
		}
	}
	std::string strLogfile;
#ifdef ZQ_OS_MSWIN
	if(_access(logFolder, 0))
#else
	if(access(logFolder, 0))
#endif
	{
		std::string strCurDir = FS::getImagePath();

		int nIndex = strCurDir.rfind(FNSEPC);
		strCurDir = strCurDir.substr(0,nIndex);
		nIndex = strCurDir.rfind(FNSEPC);
		strCurDir = strCurDir.substr(0,nIndex); //end with "\\"	
		strCurDir = strCurDir + FNSEPS "logs" FNSEPS;
		strLogfile = strCurDir + MODPLUGINLOGFILE;

		if(strcmp(modinstanceID, "1") == 0)
			strLogfile = strCurDir + MODPLUGINLOGFILE;
		else
		{
			strLogfile = strCurDir + "GBMODPlugIn_" ;
			strLogfile += modinstanceID;
			strLogfile += ".log";
		}
	}
	else
	{

		strLogfile = logFolder;
		size_t dwLength = strLogfile.length();
		if(strLogfile[dwLength -1] != FNSEPC)
		{
			strLogfile += FNSEPS;
		}
		if(strcmp(modinstanceID, "1") == 0)
		   strLogfile += MODPLUGINLOGFILE;
		else
		{
			strLogfile += "GBMODPlugIn_" ;
			strLogfile += modinstanceID;
			strLogfile += ".log";
		}
	}
	
	PMHOlog = new ZQ::common::FileLog(strLogfile.c_str(), LogLevel, LogCount, LogSize);
    
	if(!PMHOlog)
	{
		_TRACE("Init MHO Log file error!\n");
		return;
	}

#ifdef ZQ_OS_MSWIN
    _TRACE("This MHO_MODPlugin version is %d.%d.%d.%d", 
		ZQ_PRODUCT_VER_MAJOR,ZQ_PRODUCT_VER_MINOR,ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
#endif
    _TRACE("init MHO log successful, logpath = %s", strLogfile.c_str());


	if (mhoMgr)
		return;
	
	mhoMgr = &mgr;
	_iceComm = ic;

/*	try
	{
		_iceComm = Ice::initialize(__argc, __argv);
	}
	catch (Ice::Exception& ex)
	{
		glog("Initialize ICE communicat catch error at %s", 
			ex.ice_name().c_str());
		return;
	}

	if(!_iceComm)
	{
		glog("Initialize ICE communicat error");
		return;
	}
	_TRACE("Initialize ICE communicat successful");*/

/*	OteAuthorization = new ZQAPPMOD::OTEAuthorization(_iceComm);
	if(!OteAuthorization)
	{
		glog("Create OTEAuthorization object error");
		return;
	}

	lamplaylistQuery = new ZQAPPMOD::LAMPlayListQuery(_iceComm);

	if(!lamplaylistQuery)
	{
		glog("Create Get PlayList object error");
		lamplaylistQuery = NULL;
	}
	lamplaylistQuery3 = new ZQAPPMOD::LAMPlayListQuery3(_iceComm);
	if(!lamplaylistQuery3)
	{
        glog("Create Get PlayList3 object error");
		lamplaylistQuery3 = NULL;
	}

	LsmsAuthorization = new ZQAPPMOD::LSMSForMODAuthorization(_iceComm);
	if(!LsmsAuthorization)
	{
		glog("Create LSMSForMODAuthorization object error");
	}

	admpacement = new ZQAPPMOD::ADMPlacement(_iceComm);
	if(!admpacement)
	{
		glog("Create ADMPlacement object error");
	}*/

	aaaquery = new  ZQAPPMOD::AAAQuery();
	if(!aaaquery)
	{
		(*PMHOlog)("Create AAAQuery object error");
		aaaquery = NULL;
	}

	henamaaaquery = new  ZQAPPMOD::HeNanAAAQuery();
	if(!henamaaaquery)
	{
		(*PMHOlog)("Create HeNanAAAQuery object error");
		henamaaaquery = NULL;
	}
/*	if(OteAuthorization)
		mgr.registerAuthorization(OTE_Authorization_NAME, *OteAuthorization, pCtx);
	if(lamplaylistQuery)
		mgr.registerPlayListQuery(LAM_PlayList_Name, *lamplaylistQuery, pCtx);
	if(lamplaylistQuery3)
		mgr.registerPlayListQuery(LAM_PlayList3_Name, *lamplaylistQuery3, pCtx);
	if(LsmsAuthorization)
		mgr.registerAuthorization(LSMS_Authorization_NAME, *LsmsAuthorization, pCtx);
    if(admpacement)
		mgr.registerAdsReplacement(LAM_ADMPLACEMENT_Name, *admpacement, pCtx);*/
	if(aaaquery)
		mgr.registerAAA(LAM_AAAQUERY_Name, *aaaquery, pCtx);

	if(henamaaaquery)
		mgr.registerAAA(HENAN_AAAQUERY_Name, *henamaaaquery, pCtx);

	_TRACE("Leave InitializeMHO()");
}

void UnInitMHO(void)
{
	_TRACE("Entry UnInitializeMHO()...");
	
	try
	{	
		if (mhoMgr) 
		{
		/*	mhoMgr->unregisterAuthorization(OTE_Authorization_NAME);
			mhoMgr->unregistPlayListQuery(LAM_PlayList_Name);
			mhoMgr->unregistPlayListQuery(LAM_PlayList3_Name);
			mhoMgr->unregisterAuthorization(LSMS_Authorization_NAME);
			mhoMgr->unregistAdsReplacement(LAM_ADMPLACEMENT_Name);
			
			if(OteAuthorization)
			{
				delete OteAuthorization;
				OteAuthorization = NULL;
			}

			if(lamplaylistQuery)
			{
				delete lamplaylistQuery;
				lamplaylistQuery = NULL;
			}

			if(lamplaylistQuery3)
			{
				delete lamplaylistQuery3;
				lamplaylistQuery3 = NULL;
			}

			if(LsmsAuthorization)
			{
				delete LsmsAuthorization;
				LsmsAuthorization = NULL;
			}

			if(admpacement)
			{
				delete admpacement;
				admpacement = NULL;
			}*/
			mhoMgr->unregistAAA(LAM_AAAQUERY_Name);
			if(aaaquery)
			{
				delete aaaquery;
				aaaquery = NULL;
			}
			mhoMgr->unregistAAA(HENAN_AAAQUERY_Name);
			if(henamaaaquery)
			{
				delete henamaaaquery;
				henamaaaquery = NULL;
			}
		}
/*		if (_iceComm)
		{
			try 
			{
				_iceComm->destroy();
			} 
			catch (const Ice::Exception & ex)
			{
				_TRACE("destroy ICE communicat caught at %s\n", ex.ice_name().c_str());
			}
		}*/
		
		if(PMHOlog)
		{
//			ZQ::common::setGlogger(NULL);
			delete PMHOlog;
			PMHOlog = NULL;
		}
	}
	catch (...)
	{
	}
}
}

inline void logTrace(const char* lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	
	int nBuf;
	char szBuffer[1024];
	
	nBuf = vsprintf(szBuffer, lpszFormat, args);
	if (PMHOlog) {
		(*PMHOlog)(ZQ::common::Log::L_DEBUG, szBuffer);
	} else {
#ifdef ZQ_OS_MSWIN
		OutputDebugStringA(szBuffer);
#endif
	}
	
	va_end(args);
}
