
#ifndef _tianshan_ngod_service_control_header_file_h__
#define _tianshan_ngod_service_control_header_file_h__

#include <FileLog.h>
#include "NgodSessionManager.h"
#include "StreamSmithModule.h"
#include "StreamEventSinker.h"
// #include "SnmpQueryServantImpl.h"
#include "D5Update.h"

namespace NGOD
{
class NgodService
{
public:
	NgodService(void);
	virtual ~NgodService(void);

public:
	
	bool						start( IStreamSmithSite* pSite , const char* pConfPath ,const char* logfolder );
	
	void						stop( );

	RequestProcessResult		processRequest( IClientRequest* clireq );

	const std::string&			getErrMsg( ) const;

	ZQ::common::FileLog&        getFileLog() { return mMainLogger; };

	ZQ::common::FileLog&        getEventFileLog() { return mEventLogger; };

	ZQ::common::FileLog&        getIceFileLog() { return mIceLogger; };

	Ice::CommunicatorPtr        getCommunicator() { return mIc; };

protected:

	bool						initSelManager();

	bool						initNgodEnv( );

	bool						initSelectionEnv( );

	bool						loadConfig( const char* confPath );

	bool						initLogger( const char* logfolder );

	bool						initEventSinker( );

	bool						initIceRuntime( );

	bool                        initPublishedLogs( );

	void						uninitNgodEnv();

	void						uninitSelectionEnv();

	void						uninitLogger();

	void						uninitIceRuntime();

private:

	void						setErroMsg( const char* fmt , ... );


private:

	NgodEnv						mEnv;	
	NgodSessionManager			mSessManager;
	StreamEventDispatcher		mEventDispatcher;
	std::string					mErrMsg;
	StreamEventSinker			mEventSinker;
	// SnmpQueryServantImpl	    mSnmpServant;
	D5Speaker*					mD5Speaker;

	ZQ::common::FileLog			mMainLogger;
	ZQ::common::FileLog			mEventLogger;
	ZQ::common::FileLog			mIceLogger;
	Ice::CommunicatorPtr		mIc;
	bool						mbStopped;

};

extern NGOD::NgodService ngodService;

}//namespace NGOD

#endif//_tianshan_ngod_service_control_header_file_h__
