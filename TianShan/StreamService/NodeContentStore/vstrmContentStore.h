
#ifndef _ZQ_STREAMSMITH_CONTENT_STORE_HEADER_FILE_H__
#define _ZQ_STREAMSMITH_CONTENT_STORE_HEADER_FILE_H__

#include <Guid.h>
#include <locks.h>
#include <list>
#include <Log.h>
#include <NativeThread.h>
extern "C"
{
#include <vstrmuser.h>
}

#include "ContentImpl.h"




namespace ZQTianShan
{
namespace ContentStore
{


class ReplicaStatusReporter : public ZQ::common::NativeThread
{
public:
	ReplicaStatusReporter(  );
	~ReplicaStatusReporter( );
public:

	void		set( ZQTianShan::ContentStore::ContentStoreImpl::Ptr	store, const std::string& subScriberEndpoint ,	int	interval )
	{
		_store = store;
		replicaListenerEndpoint = subScriberEndpoint;
		defaultUpdateInterval	= interval;
		if( defaultUpdateInterval < 10 * 1000)
			defaultUpdateInterval = 10 * 1000;
	}

	void		stop( );
	int			run( );
	void		runTimerTask(void);	

private:
	HANDLE			_hQuitEvent;
	bool			_bQuit;
	int				updateInterval;
	int				defaultUpdateInterval;
	std::string		replicaListenerEndpoint;
	ZQTianShan::ContentStore::ContentStoreImpl::Ptr		_store;
};


struct PortalCtx;
class vstrmFileSystemSink : public ZQ::common::NativeThread
{

public :
	vstrmFileSystemSink( ZQTianShan::ContentStore::ContentStoreImpl& st , PortalCtx* ctx );
	~vstrmFileSystemSink( );

public:

	void		attchVstrmHandle( VHANDLE vstrmHandle );

	void		stop( );

	VHANDLE		getVstrmHandle( );
private:

	bool		init(void);
	int			run(void);
	void		final(void);

private:
	
	bool					_bQuit;
	ZQTianShan::ContentStore::ContentStoreImpl&		store;

	VHANDLE					_vstrmHandle;
	PortalCtx*				_portCtx;
};

class DeleteLaterProcudure : public ZQ::common::ThreadRequest
{
public:
	DeleteLaterProcudure( ZQ::common::NativeThreadPool& pool ,
							PortalCtx* ctx,
							ContentStoreImpl& store);
	virtual ~DeleteLaterProcudure( );

	void		pushContent( const std::string& contentName );

	void		stop( );

protected:
	
	bool		init(void);

	int			run( );

	void		final(int retcode , bool bCancelled );

	const uint8 getPriority(void) { return 100; }

protected:

	std::string		extractFileName( const std::string& fileName ) const;

private:
	ContentStoreImpl&					store;	
	PortalCtx*							portalCtx;
	bool								mbQuit;	
	ZQ::common::Mutex					mMutex;
	typedef std::list<std::string>		CONTENTLIST;
	CONTENTLIST							mCtntList;
	std::string							mCurContent;
	bool								mbRunning;
	int32								mExecCount;
};

void	addContentToLaterDeleteProcudure( const std::string& contentName );

class vstrmContentStoreImpl :public ZQTianShan::ContentStore::ContentStoreImpl
{
public:
	vstrmContentStoreImpl(	ZQ::common::Log& log, ZQ::common::Log& eventlog, 
				ZQ::common::NativeThreadPool& threadPool,
				ZQADAPTER_DECLTYPE& adapter,
				const char* databasePath =NULL );
		
	virtual void OnVolumeMounted(const ::Ice::Identity& identVolume, const ::std::string& path) ;
};
}}//namespace ZQ::StreamSmith

#endif //_ZQ_STREAMSMITH_CONTENT_STORE_HEADER_FILE_H__
