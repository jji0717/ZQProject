
#ifndef _ZQ_STREAMSMITH_CONTENT_STORE_HEADER_FILE_H__
#define _ZQ_STREAMSMITH_CONTENT_STORE_HEADER_FILE_H__

#include <Guid.h>
#include <locks.h>
#include <list>
#include <Log.h>
#include <NativeThread.h>
#include <TianShanIceHelper.h>
#include <UDPSocket.h>

extern "C"
{
#include <VstrmVer.h>
#include <vstrmuser.h>
}

#include "ContentImpl.h"


#if (VER_PRODUCTVERSION_MAJOR >= 6) &&  ( ( (VER_PRODUCTVERSION_MINOR == 3) && (VER_PRODUCTBUILD >= 9792) ) || (VER_PRODUCTVERSION_MINOR > 3) )
	#define SUPPORT_VSTRM_VSIS_EVENT
#endif//


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
	vstrmFileSystemSink( ZQTianShan::ContentStore::ContentStoreImpl& st , PortalCtx* ctx ,bool bSupportNpvr);
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
	bool					_bSupportNpvr;
};


#ifdef SUPPORT_VSTRM_VSIS_EVENT
class VsisEventMultiReceiver : public ZQ::common::NativeThread
{
public:

	VsisEventMultiReceiver( ZQTianShan::ContentStore::ContentStoreImpl& s );
	virtual ~VsisEventMultiReceiver();

public:

	struct VsisEventInfo 
	{
		std::string			magic;
		std::string			nodeNetId;
		std::string			clusterId;
		std::string			assetName;
		VSTRM_VSIS_EVENT	vsisEvent;

		VsisEventInfo();
		size_t		dataSize( ) const;
		bool		toBuffer( char* buffer , size_t& bufSize ) const;
		bool		fromBuffer( const char* buffer , size_t bufSize );
	};

	void	onEvent( VsisEventInfo& info );

	bool	start( const std::string& nodeNetId , const std::string clusterId , const std::string& localIp ,const std::string& groupIp , int groupPort );

	void	stop( );

protected:

	int		run();

	void	dispatchEvent( const VsisEventInfo& info , bool bMulticast = false);

	void	multicastEvent( const VsisEventInfo& info );

	void	clearResource( );

private:
	
	ZQTianShan::ContentStore::ContentStoreImpl&		store;
	ZQ::common::UDPReceive*		mReceiver;	
	ZQ::common::UDPMulticast*	mMulticaster;
	bool						mbQuit;
	std::string					mNodeNetId;
	std::string					mClusterId;	
};

class VsisEventSinker : public ZQ::common::NativeThread
{
public:
	VsisEventSinker( ZQTianShan::ContentStore::ContentStoreImpl& st , PortalCtx* ctx );
	virtual ~VsisEventSinker();
	void	stop();
	bool	start();
protected:
	int run();
private:
	bool											mbQuit;
	ZQTianShan::ContentStore::ContentStoreImpl&		store;

	VHANDLE											mVstrmHandle;
	PortalCtx*										mPortCtx;
	VsisEventMultiReceiver							mMultiReceiver;
	ULONG											mClusterId;
};

#endif//SUPPORT_VSTRM_VSIS_EVENT

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
