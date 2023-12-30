// Ftp_Svr.cpp: implementation of the Ftp_Svr class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4503)
#include "FtpServer.h"
#include "FtpSite.h"
#include <string>
#include "utils.h"
#include "CPECfg.h"
#include "FileLog.h"
#include "CPEEnv.h"
#include "HostToIP.h"

#define FtpSrv			"FtpSrv"

#define MOLOG		glog

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FtpServer::FtpServer(CPEEnv& env):_env(env)
{
	_site = NULL;
	_threadPool = NULL;
}

FtpServer::~FtpServer()
{
	Uninitialize();
}

bool FtpServer::Initialize()
{
 	if (!_gCPECfg._ftpBindIP[0]) {
 		strcpy(_gCPECfg._ftpBindIP, "0.0.0.0");
 	}

	std::string ftpBindIp;// = GetIp(_gCPECfg._ftpBindIP);
	if (!HostToIP::translateHostIP(_gCPECfg._ftpBindIP, ftpBindIp))
		ftpBindIp = _gCPECfg._ftpBindIP;

	_threadPool = new NativeThreadPool(_gCPECfg._ftpThreadPoolSize);	
	_site = new FtpSite(*_threadPool);
 	_site->setBindIP(ftpBindIp.c_str());
 	_site->setListenPort(_gCPECfg._ftpListenPort);
 	_site->setHomeDir(_gCPECfg._homeDir);
	_site->setValidatePushCB(validatePush, this);
  _site->setMaxConnection(_gCPECfg._maxConnection);
  
	MOLOG(Log::L_INFO, CLOGFMT(FtpSrv, "Ftp server initialized on IP[%s], port[%d]"),
		ftpBindIp.c_str(), _gCPECfg._ftpListenPort);	//orig para is _gCPECfg._ftpBindIP;
	
	return true;
}

bool
FtpServer::Start()
{
	if (!_site->Initialize()) {
		MOLOG(Log::L_ERROR, CLOGFMT(FtpSrv, "Failed to start ftp server"));	
		return false;	
	}
	
	MOLOG(Log::L_INFO, CLOGFMT(FtpSrv, "Ftp server started"));	

	return true;
}

bool FtpServer::Stop()
{
	if (_site)
	{
		_site->Stop();
		_site->Uninit();
	}

	return true;
}

void FtpServer::Uninitialize()
{
	if (_site)
	{		
		delete _site;
		_site = NULL;

		MOLOG(Log::L_INFO, CLOGFMT(FtpSrv, "Ftp server uninitialized"));	
	}

	if (_threadPool)
	{
		delete _threadPool;
		_threadPool = NULL;
	}
}

IPushSource* FtpServer::findPushSource(const char* contentStoreNetId, 
									   const char* volume,
										const char* content)
{
	if (!_site)
		return NULL;

	std::string contentKey = _site->makeContentKey(contentStoreNetId,
		volume,
		content);

	return _site->findPushXfer(contentKey.c_str());
}

bool FtpServer::validatePush(void* pCtx, const char* szNetId, const char* szVolume, const char* szContent)
{
	if (pCtx)
		return ((FtpServer*)pCtx)->validatePush(szNetId, szVolume, szContent);

	return false;
}

bool FtpServer::validatePush(const char* szNetId, const char* szVolume, const char* szContent)
{
	MOLOG(Log::L_INFO, CLOGFMT(FtpSrv, "validatePush with netid[%s], volume[%s], content[%s]"), szNetId, szVolume, szContent);	
	
	std::string strContentProvisionSvc = std::string(SERVICE_NAME_ContentProvisionService ":") + _gCPECfg._cpeEndPoint;
	TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx = 
		TianShanIce::ContentProvision::ContentProvisionServicePrx::uncheckedCast(_env._communicator->stringToProxy(strContentProvisionSvc));

	if (!cpePrx)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FtpSrv, "Failed to open ContentProvisionService with endpoint: [%s]"), strContentProvisionSvc.c_str());	
		return false;
	}
	
	::Ice::Long c = ZQTianShan::now();
	char buf[32];
	sprintf(buf, FMT64, c++);
	TianShanIce::ContentProvision::ProvisionContentKey contentKey;
	contentKey.content = szContent;
	contentKey.contentStoreNetId = szNetId;
	contentKey.volume = szVolume;
	
	TianShanIce::ContentProvision::ProvisionSessionPrx sess;
	bool bOpenSucc = false;
	try
	{
		do
		{
			sess = cpePrx->openSession(contentKey);
			if (!sess)
			{
				MOLOG(Log::L_INFO, CLOGFMT(FtpSrv, "can not find session with contentKey(%s|%s|%s)"), 
					contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), contentKey.content.c_str());	
				break;
			}

			bOpenSucc = true;
		}while(0);
	}
	catch(Ice::Exception& ex)
	{
		MOLOG(Log::L_INFO, CLOGFMT(FtpSrv, "can not find session with contentKey(%s|%s|%s), ice exception caught: %s"), 
			contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), contentKey.content.c_str(), ex.ice_name().c_str());	
	}

	try
	{
		TianShanIce::ContentProvision::ProvisionSessionExPrx sessEx;
		sessEx = TianShanIce::ContentProvision::ProvisionSessionExPrx::uncheckedCast(sess);
		sessEx->forceToStart();
	}
	catch(Ice::Exception& ex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FtpSrv, "Failed to forceToStart session with contentKey(%s|%s|%s), ice exception caught: %s"), 
			contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), contentKey.content.c_str(), ex.ice_name().c_str());	
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(FtpSrv, "validatePush with netid[%s], volume[%s], content[%s] successful"), szNetId, szVolume, szContent);	
	return true;
}
