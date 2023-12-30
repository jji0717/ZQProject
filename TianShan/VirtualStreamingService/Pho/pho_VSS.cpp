// pho_NGOD.cpp : Defines the entry point for the DLL application.
//

#include "TianShanIceHelper.h"
#include "PhoNGOD.h"
#include "PhoNGOD_DVBC.h"
#include "PhoCisco.h"
#include "PhoHsnTree.h"
#include "phoStorageLink.h"
#include "FileLog.h"
#include "Configuration.h"
#include "FileSystemOp.h"

#ifdef ZQ_OS_MSWIN
extern "C" {
#include <stdafx.h>
}
#endif


#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::IpEdgePHO > IpEdgePHOPtr;
typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::IpEdgePHO_DVBC > IpEdgePHO_DVBCPtr;
typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::PHOCisco > PHOCiscoPtr;
typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::PHOHsnTree > PHOHsnTreePtr;
typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::NssC2Transfer> NssC2TransferPtr;

IpEdgePHOPtr			_gIpEdgePHOPtr		= NULL;
IpEdgePHO_DVBCPtr		_gIpEdgePHO_DVBCPtr = NULL;
PHOCiscoPtr				_gPHOCiscoPtr		= NULL;
PHOHsnTreePtr			_gPHOHsnTreePtr		= NULL;
NssC2TransferPtr		_gPHOC2Transfer		= NULL;

bool bInService = false;
ZQ::common::Config::Loader<PHOConfig>       cfg("Weiwoo.xml");
ZQ::common::Config::Loader<PhoVssConf>		phoConfig("pho_vss.xml");

extern "C" __EXPORT void InitPHO( ZQTianShan::AccreditedPath::IPHOManager& mgr,
									void* pCtx,
									const char* configFile,
									const char* logFolder)
{
	if (bInService) 
		return;	

	std::string		strLogFolder;
	std::string		strLogFileName	= "pho_VSS.log";
	long			logFileSize		= 50*1024*1024;
	long			logBufferSize	= 204800;
	long			logWriteTimeout	= 2;
	long			logLevel=7;

	if(configFile&&strlen(configFile)>0)
	{
		//read the config file
		if(cfg.load(configFile))
		{
			strLogFileName=cfg.szPHOLogFileName;
			logFileSize=cfg.lPHOLogFileSize;
			logBufferSize=cfg.lPHOLogBufferSize;
			logWriteTimeout=cfg.lPHOLogWriteTimteout;				
			logLevel = cfg.lPHOLogLevel;
		}
	}

	if( logFolder&&strlen(logFolder) > 0 )
	{
		strLogFolder=logFolder;
	}
	else
	{
		strLogFolder=FS::getImagePath();
		std::string::size_type iPos=strLogFolder.rfind(FNSEPC);
		if(iPos!=std::string::npos)
		{
			strLogFolder=strLogFolder.substr(0,iPos);
		}
	}

	if(  !(strLogFolder.at(strLogFolder.length()-1)==FNSEPC))
	{
		strLogFolder+=FNSEPC;
	}

	ZQ::common::FileLog* phoVSSLog = new ZQ::common::FileLog( (strLogFolder+strLogFileName).c_str(),
											logLevel, ZQLOG_DEFAULT_FILENUM, logFileSize, logBufferSize, logWriteTimeout);

	ZQ::common::setGlogger(phoVSSLog);
	std::string phoConfigurationFilePath = "pho_vss.xml";
	if( !cfg.phoConf.phoVssConfPath.empty( ) )
	{
		phoConfigurationFilePath = cfg.phoConf.phoVssConfPath;
	}

	phoConfigurationFilePath = ZQTianShan::Util::fsConcatPath( ZQTianShan::Util::fsGetParentFolderPath(configFile) , phoConfigurationFilePath );
	phoConfig.setLogger(phoVSSLog);
	if( !phoConfig.load(phoConfigurationFilePath.c_str() ) )
	{
		glog(ZQ::common::Log::L_WARNING,CLOGFMT(pho_VSS,"failed to load configuration file[%s]"), phoConfigurationFilePath.c_str() );
	}
	
	if (!_gIpEdgePHOPtr)
		_gIpEdgePHOPtr = new ZQTianShan::AccreditedPath::IpEdgePHO(mgr);

	if ( _gIpEdgePHOPtr )
	{
		mgr.registerStreamLinkHelper(STRMLINK_TYPE_NGOD, *_gIpEdgePHOPtr, pCtx);
		mgr.registerStreamLinkHelper(STRMLINK_TYPE_NGOD_SHARELINK, *_gIpEdgePHOPtr, pCtx);

	}
	else
	{
		glog(ZQ::common::Log::L_ERROR , CLOGFMT(pho_VSS,"create IpEdgePHO failed"));
	}

	if (!_gIpEdgePHO_DVBCPtr)
		_gIpEdgePHO_DVBCPtr = new ZQTianShan::AccreditedPath::IpEdgePHO_DVBC(mgr);

	if ( _gIpEdgePHO_DVBCPtr )
	{
		mgr.registerStreamLinkHelper(STRMLINK_TYPE_NGOD_DVBC, *_gIpEdgePHO_DVBCPtr, pCtx);
		mgr.registerStreamLinkHelper(STRMLINK_TYPE_NGOD_DVBC_SHARELINK, *_gIpEdgePHO_DVBCPtr, pCtx);
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR , CLOGFMT(pho_VSS,"create IpEdgePHO_DVBC failed"));
	}

	//if (!_gPHOCiscoPtr)
	//	_gPHOCiscoPtr = new ZQTianShan::AccreditedPath::PHOCisco(mgr);
	//if ( _gPHOCiscoPtr )
	//{
	//	mgr.registerStreamLinkHelper(STRMLINK_TYPE_CISCO, *_gPHOCiscoPtr, pCtx);
	//}
	//else
	//{
	//	glog(ZQ::common::Log::L_ERROR , CLOGFMT(pho_VSS,"create PHOCisco failed"));
	//}

	if (!_gPHOHsnTreePtr)
		_gPHOHsnTreePtr = new ZQTianShan::AccreditedPath::PHOHsnTree(mgr);

	if ( _gPHOHsnTreePtr )
	{
		mgr.registerStreamLinkHelper( STRMLINK_TYPE_HsnTree, *_gPHOHsnTreePtr, pCtx);
		_gPHOHsnTreePtr->init();
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR , CLOGFMT(pho_VSS,"create PHOHsnTree failed"));
	}

	if( !_gPHOC2Transfer )
		_gPHOC2Transfer = new ZQTianShan::AccreditedPath::NssC2Transfer(mgr);

	if ( _gPHOC2Transfer )
	{
		mgr.registerStorageLinkHelper( STORLINK_TYPE_C2TRANSFER, *_gPHOC2Transfer, pCtx);
		mgr.registerStorageLinkHelper( STORLINK_TYPE_C2TRANSFER_STANDBY, *_gPHOC2Transfer, pCtx);
		mgr.registerStorageLinkHelper( STORLINK_TYPE_C2OVERAQUA, *_gPHOC2Transfer, pCtx);
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR , CLOGFMT(pho_VSS, "create PHOC2Transfer failed"));
	}
	
	bInService = ( _gIpEdgePHOPtr && _gIpEdgePHO_DVBCPtr && _gPHOHsnTreePtr && _gPHOC2Transfer);

}

extern "C" __EXPORT  void UninitPHO(void)
{
	ZQ::common::Log* phoVSSLog = ZQ::common::getGlogger();
	ZQ::common::setGlogger(NULL);
	if(phoVSSLog)
	{
		try
		{
			delete phoVSSLog;
			phoVSSLog = NULL;
		}
		catch (...)
		{
		}
	}

	bInService			= false;	
	_gIpEdgePHOPtr		= NULL;
	_gIpEdgePHO_DVBCPtr = NULL;
	_gPHOCiscoPtr		= NULL;
	
	_gPHOHsnTreePtr->uninit();
	_gPHOHsnTreePtr		= NULL;
	_gPHOC2Transfer		= NULL;

}
