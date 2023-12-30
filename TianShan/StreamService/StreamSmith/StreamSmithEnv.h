
#ifndef _zq_StreamService_StreamSmith_Environment_header_file__
#define _zq_StreamService_StreamSmith_Environment_header_file__

#include <SsEnvironment.h>
#include "VstrmSessionCallbackManager.h"
#include "VstrmStreamerManager.h"
#include "VstrmSessionScaner.h"
#include <IdxFileParserEnvironment.h>

namespace ZQ
{
namespace StreamService
{

class StreamSmithEnv : public SsEnvironment
{
public:
	
	StreamSmithEnv();
	~StreamSmithEnv();

	
	inline VstrmSessionCallbackManager&		getCallBackManager( )
	{
		return mVstrmSessionCBManager;
	}

	inline VstrmStreamerManager&			getStreamerManager( )
	{
		return mStreamerManager;
	}

	inline VstrmSessionScaner&				getSessionScaner( )
	{
		return mSessionScaner;
	}

	TianShanIce::Storage::ContentStorePrx	getCsPrx( )
	{
		return mCsPrx;
	}

	RemoteSessionMonitor&					getRemoteSessionMonitor( )
	{
		return mStreamerManager.getRSMonitor( );
	}

	ZQ::IdxParser::IdxParserEnv&			getIdxParserEnv( )
	{
		return mIdxParserEnv;
	}

public:
	ZQ::IdxParser::IdxParserEnv					mIdxParserEnv;	
	VstrmSessionCallbackManager					mVstrmSessionCBManager;
	VstrmStreamerManager						mStreamerManager;
	VstrmSessionScaner							mSessionScaner;
	TianShanIce::Storage::ContentStorePrx		mCsPrx;	
};


#define SERVER_MODE_NPVR			1
#define SERVER_MODE_EDGE			2
#define SERVER_MODE_NORMAL			0

}}

#endif//_zq_StreamService_StreamSmith_Environment_header_file__
