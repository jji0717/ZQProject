
#ifndef _Cdn_streamservice_environment_header_file_h__
#define _Cdn_streamservice_environment_header_file_h__


#include <SsEnvironment.h>
#include "CdnStreamerManager.h"
#include "CdnSessionScan.h"
#include <TsStorage.h>

namespace ZQ
{
namespace StreamService
{

class CdnSsEnvironment : public SsEnvironment
{
public:
	CdnSsEnvironment( ZQ::common::Log& mainLog, ZQ::common::Log& sessLog , ZQ::common::NativeThreadPool& pool);
	virtual ~CdnSsEnvironment( );
public:
	inline const std::string&	TransferServerHttpIp() const
	{
		return mTransferServerHttpIp;
	}
	inline const std::string&	TransferServerHttpPort( ) const
	{
		return mTransferServerHttpPort;
	}
	inline const std::string&	getNetId( ) const
	{
		return mNetId;
	}
	
	inline CdnStreamerManager&			getStreamerManager( )
	{
		return *mStreamerManager;
	}
	
	TianShanIce::Storage::ContentStorePrx getCsPrx()
	{
		return mCsPrx;
	}
	inline ZQ::common::Log*					getHttpLog()
	{
		return mHttpLog;
	}
	inline const std::string&			getC2DocRoot( ) const
	{
		return mC2StreamerDocRoot;
	}

public:
	std::string									mTransferServerHttpIp;
	std::string									mTransferServerHttpPort;
	std::string									mNetId;
	std::string									mC2StreamerDocRoot;
	CdnStreamerManager*							mStreamerManager;
	TianShanIce::Storage::ContentStorePrx		mCsPrx;	
	ZQ::common::Log*							mHttpLog;
};

#define HTTPLOG	(*(env->getHttpLog()))
}}
#endif//_Cdn_streamservice_environment_header_file_h__
