#ifndef __c2client_command_header_file_h__
#define __c2client_command_header_file_h__

#include "CommandCenter.h"
#include "HttpProtocol.h"
#include "HttpSession.h"
#include "HttpDialog.h"
#include "ConnectionFactory.h"
#include <NativeThread.h>
#include <Locks.h>

class ClientCommand;
class StatisticsRunner : public ZQ::common::NativeThread
{
public:
	StatisticsRunner( ClientCommand& cc );
	virtual ~StatisticsRunner();

public:

	void	stop( );
	void	reset( );
	void	startCounting( HttpSessionFactoryPtr sessfac );
	void	stopCouting( );

protected:

	int		run();

private:

	ClientCommand&	mCC;
	bool	mbQuit;
	bool	mbCouting;
	int64	lastBytes;
	int64	totalBytes;
	int64	lastTime;
	int64	totalTime;
	ZQ::common::Semaphore mSem;
	ZQ::common::Mutex mMutex;
	HttpSessionFactoryPtr mSessfac;
};

class ClientCommand 
{
public:
	ClientCommand(void);
	virtual ~ClientCommand(void);

public:

	int		process( );
	
	void	initialize( );

	void	output( const char* fmt , ... );

	void	output( const std::string& str , ... );

public:

	void	onCycleStart( );
	void	onCycleStop( );
	
	int		createC2Session( OptResult& opts );
	int		createHttp( OptResult& opts );
	int		show( OptResult& opts );
	int		quit( OptResult& opts );
	

protected:

	void	initCallback();

	

private:
	ZQ::common::Log		mNullogger;
	ClientService		mClientService;
	CommandCenter		mCmdRunner;
	std::string			mPromotion;	
	StatisticsRunner	mStatRunner;
};

#endif//__c2client_command_header_file_h__
