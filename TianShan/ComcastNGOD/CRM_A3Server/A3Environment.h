// FileName : A3Environment.h
// Author   : Zheng Junming
// Date     : 2009-05
// Desc     : A3Environment mainly load configuration, create log and initial ice runtime

#ifndef __CRG_PLUGIN_A3SERVER_ENVIRONMENT_H__
#define __CRG_PLUGIN_A3SERVER_ENVIRONMENT_H__

#include "TianShanDefines.h"
#include "FileLog.h"
#include "TsStorage.h"
#include "IceLog.h"
#include "A3ModuleImpl.h"
#include "EventChannel.h"

namespace CRG
{
namespace Plugin
{
namespace A3Server
{

// forward declare

class A3MsgHandler;

class A3Client;

class Environment
{
public:
	Environment();
	~Environment();

public:
	bool doInit(const std::string& strLogPath, const std::string& strCfgPath);

	/// this method must be called no matter if doInit() success or fail
	void doUninit();
private:
	bool loadConfig(const std::string& strCfgPath);

	bool initIceRuntime(const std::string& strLogPath, Ice::CommunicatorPtr& pCommunicator, 
		ZQADAPTER_DECLTYPE& pAdapter);
	void uninitIceRuntime();

	bool initFreezeEvictor(A3ContentFactoryPtr& factory, A3Module::AssetIdxPtr& assetIdx, 
		A3Module::VolumeIdxPtr& volumeIdx, Freeze::EvictorPtr& a3Content);
	void closeFreezeEvictor();

	bool connectContentStore(std::map<std::string, TianShanIce::Storage::ContentStorePrx>& contentStoreProxies);
	bool connectIceStorm(TianShanIce::Events::GenericEventSinkPtr& a3Event);

private:
	/// forbidden copy and assign
	Environment(const Environment& env);
	Environment& operator=(const Environment& env);

public:
	A3MsgHandler* _A3MsgHandler;
	ZQADAPTER_DECLTYPE _pAdapter;

private: 
	ZQ::common::SysLog _sysLog;
	ZQ::common::FileLog _fileLog;

private: // ice relative member
	ZQ::common::FileLog _iceFileLog;
	TianShanIce::common::IceLogIPtr _iceLogger;
	Ice::CommunicatorPtr _pCommunicator;
	std::map<std::string, TianShanIce::Storage::ContentStorePrx> _contentStoreProxies;

private: // freeze relative member
	Freeze::EvictorPtr _a3Content;
	A3Module::AssetIdxPtr _assetIdx;
	A3Module::VolumeIdxPtr _volumeIdx;
	A3ContentFactoryPtr _factory;

private:
	TianShanIce::Events::EventChannelImpl::Ptr _eventChannel;
	A3Client* _a3Client;
	A3FacedeIPtr _a3FacedeIPtr;
};

} // end for A3Server
} // end for Plugin
} // end for CRG

#endif // end for __CRG_PLUGIN_A3SERVER_ENVIRONMENT_H__


