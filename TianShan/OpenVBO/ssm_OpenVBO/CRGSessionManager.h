// File Name : CRGSessionManager.h

#ifndef __SSM_OpenVBO_CRGSessionManager_H__
#define __SSM_OpenVBO_CRGSessionManager_H__

// ZQ Common
#include "Log.h"
#include "NativeThread.h"

// TianShan/Common
#include "TianShanDefines.h"

// generated ICE file from SsmOpenVBO.ICE
#include "SsmOpenVBO.h"
#include "StreamIdx.h"
#include "CRGSessionImpl.h"

namespace EventISVODI5
{

class Environment;

class CRGSessoionManager : public ZQ::common::NativeThread
{
public:
	CRGSessoionManager(Environment&env, ZQ::common::Log& fileLog, ::Ice::CommunicatorPtr& pCommunicator,
		ZQADAPTER_DECLTYPE& pEventAdapter);

	~CRGSessoionManager();

public:
	/// add session into evictor
	/// @param bUpdateStreamer, update streamer statistic
	bool addSession(const SsmOpenVBO::CRGSessionPtr& sessionPtr, std::string& strLastError);

	/// remove session from evictor
	/// @param bUpdateStreamer, update streamer statistic
	bool removeSession(const std::string& sessId, std::string& strLastError, bool bUpdateStreamer = true);
	bool removeSession(CRGSessionImplPtr sessImpl, std::string& strLastError, bool bUpdateStreamer = true);

	/// get session proxy
	SsmOpenVBO::CRGSessionPrx findSession(const std::string& sessId, std::string& strLastError, int& statusCode);

	/// open database, create evictor and update sessions in storage
	bool openDB(std::string& databasePath, int32 evictorSize);

	/// close database
	void closeDB();

    void updateDbEnvConfig( const std::string& env, const std::string& key, const std::string& value );
    void updateDbFileConfig( const std::string& env, const std::string& file ,const std::string& key, const std::string& value );

	int64 getTotalSessionsNum();

	void getSessionlist(std::string& sessionList);

	::std::vector< Ice::Identity > findStreams(const ::std::string&uid, Ice::Int index) const;

	::Ice::Identity getIdentity(const std::string& sessId);

	SsmOpenVBO::CRGSessionPrx getSessionContext(const std::string& sessId, TianShanIce::Properties& sessionContext);

	void stop();

protected:
	virtual bool init(void);

	virtual int run(void);

	virtual void final();

	/// whether session's stream exist
 	bool pingStreamOfSession(const std::string& sessId, TianShanIce::Properties& metaData);

private:
	Environment& _env;
	ZQ::common::Log& _fileLog;
	::Ice::CommunicatorPtr& _pCommunicator;
	ZQADAPTER_DECLTYPE& _pEventAdapter;

private:
	::Freeze::EvictorPtr _pContextEvtr;
	std::string _dbPath;
	std::string _programRootPath;

private: // freeze relative properties
	Ice::ObjectFactoryPtr _pFactory;
	SsmOpenVBO::StreamIdxPtr _pStreamIdx;
	bool _bQuit;
};

} // end EventISVODI5

#endif // end __SSM_OpenVBO_CRGSessionManager_H__
