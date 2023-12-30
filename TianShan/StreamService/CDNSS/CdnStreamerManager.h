
#ifndef _cdn_streamer_manager_header_file_h__
#define _cdn_streamer_manager_header_file_h__


#include <Locks.h>
#include <NativeThread.h>
#include <SsServiceImpl.h>
#include <C2StreamerLib.h>
#ifndef ZQ_CDN_UMG
#	include <C2StreamerEnv.h>
#endif
#include "C2HttpLibAsyncBridge.h"
#include "C2StreamerService.h"

namespace ZQ
{
namespace StreamService
{

class CdnSsEnvironment;
class DelayDeleteSessionQueue;

class DeleyDeleteRequest : public ZQ::common::ThreadRequest
{
public:
	DeleyDeleteRequest( SsEnvironment * environment , const std::string& sessionId ,const std::string& contextKey , DelayDeleteSessionQueue& q);
	virtual ~DeleyDeleteRequest( );

protected:

	int				run(void);

private:
	std::string		sessId;
	std::string		ctxKey;
	DelayDeleteSessionQueue&	delayQueue;
	SsEnvironment * env;

};

#ifndef ZQ_CDN_UMG
class ContentFileAttributeBridge : public C2Streamer::IAttrBridge
{
public:
	ContentFileAttributeBridge( CdnSsEnvironment* environment);	
	
	virtual	bool	isFileBeingWritten( const std::string& filename ,const std::string& sessionId );
	virtual bool	getFileDataRange( const std::string& filename , const std::string& sessionId , int64& startByte , int64& endByte ) ;
	
private:
	std::string filenameToContentName( const std::string& filename ) const;	
	std::string filenameToExtname( const std::string& filename ) const;
private:
	TianShanIce::Storage::ContentStorePrx mStorePrx;
	CdnSsEnvironment* env;
};
#endif

class CdnStreamerManager;
class C2EventSinkerI : public C2Streamer::C2EventSinker
{
public:
	C2EventSinkerI( CdnSsEnvironment* environment ,SsServiceImpl& serviceImpl ,CdnStreamerManager& streamerManager );
	virtual ~C2EventSinkerI();
	
public:
	
	virtual int32	publish(const C2Streamer::C2EventPtr request);

protected:

	int32		onSessEvent( const C2Streamer::TransferStateUpdateEventPtr event );
	int32		onStreamerEvent( const C2Streamer::PortStateUpdateEventPtr event);

private:
	CdnSsEnvironment*	env;
	SsServiceImpl&		mSvc;
	CdnStreamerManager&	mStreamerManager;
};

class CdnStreamerManager;

class C2ShadowIndexGetter : public ZQ::common::ThreadRequest
{
public:
	C2ShadowIndexGetter( CdnSsEnvironment* environment, 
		CdnStreamerManager& manager, 
		ZQ::common::NativeThreadPool& pool,
		const std::string& upstreamUrl,
		const std::string& contentName,
		const std::string& pid, const std::string& paid, const std::string& subtype );

	virtual ~C2ShadowIndexGetter( );
private:
	int		run( );

	void 	final(int,bool);

private:
	CdnSsEnvironment*		env;
	CdnStreamerManager&		mStreamerManager;
	std::string				mUpstreamUrl;
	std::string				mContentName;
	std::string				mPid;
	std::string				mPaid;
	std::string				mSubtype;
	
};

class CdnStreamerManager : public ZQ::common::NativeThread
{
public:
	CdnStreamerManager( CdnSsEnvironment* environment ,SsServiceImpl& serviceImpl );
	virtual ~CdnStreamerManager( );

public:

	typedef struct _StreamerAttr 
	{
		std::string				portName; //actually it should be streamer name
		std::vector<std::string> transferAddressIpv4;
		std::vector<std::string> transferAddressIpv6;
		std::string				transferTcpPort;
		int64					capacity;
		bool					bUp;
		int32					activeTransferCount;
		int64					activeBandwidth;
	}StreamerAttr;

	bool			startup( );

	void			shutdown( );

	void			reportStreamerState( const StreamerAttr& attr  );

	void			reportStreamState( const std::string& sessId , SsServiceImpl::StreamEvent event );

	bool			listStreamer( SsReplicaInfoS& infos ) const;

	void			getShadowIndex( const std::string& upstreamUrl, const std::string& name, const std::string& pid, const std::string& paid, const std::string& subtype  );

	void			c2shadowIndexComplete( const std::string& name , const std::string& pid, const std::string& paid, const std::string& subtype );

	C2Streamer::C2Service*	getC2Service() { return  mC2Service; }
	C2Streamer::C2StreamerEnv* getC2Env() { return &mC2Env; }
protected:

	int				run( );

	void 			queryStreamersInfo();
#ifndef ZQ_CDN_UMG
	bool			startupC2Streamer();
	bool 			setC2Conf(C2Streamer::C2StreamerEnv& env );
#else
public:
    CdnSsEnvironment* getCdnSsEnv() {
        return env;
    }
    SsServiceImpl* getSsImpl() {
        return &ss;
    }
#endif
private:	
	typedef std::map<std::string ,  StreamerAttr > StreamerAttrs;
	
	StreamerAttrs						mStreamers;
	ZQ::common::Mutex					mStreamerLocker;

	CdnSsEnvironment*					env;
	SsServiceImpl&						ss;
	bool								mbQuit;
	ZQ::common::Mutex					mMutex;
	ZQ::common::Cond					mCond;
	bool								mbStarted;
#ifndef ZQ_CDN_UMG
	C2Streamer::C2StreamerEnv			mC2Env;
	C2Streamer::C2Service*				mC2Service;
	ZQHttp::Engine*						mC2HttpEngine;
	LibAsync::HttpServer::Ptr 			mLibAsyncHttpServer;
#endif

	ZQ::common::Mutex					mC2ShadowIndexMutex;
	ZQ::common::NativeThreadPool		mC2ShadowIndexGetterPool;
	struct C2ShadowIndexGetterInfo 
	{
		std::string				name;
		std::string				pid;
		std::string				paid;
		std::string				subtype;
	};
	typedef std::map<std::string,C2ShadowIndexGetterInfo> C2SHADOWINDEXMAP;
	C2SHADOWINDEXMAP					mC2ShadowIndexMap;
};


class IngressCapacityUpdateEvent
{
public:
	IngressCapacityUpdateEvent( CdnSsEnvironment* environment );
	virtual ~IngressCapacityUpdateEvent( );
public:
	void					post( const std::string& clientAddress , int64 ingressCapacity  );

private:
	CdnSsEnvironment*		env;

};

class TransferSessionStateUpdateEvent
{
public:
	TransferSessionStateUpdateEvent(  CdnSsEnvironment* environment );
	virtual ~TransferSessionStateUpdateEvent( );

public:
	
	void				post( const std::string& client , const std::string& transferId , const std::string& state );

private:
	 CdnSsEnvironment*			env; 
};


}}

#endif//_cdn_streamer_manager_header_file_h__
