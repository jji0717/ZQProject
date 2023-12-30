#ifndef _zq_streamsmith_service_header_file_h__
#define _zq_streamsmith_service_header_file_h__

#include <BaseZQServiceApplication.h>
#include <FileLog.h>
#include <NativeThread.h>
#include <IceLog.h>
#include <StreamSmithEnv.h>
#include <VstrmStreamerManager.h>

class StreamSmithService: public ZQ::common::BaseZQServiceApplication , public ZQ::common::NativeThread
{
public:
	StreamSmithService();
	virtual ~StreamSmithService();

protected:

	HRESULT OnInit();

	HRESULT OnStart();

	HRESULT OnStop();

	HRESULT OnUnInit();	

	void	OnSnmpSet(const char *varName);

protected:

	bool	InitializeIceRunTime( );
	
	void	UninitializeIceRunTime( );

	bool	initializeCrashDumpLocation( );

	bool	initializeLogger( );

	void	uninitializeLogger( );

	bool	initializeServiceParameter( );

	int		run( );

	std::string	getLoggerFolder( ) const;

	void	adjustConfiguration( );

private:

	Ice::CommunicatorPtr				mIc;
	ZQ::common::FileLog					mIceFileLogger;
	ZQ::common::FileLog					mSessionLogger;
	ZQ::StreamService::StreamSmithEnv	mStreamSmithEnv;
	ZQ::StreamService::SsServiceImpl*	mpServiceImpl;
};



#endif//_zq_streamsmith_service_header_file_h__

