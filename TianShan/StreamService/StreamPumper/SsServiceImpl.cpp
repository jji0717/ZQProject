
#include <ZQ_common_conf.h>
#include <algorithm>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <TianShanDefines.h>
#include <TianShanIceHelper.h>
#include "SsServiceImpl.h"
#include "SsStreamImpl.h"
#include "StreamFactory.h"
#include <FileSystemOp.h>
#include <math.h>
#include <Guid.h>

#include <assert.h>

#ifdef ZQ_OS_MSWIN
#	include <io.h>
#else
#	include <sys/stat.h>
#endif

#ifdef ZQ_OS_MSWIN
#include "memoryDebug.h"
#endif

#if defined(ZQ_OS_MSWIN)
	#define	SSFMT(x, y) 	CLOGFMT(SsServiceImpl, "strm[%s] %s "##y), streamId.c_str(), #x
#else 
	#define	SSFMT(x, y) 	CLOGFMT(SsServiceImpl, "strm[%s] %s " y), streamId.c_str(), #x
#endif

namespace ZQ
{
namespace StreamService
{


SsServiceImpl::SsServiceImpl( SsEnvironment* environment, const std::string& serviceId )
:env(environment),
mEventSender(environment),
mEventSenderManager(mEventSender, environment),
mTicketRenewCenter(environment, *this)
{
	iReplicaReportInterval	=			60 * 1000;
	mbQuit					=			false;	
	strCheckpointPeriod		=			"600";
	strDbRecoverFatal		=			"1";
	strSavePeriod			=			"60000";
	strSaveSizeTrigger		=			"100";
	iEvictorStreamSize		=			1000;
	iReplicaReportInterval	=			60*1000;
	bShowDBOperationTimeCost=			true;
	localId.category		=			"";
	localId.name			=			serviceId;
}

SsServiceImpl::~SsServiceImpl(void)
{
	closeDatabase();
}

void SsServiceImpl::stop()
{
	mDispatchEventThdPool.stop();
	env->getMainThreadPool().stop();
	mTicketRenewCenter.stop();
	mEventSender.stopSender();
	mEventSenderManager.stopSender();
	//stop replica report thread
	mbQuit = true;
	mReplicaReportCond.signal();
	waitHandle(5000);
	closeDatabase();
	env->getSceduler().stop();
}

bool SsServiceImpl::reuseStreamPort( const std::string& contextKey )
{
	Ice::Identity tmpId; tmpId.name = contextKey; tmpId.category = EVICTOR_NAME_STREAM;

	std::string strStreamPort = "";
	std::string strStreamerId = "";
	//get the stream port here			
	TianShanIce::Streamer::SsPlaylistPrx pl = GETOBJECT(TianShanIce::Streamer::SsPlaylistPrx, tmpId);
	if( pl )
	{
		try
		{
			strStreamPort = pl->getAttribute(STREAMINGRESOURCE_STREAMPORT_KEY);
			strStreamerId = pl->getAttribute(STREAMINGRESOURCE_STREAMERID_KEY);
		}
		catch(...) {}
	}

	if( /*!strStreamPort.empty() &&*/ !strStreamerId.empty()  )
	{
		SsReplicaInfoS::iterator itStreamer = mReplicaInfos.find(strStreamerId);
		if( itStreamer != mReplicaInfos.end() )
		{
			if(!itStreamer->second.bHasPorts )
				return true;//If no streamer port in this streamer, just ignore this step

			std::list<std::string>& ports = itStreamer->second.ports;
			std::list<std::string>::iterator itPort = ports.begin();

			bool bFindPort = false;
			for ( ; itPort != ports.end() ; itPort ++ )
			{
				if( *itPort == strStreamPort )
				{					
					bFindPort = true;
					TianShanIce::SRM::ResourceMap dummyResource;
					if(!allocateStreamResource( *this, itStreamer->first, *itPort, dummyResource ))
					{
						ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "can't reuse stream port, destroy the playlist[%s]"), contextKey.c_str());
						try
						{
							Ice::Context ctx;
							ctx["caller"] = "reuseStreamPort";
							pl->destroy(ctx);
						}
						catch(...) {}

						return false;
					}

					ports.erase(itPort);
					return true;
				}
			}

			if( !bFindPort )
			{
				ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "can't find the reused stream port, destroy the playlist[%s]"), contextKey.c_str());
				try
				{
					Ice::Context ctx;
					ctx["caller"] = "can't find the reused stream por";
					pl->destroy(ctx);
				}
				catch(...) {}

				return false;
			}
		}
		else
		{

		}
		//
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "Invaid streamer id, destroy the playlist[%s]"), contextKey.c_str());
		try
		{
			Ice::Context ctx;
			ctx["caller"] = "Invaid streamer id";
			pl->destroy(ctx);
		}
		catch(...) {}

		return false;
	}

	return false;
}

void	SsServiceImpl::restoreSessions( )
{
#pragma message(__MSGLOC__"TODO: must rewrite this function")
	//I must use another way to restore a session
	
	//enum all session's from DB
	std::vector<Ice::Identity> ids;
	ids.reserve( 8 * 1024 );
	try
	{
		Freeze::EvictorIteratorPtr iter = mStreamEvictor->getIterator("", 2048);
		while (iter->hasNext())
			ids.push_back( iter->next() );		
	}
	catch( const Ice::Exception& ex)
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "restoreSessions() failed to enum sessions from DB due to [%s]"), ex.ice_name().c_str() );
		return ;
	}
	TianShanIce::Streamer::PlaylistItemSetupInfo dummyInfo;
	std::vector<Ice::Identity>::const_iterator itId = ids.begin();
	for( ; itId != ids.end() ; itId ++ )
	{
		TianShanIce::Streamer::SsPlaylistPrx pl = NULL;
		try
		{
			pl = GETOBJECT( TianShanIce::Streamer::SsPlaylistPrx, *itId );
			pl->onRestore();
		}
		catch( const Ice::Exception& ex)
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "restoreSessions() failed to get instance for session[%s] due to [%s]"),
				itId->name.c_str(), ex.ice_name().c_str() );
			continue;
		}
	}

	/*
	ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "restoreSessions() enter restoreSessions"));

	std::vector<Ice::Identity> identities;
	identities.reserve( 2 * 1024 );
	try
	{
		Freeze::EvictorIteratorPtr iter = mStreamEvictor->getIterator("", 1024);
		while (iter->hasNext())
			identities.push_back( iter->next() );		

		SessionRestoreInfos restoreInfos;

		std::vector<Ice::Identity>::iterator itId = identities.begin();
		for( ; itId != identities.end() ; itId ++ )
		{
			try
			{
				//collect all running stream sessions and its associated context key
				TianShanIce::Streamer::SsPlaylistPrx pl = GETOBJECT( TianShanIce::Streamer::SsPlaylistPrx, *itId );
				if(!pl)
				{
					ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "can't create proxy from session [%s]"),
						itId->name.c_str());
				}
				else
				{
					//get SessionRestoreInfo
					SessionRestoreInfo info;
					if( pl->getRunningSession( info.currentStreamId, info.nextStreamId, info.speed, info.state ) )
					{
						info.contextKey = itId->name;
						restoreInfos.push_back(info);
						if( !info.currentStreamId.empty() )
						{
							registerStreamId( info.currentStreamId, itId->name);
						}
						if( !info.nextStreamId.empty() )
						{
							registerStreamId( info.nextStreamId, itId->name );

						}
					}
				}
			}
			catch( const TianShanIce::BaseException& ex)
			{
				ENVLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(SsServiceImpl, "restoreSessions() caught Tianshan exception [%s] when restore session[%s]"),
					ex.message.c_str(),
					itId->name.c_str() );
			}
			catch( const Ice::Exception& ex)
			{
				ENVLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(SsServiceImpl, "restoreSessions() caught ice exception [%s] when restore session[%s]"),
					ex.ice_name().c_str(),
					itId->name.c_str() );
			}
		}
		
		SessionRestoreInfos retSessions = checkSessions( *this, restoreInfos );
		std::sort( restoreInfos.begin(), restoreInfos.end(), SortSessionRestoreInfo() );
		std::sort( retSessions.begin(), retSessions.end(), SortSessionRestoreInfo()  );
		
		SessionRestoreInfos::const_iterator itRestore = restoreInfos.begin();
		SessionRestoreInfos::const_iterator itRet = retSessions.begin();


		while ( itRestore != restoreInfos.end() && itRet != retSessions.end() )
		{
			if( !reuseStreamPort( itRestore->contextKey) )
			{//if we can reuse the port, just destroy the playlist
				if( itRestore->contextKey == itRet->contextKey)
				{
					itRet++;//skip to next ret session because current session can't be re-built
				}
				itRestore++;//skip to next restore session because current session can't be re-built
				continue;
			}
			//ok, the playlist can be re-built, just check its running stream session
			if( itRet->contextKey > itRestore->contextKey )
			{//both stream session of restore one are missing
				Ice::Identity tmpId; tmpId.name = itRestore->contextKey; tmpId.category = EVICTOR_NAME_STREAM;				
				try
				{//destroy the playlist because restore context is gone
					TianShanIce::Streamer::SsPlaylistPrx pl = GETOBJECT(TianShanIce::Streamer::SsPlaylistPrx, tmpId);
					ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "playlist expired, destroy the playlist[%s]"), tmpId.name.c_str());
					pl->destroy();
				}
				catch( ... ){}
				itRestore++;
			}
			else if( itRet->contextKey == itRestore->contextKey )
			{
				if( itRet->currentStreamId.empty() )
				{
					StreamParams				paras;
					TianShanIce::Properties		props;
					OnStreamEvent( seGone, itRestore->currentStreamId, paras, props );
					if( fabs( itRestore->speed - itRet->speed ) > 0.01 )
					{//speed changed
						OnStreamEvent( seScaleChanged, itRet->currentStreamId, paras, props );
					}
					if ( itRestore->state != itRestore->state )
					{
						OnStreamEvent( seStateChanged, itRet->currentStreamId, paras, props );
					}
				}
				else if( itRet->nextStreamId.empty() )
				{
					StreamParams				paras;
					TianShanIce::Properties		props;
					OnStreamEvent( seGone, itRestore->nextStreamId, paras, props );
					if( fabs( itRestore->speed - itRet->speed ) > 0.01 )
					{//speed changed
						OnStreamEvent( seScaleChanged, itRet->nextStreamId, paras, props  );
					}
					if ( itRestore->state != itRestore->state )
					{
						OnStreamEvent( seStateChanged, itRestore->nextStreamId, paras, props );
					}
				}				
				itRet ++ ;
				itRestore ++ ;					
			}
			else
			{
				itRestore++;
				assert(false);
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "restoreSessions() bad implementation"));
			}
		}
		while ( itRestore != restoreInfos.end() )
		{
			reuseStreamPort( itRestore->contextKey) ;//do check the return value, if failed, the playlist will be destroyed			
			Ice::Identity tmpId; tmpId.name = itRestore->contextKey; tmpId.category = EVICTOR_NAME_STREAM;
			
			try
			{
				TianShanIce::Streamer::SsPlaylistPrx pl = GETOBJECT(TianShanIce::Streamer::SsPlaylistPrx, tmpId);
				ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "playlist expired, destroy the playlist[%s]"), tmpId.name.c_str());
				//destroy the playlist because restore context is gone
				pl->destroy();
			}
			catch( ... ){}
			itRestore++;
		}
		if( itRet != retSessions.end() )
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "restoreSessions() bad implementation, retSession still have some members"));
		}
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(SsServiceImpl, "restoreSessions() ice exception [%s] caught when restoring sessions"),
			ex.ice_name().c_str( ) );
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "restoreSessions() unknown exception caught when restoring sessions"));
	}
	ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "restoreSessions() leave restoreSessions"));
	*/
}

bool SsServiceImpl::refreshStreamer(  )
{
	ZQ::common::MutexGuard gd(mStreamerInfoLocker);

	SsReplicaInfoS backInfos =   mReplicaInfos;
	mReplicaInfos.clear();
	//get all streamers from porting layer
	SsReplicaInfoS streamers;
	if( !listAllReplicas( *this, streamers ) )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "can't list all streamers, just quit starting"));
		return false;
	}
	//setup streamers information
	{		
		ZQ::common::MutexGuard gd(mStreamerInfoLocker);
		SsReplicaInfoS::const_iterator it = streamers.begin();
		for( ; it != streamers.end() ; it ++ )
		{
			SsReplicaInfo	 infoEx;

			infoEx.category			=	it->second.category;
			infoEx.bStreamReplica	=	it->second.bStreamReplica;
			infoEx.streamerType		=	it->second.streamerType;			
			infoEx.maxPrioritySeenInGroup = it->second.maxPrioritySeenInGroup;
			infoEx.obj				=	UCKGETOBJECT( Ice::ObjectPrx, localId );
			infoEx.priority			=	it->second.priority;
			infoEx.replicaId		=	it->first;
			infoEx.replicaState		=	it->second.replicaState;
			infoEx.stampBorn		=	(it->second.replicaState == TianShanIce::stInService) ? ZQTianShan::now() : 0;
			infoEx.stampChanged		=	ZQTianShan::now();			
			infoEx.ports			=	it->second.ports;			
			infoEx.bHasPorts		=	it->second.bHasPorts;

			if( !it->second.groupId.empty() )
				infoEx.groupId		=	it->second.groupId;
			else
				infoEx.groupId			=	strNetId;

			ZQTianShan::Util::mergeProperty( infoEx.props, it->second.props );

			if( infoEx.replicaState == TianShanIce::stInService )
			{
				if( backInfos.find( it->first ) == backInfos.end())
				{
					ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "repliace[%s] first up"), it->first.c_str());
				}
			}

			mReplicaInfos[ it->first ] =	infoEx;

			if( backInfos.find( it->first ) == backInfos.end())
			{
				ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "get a replica [%s] status[%s] port count[%u] "),
												it->first.c_str(), 	infoEx.replicaState == TianShanIce::stInService ? "UP" : "DOWN", infoEx.ports.size());
			}
		}
	}
	return ( mReplicaInfos.size() > 0 );
}

bool SsServiceImpl::start(	const std::string& dbPath, const std::string& eventChannelEndpoint,
							const std::string& subscriberEP, const std::string& serviceNetId,
							const std::string& serviceName )
{
	ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "start streaming service with dbpath[%s] serviceNetId[%s] serviceName[%s]"),
		dbPath.c_str(), serviceNetId.c_str(), serviceName.c_str() );

	env->getCommunicator()->addObjectFactory( new ZQ::StreamService::StreamFactory( env, *this ), TianShanIce::Streamer::SsPlaylist::ice_staticId() );
	env->getMainAdapter()->ZQADAPTER_ADD( env->getCommunicator(), this, serviceName);

	strNetId = serviceNetId;
	localId.name = serviceName;

	if( strNetId.empty() )
	{
		char szLocalName[1025];
        memset(szLocalName, '\0', 1025);
		gethostname(szLocalName, 1024);
		strNetId = szLocalName;
        ENVLOG(ZQ::common::Log::L_DEBUG, "no netid provided, use the host name:%s", szLocalName);
	}

	if(!refreshStreamer())
    {
        ENVLOG(ZQ::common::Log::L_ERROR, "failed to refresh the streamer, abort the start.");
		return false;
    }

	if(! openDatabase( dbPath ) )
	{
        ENVLOG(ZQ::common::Log::L_ERROR, "failed to open db at %s, abort the start.", dbPath.c_str());
		return false;
	}

	mEventSender.startSender( eventChannelEndpoint );
	mEventSenderManager.startSender( eventChannelEndpoint );
	mReplicaSubscriberEP = subscriberEP;
	mTicketRenewCenter.start();
	if(!ZQ::common::NativeThread::start())
	{
        ENVLOG(ZQ::common::Log::L_ERROR, "failed to start the replica report thread. abort the start." );
		return false;
	}

	env->getSceduler().start();
	restoreSessions( );

	ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "--->streaming service started<---"));
	return true;
}

std::string svcCreateSessionId()
{
	ZQ::common::Guid uid;
	uid.create();
	char szUidBuffer[128];
	szUidBuffer[ sizeof(szUidBuffer) - 1 ] = 0;
	uid.toString( szUidBuffer, sizeof(szUidBuffer) - 2 );	
	return std::string(szUidBuffer);
}

TianShanIce::Streamer::StreamPrx SsServiceImpl::createStream(const ::TianShanIce::Transport::PathTicketPrx& ticket, 
															 const ::Ice::Current& /*= ::Ice::Current()*/) 
{	
	ZQTianShan::Util::TimeSpan sw;sw.start();
	std::string			streamId = svcCreateSessionId();	
	ENVLOG(ZQ::common::Log::L_DEBUG, SSFMT(createStream, "enter createStream"));
	if(!ticket)
	{
		TianShanIce::SRM::ResourceMap dummyResource;
		dummyResource.clear();
		return doCreateStream(streamId, dummyResource, NULL, "", NULL, TianShanIce::Properties()  );
	}	

	std::string ticketId = ticket->getIdent( ).name;
	ENVLOG(ZQ::common::Log::L_INFO, SSFMT( createStream, "create stream with ticket[%s]" ), ticketId.c_str() );
	TianShanIce::SRM::ResourceMap ticketResource		= ticket->getResources( );

	std::string netId;
	TianShanIce::Transport::StreamLinkPrx streamLink	= ticket->getStreamLink( );
	if( streamLink)
	{
		TianShanIce::Transport::Streamer streamInfo			= streamLink->getStreamerInfo();
		netId = streamInfo.netId;
		std::string::size_type bsPos =netId.find("/");
		if(  bsPos != std::string::npos )
		{
			netId = netId.substr(  bsPos +1);
		}
		ENVLOG(ZQ::common::Log::L_INFO, SSFMT(createStream, "create stream with streamer[%s] coming from stream link information"),
			netId.c_str() );
	}
	else
	{
		//get netId from ticket resource
		try
		{
			ZQTianShan::Util::getResourceData( ticketResource, TianShanIce::SRM::rtStreamer, "NetworkId", netId );
		}
		catch( const TianShanIce::InvalidParameter& ex )
		{
			ENVLOG(ZQ::common::Log::L_ERROR, SSFMT(createStream, "Can't find Streamer NetworkId from ticket resource"));
			ex.ice_throw();
		}

		std::string::size_type bsPos =netId.find("/");
		if(  bsPos != std::string::npos )
		{
			netId = netId.substr(  bsPos +1);
		}
		ENVLOG(ZQ::common::Log::L_INFO, SSFMT(createStream, "create stream with streamer[%s] coming from ticket resource"),
			netId.c_str() );
		//ENVLOG(ZQ::common::Log::L_INFO, SSFMT(createStream, "get ticket but no streamLink is found, so no streamer info can be found"));
	}

	TianShanIce::Streamer::StreamPrx  newstream = doCreateStream(streamId, ticketResource, streamLink, netId, ticket, TianShanIce::Properties() );
	ENVLOG(ZQ::common::Log::L_INFO, SSFMT(createStream, "createstream, cost[%lld]"), sw.stop());
	return newstream;
}

TianShanIce::Streamer::StreamPrx 
SsServiceImpl::createStreamByResource( const ::TianShanIce::SRM::ResourceMap& resource, 
											const TianShanIce::Properties& props,
											const ::Ice::Current& ) 
{
	std::string streamId = svcCreateSessionId();
	ENVLOG(ZQ::common::Log::L_DEBUG, SSFMT( createStream, "enter createStreamByResource"));	
	
	bool bHasStreamer = false;

	std::string strStreamerNetId = "";

	ZQTianShan::Util::getResourceDataWithDefault( resource, TianShanIce::SRM::rtStreamer, "NetworkId", "", strStreamerNetId );
	bHasStreamer = !strStreamerNetId.empty();
	
	if( !bHasStreamer )
		return doCreateStream( streamId, resource, NULL, "", NULL, props);

	std::string::size_type bsPos =strStreamerNetId.find("/");
	if(  bsPos != std::string::npos )
	{
		strStreamerNetId = strStreamerNetId.substr( bsPos + 1);
	}

	return doCreateStream( streamId, resource, NULL, strStreamerNetId, NULL, props );		
}

bool SsServiceImpl::isObjectExist( const Ice::Identity& id )
{
	return mStreamEvictor->hasObject(id);
}

TianShanIce::Transport::PathTicketPrx SsServiceImpl::getPathTicket( const std::string& contextKey )
{
	TianShanIce::Transport::PathTicketPrx ticket = NULL;
	try
	{
		Ice::Identity id;
		id.name			=	contextKey;
		id.category		=	EVICTOR_NAME_STREAM;
		TianShanIce::Streamer::SsPlaylistPrx playlistPrx =UCKGETOBJECT(TianShanIce::Streamer::SsPlaylistPrx, id); 
		ticket = playlistPrx->getPathTicket();			
		return ticket;
	}
	catch( const Ice::Exception& )
	{
	}

	return NULL;
}

bool SsServiceImpl::allocateStreamingResource( const std::string& streamerId,
											  const TianShanIce::SRM::ResourceMap& srmResources,
											  TianShanIce::Properties& streamingRes )
{
	std::string streamerNetId = streamerId;
	std::string	streamPort = "";

	bool		bGetStreamPort = false;		
	
	{
		//find streamer and allocate streaming resource
		ZQ::common::MutexGuard gd(mStreamerInfoLocker);

		if( streamerNetId.empty() )
		{//no streamer is specified, take all streamers as candidate
			return true;//do nothing if streamerNetId is empty

			SsReplicaInfoS::iterator it = mReplicaInfos.begin();
			for( ; it != mReplicaInfos.end() && !bGetStreamPort ; it ++ )
			{
				if ( it->second.replicaState != TianShanIce::stInService)
				{
					continue;
				}
								
				if( it->second.bHasPorts )
				{
					std::list<std::string>& ports = it->second.ports;
					std::list<std::string>::iterator itPort = ports.begin();
					for( ; itPort != ports.end() && !bGetStreamPort ; itPort ++ )
					{
						if( allocateStreamResource( *this, it->first, *itPort, srmResources ) )
						{
							streamerNetId = it->first;
							bGetStreamPort = true;
							streamPort = *itPort;
							ports.erase(itPort);
							break;
						}
					}
#if defined _DEBUG || defined DEBUG
					ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "current port count[%u]"),
						ports.size() );
#endif

				}
				else
				{
					//if no ports in this streamer, just ignore it
					if( allocateStreamResource(*this, it->first, "", srmResources ) )
					{
						streamerNetId = it->first;
						bGetStreamPort = true;
						streamPort ="";
						break;
					}
				}
			}
		}
		else
		{
			SsReplicaInfoS::iterator it = mReplicaInfos.find( streamerNetId );
			if( it != mReplicaInfos.end() )
			{
				if( it->second.replicaState != TianShanIce::stInService )
				{
					ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG, LOGCATAGORY, 0, CLOGFMT(SsServiceImpl, "allocateStreamingResource() specified streamer[%s] is unavailable"),
						streamerNetId.c_str());
				}
				if( it->second.bHasPorts )
				{
					std::list<std::string>& ports = it->second.ports;

					std::list<std::string>::iterator itPort = ports.begin();
					for( ; itPort != ports.end() && !bGetStreamPort ; itPort ++ )
					{
						if( allocateStreamResource( *this, it->first, *itPort, srmResources ) )
						{
							bGetStreamPort = true;
							streamPort = *itPort;
							ports.erase(itPort);
							break;
						}
					}
				}
				else
				{
					if( allocateStreamResource( *this, it->first, "", srmResources ) )
					{
						bGetStreamPort = true;
						streamPort = "";						
					}
				}
			}
		}
	}

	if( !bGetStreamPort)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG, LOGCATAGORY, 0,
			CLOGFMT(SsServiceImpl, "can't find a suitable stream port for streamer[%s]"),
			streamerNetId.c_str() );
	}

	ZQTianShan::Util::updatePropertyData( streamingRes, STREAMINGRESOURCE_STREAMERID_KEY, streamerNetId);
	ZQTianShan::Util::updatePropertyData( streamingRes, STREAMINGRESOURCE_STREAMPORT_KEY, streamPort );

	ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "allocate a suitable stream port [%s] for streamer[%s]"),
		streamPort.c_str(), streamerNetId.c_str() );


	return true;
}

void SsServiceImpl::releaseStreamingResource( const TianShanIce::Properties& streamingRes )
{
	std::string		streamerId;
	std::string		streamPort;
	ZQTianShan::Util::getPropertyDataWithDefault(streamingRes, STREAMINGRESOURCE_STREAMERID_KEY, "", streamerId );
	ZQTianShan::Util::getPropertyDataWithDefault(streamingRes, STREAMINGRESOURCE_STREAMPORT_KEY, "", streamPort );
	ENVLOG(ZQ::common::Log::L_INFO, 	CLOGFMT(SsServiceImpl, "releaseStreamingResource() release stream port[%s] for streamer[%s]"),
		streamPort.c_str(), streamerId.c_str() );
	if( streamerId.empty() || streamPort.empty() )
	{
		ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SsServiceImpl, "releaseStreamingResource() empty streamer id [%s] or stream port[%s]"),
			streamerId.c_str(),
			streamPort.c_str() );
		return;
	}

	{
		ZQ::common::MutexGuard gd(mStreamerInfoLocker);
		SsReplicaInfoS::iterator it = mReplicaInfos.find( streamerId );
		if( it == mReplicaInfos.end() )
		{
			ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SsServiceImpl, "releaseStreamingResource() can't find streamer[%s]"),
				streamerId.c_str() );
			return ;
		}
		std::list<std::string>& ports = it->second.ports;
		ports.push_front( streamPort );
#if defined _DEBUG || defined DEBUG
		ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "current port count[%u]"),
			ports.size() );
#endif
	}
}


TianShanIce::Streamer::StreamPrx
SsServiceImpl::doCreateStream(	const std::string& streamId, 
								const TianShanIce::SRM::ResourceMap& srmResources,
								TianShanIce::Transport::StreamLinkPrx streamLink,
								const std::string&		strmerNetId,
								TianShanIce::Transport::PathTicketPrx ticket,
								const TianShanIce::Properties& props )
{
	std::string			destIp;
	Ice::Int			destPort;
	std::string			destMac;
	std::string			serviceIP;
	Ice::Int			servicePort;
	Ice::Long			bandwidth;
	Ice::Int			enableNAT;
	std::string			natSessionId;

	std::string			ticketId = "";	
	bool				bHasAttribute = false;

	std::string			streamerNetId = strmerNetId;

	try
	{
		std::string logHint = streamId + " <Resource>";
		if(ticket)
		{
			ticketId = ticket->getIdent().name;

			logHint = "<";
			logHint = logHint + streamId;
			logHint = logHint + ">";			
		}

		ZQTianShan::Util::dumpTianShanResourceMap( srmResources, ENVLOG, logHint );

		ZQTianShan::Util::getResourceDataWithDefault( srmResources, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", 0, bandwidth);
		ZQTianShan::Util::getResourceDataWithDefault( srmResources, TianShanIce::SRM::rtEthernetInterface, "destIP", "", destIp );
		ZQTianShan::Util::getResourceDataWithDefault( srmResources, TianShanIce::SRM::rtEthernetInterface, "destPort", 0, destPort );
		ZQTianShan::Util::getResourceDataWithDefault( srmResources, TianShanIce::SRM::rtEthernetInterface, "destMac", "", destMac );
		ZQTianShan::Util::getResourceDataWithDefault( srmResources, TianShanIce::SRM::rtEthernetInterface, "srcPort", 0, servicePort);
		ZQTianShan::Util::getResourceDataWithDefault( srmResources, TianShanIce::SRM::rtEthernetInterface, "natPenetrating", 0, enableNAT );
		if( enableNAT ) 
		{ 
			ZQTianShan::Util::getResourceDataWithDefault(srmResources, TianShanIce::SRM::rtEthernetInterface, "pokeholeSession", "", natSessionId); 
		}

		bHasAttribute = true; 
	}
	catch( const TianShanIce::BaseException& ex)
	{		
		ENVLOG(ZQ::common::Log::L_ERROR, SSFMT( doCreateStream, "create stream failed:%s" ), ex.message.c_str());
		ex.ice_throw();
	}
	catch( const Ice::Exception& ex)
	{		
		ENVLOG(ZQ::common::Log::L_ERROR, SSFMT( doCreateStream, "create stream failed:%s" ), ex.ice_name().c_str());
		ex.ice_throw();
	}	

	TianShanIce::Properties streamingRes;
	allocateStreamingResource( streamerNetId, srmResources, streamingRes );
	ZQTianShan::Util::mergeProperty(streamingRes, props);

	Ice::Identity id;
	id.name			= streamId;
	id.category		= EVICTOR_NAME_STREAM;

	ZQ::StreamService::SsStreamImpl* strmImpl = new ZQ::StreamService::SsStreamImpl( *this, env, streamingRes, id );
	assert( strmImpl != NULL );

#pragma message(__MSGLOC__"TODO: initialize stream implementation if has attribute")	

	strmImpl->ident			=	id;
	strmImpl->crResource	=	srmResources;
	
	if( bHasAttribute )
	{//set destination ip / port what ever it exist or not
		strmImpl->setDestination(destIp, destPort);
		if (!destMac.empty() )	strmImpl->setDestMac(destMac);
		strmImpl->setMuxRate( 0, (Ice::Int)bandwidth, 0 );		
	}

	//register path ticket to ticket renew center
	strmImpl->pathTicket = ticket;
	
	addServantToDB(strmImpl, id);
	
	if( ticket )
	{
		mTicketRenewCenter.doRegister(id, ticket);
	}

	if( env->getConfig().iPlaylistTimeout > 0 )
	{
		strmImpl->updateTimer();
	}

	return UCKGETOBJECT( TianShanIce::Streamer::StreamPrx, id);
}

TianShanIce::Streamer::PlaylistIDs SsServiceImpl::listPlaylists(const ::Ice::Current& ) 
{
	TianShanIce::Streamer::PlaylistIDs ids;
	
	Freeze::EvictorIteratorPtr iter = mStreamEvictor->getIterator("", 4096);
	while (iter->hasNext())
		ids.push_back( iter->next().name );		
	return ids;
}

TianShanIce::Streamer::StreamPrx SsServiceImpl::openStream( const std::string& id, const Ice::Current& )
{	
	Ice::Identity ident;
	
	ident.name		= id;
	ident.category	= EVICTOR_NAME_STREAM;
	ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl, "open stream by id[%s]"), id.c_str() );
	try
	{
		TianShanIce::Streamer::StreamPrx prx = GETOBJECT( TianShanIce::Streamer::StreamPrx, ident ) ; 
		ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl, "stream[%s] opened"), id.c_str() );
		return prx;
	}
	catch( const Ice::ObjectNotExistException&)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG, LOGCATAGORY, 0, "[SsServiceImpl] can't find stream[%s]", id.c_str() );
	}
	catch( const Ice::Exception& ex )
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG, LOGCATAGORY, 0, "[SsServiceImpl] failed to open stream instance by id[%s] due t [%s]",
			id.c_str(), ex.ice_name().c_str() );
	}

	return NULL;
}

TianShanIce::Streamer::SsPlaylistS SsServiceImpl::listSessions(const ::Ice::Current& /*= ::Ice::Current()*/)
{
	TianShanIce::Streamer::SsPlaylistS pls;
	TianShanIce::Streamer::PlaylistIDs ids = listPlaylists();
	pls.reserve(ids.size());
	TianShanIce::Streamer::PlaylistIDs::const_iterator it = ids.begin();
	for ( ; it != ids.end() ; it ++ )
	{
		Ice::Identity id;
		id.name = *it;
		id.category = EVICTOR_NAME_STREAM;
		pls.push_back(UCKGETOBJECT(TianShanIce::Streamer::SsPlaylistPrx, id));
	}

	return pls;
}

TianShanIce::Streamer::PlaylistPrx SsServiceImpl::openPlaylist(const ::std::string& streamId, 
															   const ::TianShanIce::Streamer::SpigotBoards&,
															   bool, 
															   const ::Ice::Current&)
{
	Ice::Identity id;
	id.name			= streamId;
	id.category		= EVICTOR_NAME_STREAM;
	return UCKGETOBJECT( TianShanIce::Streamer::PlaylistPrx, id);
}


void SsServiceImpl::updateIceProperty(Ice::PropertiesPtr iceProperty, 
									  const std::string& key,
									  const std::string& value )
{
	assert(iceProperty);
	iceProperty->setProperty( key, value );
	ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "updateIceProperty() [%s] = [%s]"), key.c_str(), value.c_str() );	
}


#define FREEZEPROPENV(x, y)	std::string("Freeze.DbEnv.")+x+y
#define FREEZEPROPEVTSTREAM(x, y) std::string("Freeze.Evictor.")+x+"."+EVICTOR_NAME_STREAM+y

bool SsServiceImpl::openDatabase( const std::string &dbPath )
{
	assert( dbPath.size() > 0 );
	
	std::string dbTopPath = ZQTianShan::Util::fsConcatPath( dbPath, STREAMSERVICEDBFOLDER );

	if(! FS::createDirectory(dbTopPath, true))
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "openDatabase() failed to create database top path [%s]"), dbTopPath.c_str() );
		return false;
	}

	try
	{
		{
#define MAX_CONTENTS 100000
#define DEFAULT_CACHE_SIZE (160*1024*1024)
			::std::string dbConfFile = ZQTianShan::Util::fsConcatPath( dbTopPath, "DB_CONFIG" );
			if ( -1 == ::access(dbConfFile.c_str(), 0))
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl, "initializing %s"), dbConfFile.c_str());
				FILE* f = ::fopen(dbConfFile.c_str(), "w+");
				if (NULL != f)
				{
					::fprintf(f, "set_lk_max_locks %d\n",  MAX_CONTENTS);
					::fprintf(f, "set_lk_max_objects %d\n", MAX_CONTENTS);
					::fprintf(f, "set_lk_max_lockers %d\n", MAX_CONTENTS);
					::fprintf(f, "set_cachesize 0 %d 0\n", 	DEFAULT_CACHE_SIZE);
					::fclose(f);
				}
			}

		}
		Ice::PropertiesPtr iceProperty = env->getCommunicator()->getProperties();
		updateIceProperty( iceProperty, FREEZEPROPENV(dbTopPath, ".CheckpointPeriod"), strCheckpointPeriod );
		updateIceProperty( iceProperty, FREEZEPROPENV(dbTopPath, ".DbRecoverFatal" ), strDbRecoverFatal );
		updateIceProperty( iceProperty, FREEZEPROPEVTSTREAM(dbTopPath, ".SavePeriod"), strSavePeriod );
		updateIceProperty( iceProperty, FREEZEPROPENV(dbTopPath, ".DbPrivate"), "0");
		updateIceProperty( iceProperty, FREEZEPROPEVTSTREAM(dbTopPath, ".SaveSizeTrigger"), strSaveSizeTrigger);
		//updateIceProperty( iceProperty, FREEZEPROPENV(dbTopPath, ".OldLogsAutoDelete"), "1");
		if( env->getConfig().iUseMemoryDB )
		{
			mStreamEvictor = new Util::MemoryServantLocator( env->getMainAdapter(), EVICTOR_NAME_STREAM );
			ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "openDatabase() using MemoryDB not BerkeleyDB"));
		}
		else
		{

#if ICE_INT_VERSION / 100 >= 303			
		mStreamEvictor=Freeze::createBackgroundSaveEvictor (env->getMainAdapter(), dbTopPath, EVICTOR_NAME_STREAM );
#else
		mStreamEvictor=Freeze::createEvictor ( env->getMainAdapter(), dbTopPath, EVICTOR_NAME_STREAM );
#endif
		}
		env->getMainAdapter()->addServantLocator(mStreamEvictor, EVICTOR_NAME_STREAM);

		ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "openDatabase() set stream evictor size [%d]"), iEvictorStreamSize);
		mStreamEvictor->setSize(iEvictorStreamSize);

		ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "openDatabase() successfully open data base at [%s]"), dbTopPath.c_str() );
		return true;
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, 
			CLOGFMT(SsServiceImpl, "failed to open db and exception is [%s]"),
			ex.ice_name().c_str() );
		return false;
	}

	return true;
}

void SsServiceImpl::closeDatabase( )
{
	if(mStreamEvictor)
	{
		ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "close data base"));
		mStreamEvictor = NULL;
	}
}

bool SsServiceImpl::addServantToDB(TianShanIce::Streamer::SsPlaylistPtr s, const Ice::Identity& id)
{
	if( mStreamEvictor )
	{
		try
		{
			if( bShowDBOperationTimeCost )
			{
				IceUtil::Time opStart = IceUtil::Time::now();
				mStreamEvictor->add( s, id );
				IceUtil::Time opEnd = IceUtil::Time::now();
				ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl, "addServantToDB() add id[%s/%s] into db with time cost[%llu]"),
					id.category.c_str(), id.name.c_str(), (opEnd-opStart).toMilliSeconds() );
			}
			else
			{
				mStreamEvictor->add(s, id);
			}			
		}		
		catch ( const Ice::Exception& ex )
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "failed to add [%s/%s] to DB because ice exception [%s]"),
				id.category.c_str(), id.name.c_str(), ex.ice_name().c_str() );
			return false;
		}
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "addServantToDB() failed to add servant to DB because evictor is not opened"));
	}

	return true;
}

void SsServiceImpl::removeServantFromDB( const Ice::Identity& id, const TianShanIce::Properties& resource )
{
	unregisterStreamIdByContextKey(id.name);
	mTicketRenewCenter.doUnregister( id );
	if(mStreamEvictor)
	{
		try
		{
			int refCount = 0;
			if( bShowDBOperationTimeCost )
			{
				IceUtil::Time opStart = IceUtil::Time::now();
				Ice::ObjectPtr obj = mStreamEvictor->remove(id);
				refCount = obj->__getRef();
				IceUtil::Time opEnd = IceUtil::Time::now();
				ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl, "removeServantFromDB() remove id[%s/%s] from db with time cost[%llu]"),
					id.category.c_str(), id.name.c_str(), (opEnd-opStart).toMilliSeconds() );
			}
			else
			{
				//mStreamEvictor->remove(id);
				Ice::ObjectPtr obj = mStreamEvictor->remove(id);
				refCount = obj->__getRef();
			}
			
			//release allocated streaming resource
			releaseStreamingResource( resource );			
		}
		catch(  const Ice::Exception& ex )
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "failed to remove [%s/%s] from DB because ice exception [%s]"),
				id.category.c_str(), id.name.c_str(), ex.ice_name().c_str() );
		}
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(SsServiceImpl, "removeServantFromDB() can't remove servant from DB because evictor is not opened"));
	}
}

std::string SsServiceImpl::getNetId(const ::Ice::Current& ) const
{
	return strNetId;
}
std::string SsServiceImpl::getAdminUri(const ::Ice::Current& )
{
	return "";
}

TianShanIce::State SsServiceImpl::getState(const ::Ice::Current&  )
{
	return TianShanIce::stInService;
}

TianShanIce::Streamer::StreamerDescriptors SsServiceImpl::listStreamers(const ::Ice::Current& ) 
{
	TianShanIce::Streamer::StreamerDescriptors streamerIds;
	{
		TianShanIce::Streamer::StreamerDescriptor sd;
		ZQ::common::MutexGuard gd( mStreamerInfoLocker );
		SsReplicaInfoS::const_iterator it = mReplicaInfos.begin();
		for( ; it != mReplicaInfos.end() ; it ++ )
		{
			if( it->second.bStreamReplica )
			{
				sd.deviceId	=	it->second.replicaId;
				sd.type		=	it->second.streamerType;
				streamerIds.push_back(sd);
			}
		}
	}	
	return streamerIds;
}

void SsServiceImpl::collectStreamerReplicas( TianShanIce::Replicas& reps )
{
	reps.clear();
	TianShanIce::Replica rep;
	ZQ::common::MutexGuard gd( mStreamerInfoLocker );
	SsReplicaInfoS::const_iterator it = mReplicaInfos.begin();
	for( ; it != mReplicaInfos.end() ; it ++ )
	{
		if(!it->second.bStreamReplica)
			continue;

		rep.category	=	it->second.category;
		rep.groupId		=	it->second.groupId;
		rep.maxPrioritySeenInGroup	= it->second.maxPrioritySeenInGroup;
		rep.obj			=	it->second.obj;
		rep.priority	=	it->second.priority;
		rep.replicaId	=	it->second.replicaId;
		rep.replicaState=	it->second.replicaState;
		rep.stampBorn	=	it->second.stampBorn;
		rep.stampChanged=	it->second.stampChanged;
		rep.props		=	it->second.props;
		reps.push_back(rep);
	
	}
}

void SsServiceImpl::queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& callback,
										const ::std::string& category, 
										const ::std::string& groupId, 
										bool bLocal, 
										const ::Ice::Current& ) 
{
	try
	{
		{
			TianShanIce::Replicas reps;
			collectStreamerReplicas( reps );
			callback->ice_response(reps);
		}
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(queryReplicas_async, "Ice exception caught [%s]"), ex.ice_name().c_str() );
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(queryReplicas_async, "unknown exception caught"));
	}
}

uint32	SsServiceImpl::reportReplica( )
{
	uint32	interval = iReplicaReportInterval;
	
	if( mReplicaSubscriberEP.empty() )
	{
		return interval;
	}	
	
	std::string subscriberEP;
	if( mReplicaSubscriberEP.find(":") == std::string::npos )
	{
		subscriberEP = ":" + mReplicaSubscriberEP;
	}
	else
	{
		subscriberEP = mReplicaSubscriberEP;
	}

	{
		TianShanIce::Replicas reps;
		collectStreamerReplicas( reps );
		try
		{
			TianShanIce::ReplicaSubscriberPrx subscriber = TianShanIce::ReplicaSubscriberPrx::checkedCast(
				env->getCommunicator()->stringToProxy(subscriberEP) );

			if( !subscriber )
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "failed to connect to replica subscriber[%s]"),
					subscriberEP.c_str() );
				interval = iReplicaReportInterval;
			}
			else
			{
				interval = subscriber->updateReplica( reps ) ;
				ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl, "report replica ok, got interval[%d]msec: %s"),
					interval*1000, subscriberEP.c_str() );
				interval = interval * 700;
			}

			return interval;
		}
		catch( const TianShanIce::BaseException& ex )
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "failed to report replica to subscriber [%s], caught TianShan exception[%s]"),
				subscriberEP.c_str(), ex.message.c_str()	);
		}
		catch( const Ice::Exception& ex )
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "failed to report replica to subscriber [%s], caught ice exception[%s]"),
				subscriberEP.c_str(), ex.ice_name().c_str()	);
		}
		catch(...)
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "failed to report replica to subscriber [%s], caught unknown exception"),
				subscriberEP.c_str() );
		}
	}

	return iReplicaReportInterval;
}

int SsServiceImpl::run( )
{
	uint32	inteval	= iReplicaReportInterval;	
	do
	{
		inteval	= iReplicaReportInterval;

		if( !mReplicaSubscriberEP.empty()  )
			inteval = reportReplica();

		{
			ZQ::common::MutexGuard gd(mReplicaMutex);
			mReplicaReportCond.wait( mReplicaMutex, inteval );
		}

	} while ( !mbQuit );

	return 1;
}

size_t SsServiceImpl::sessionCount() const
{
	ZQ::common::MutexGuard gd(mSCMapLocker);
	return mContextToStreamMap.size();
}

void SsServiceImpl::registerStreamId(  const std::string& streamId, const std::string& contextKey )
{
	ZQ::common::MutexGuard gd(mSCMapLocker);
	mStreamToContextMap[streamId] = contextKey;
	CONTEXTTOSTREAMMAP::iterator it = mContextToStreamMap.find(contextKey);
	if( it != mContextToStreamMap.end() )
	{
		it->second.insert(streamId);
	}
	else
	{
		STRINGSET s;
		s.insert( streamId );
		mContextToStreamMap.insert( CONTEXTTOSTREAMMAP::value_type( contextKey, s ) );
	}
#if defined _DEBUG || defined DEBUG
	ENVLOG(ZQ::common::Log::L_DEBUG, 
		CLOGFMT(registerStreamId, "streamId[%s] registered with context[%s], current Stream2ContextKey count[%u] Context2Stream count[%u]"),
		streamId.c_str(), contextKey.c_str(), mStreamToContextMap.size(), mContextToStreamMap.size() );
#endif
}

void SsServiceImpl::unregisterStreamId( const std::string& streamId )
{
	ZQ::common::MutexGuard gd(mSCMapLocker);
	STRINGMAP::iterator it = mStreamToContextMap.find( streamId);
	if( it != mStreamToContextMap.end() )
	{
		const std::string& contextkey = it->second;
		CONTEXTTOSTREAMMAP::iterator itContext = mContextToStreamMap.find( contextkey );
		if( itContext != mContextToStreamMap.end() )
		{
			itContext->second.erase(streamId);
// 			if( itContext->second.empty())
// 				mContextToStreamMap.erase( itContext );
		}
		mStreamToContextMap.erase(it);
#if defined _DEBUG || defined DEBUG
		ENVLOG(ZQ::common::Log::L_INFO,
			CLOGFMT(unregisterStreamId, "unregister streamId[%s] current Stream2ContextKey count[%u] ContextKey2Stream count[%u]"),
			streamId.c_str(),
			mStreamToContextMap.size(),
			mContextToStreamMap.size()	);
#endif
	}
}

void SsServiceImpl::unregisterStreamIdByContextKey( const std::string& contextKey )
{
	ZQ::common::MutexGuard gd(mSCMapLocker);
	CONTEXTTOSTREAMMAP::iterator itContext = mContextToStreamMap.find( contextKey );
	if( itContext != mContextToStreamMap.end() )
	{
		const STRINGSET& streams = itContext->second;
		STRINGSET::const_iterator itStreamId = streams.begin();
		for( ; itStreamId != streams.end() ; itStreamId ++ )
		{
			mStreamToContextMap.erase(*itStreamId);
#if defined _DEBUG || defined DEBUG
			ENVLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(unregisterStreamId, "unregister streamId[%s] current Stream2ContextKey count[%u] ContextKey2Stream count[%u]"),
				itStreamId->c_str(),
				mStreamToContextMap.size(),
				mContextToStreamMap.size() );
#endif			
		}
		mContextToStreamMap.erase( itContext );
	}
}

TianShanIce::Streamer::SsPlaylistPrx SsServiceImpl::getSsStreamProxy( const std::string& streamSessId, bool bRemove) 
{
	Ice::Identity objId ;
	objId.category = EVICTOR_NAME_STREAM;
	try
	{
		ZQ::common::MutexGuard gd(mSCMapLocker);
		
		STRINGMAP::iterator it =  mStreamToContextMap.find( streamSessId );
		if( it == mStreamToContextMap.end() )
			return NULL;

		objId.name = it->second;
		if( bRemove )
			unregisterStreamId(streamSessId);
	}
	catch( ... )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "unknown exception caught when converting stream id to context key"));
	}

	if( objId.name.empty() )
		return NULL;

	int64 timeStart = ZQ::common::now();
	TianShanIce::Streamer::SsPlaylistPrx ss = GETOBJECT( TianShanIce::Streamer::SsPlaylistPrx, objId );
	int64 timeEnd = ZQ::common::now();
	ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "took [%lld]ms to load instance for [%s] from db( or maybe from memory)"),
		timeEnd-timeStart, objId.name.c_str() );
	return ss;
}
void SsServiceImpl::innerOnStreamEvent(StreamEvent event, const std::string& streamId, const StreamParams& currentParams, const ::TianShanIce::Properties& uparams)
{
	int64 timeStart = ZQ::common::now();
	const char* evName = "";
	switch (event )
	{
	case seNew:
		{//just ignore
			evName = "seNew";
		}
		break;
	case seStateChanged:
		{//
			// 			if( !( currentParams.mask & MASK_STATE ))
			// 			{
			// 				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seStateChanged but no stream state is available"));
			// 				return;
			// 			}
			evName = "seStateChanged";
			try
			{
				TianShanIce::Streamer::SsPlaylistPrx plPrx = getSsStreamProxy(streamId);
				if( plPrx )
				{
					plPrx->onSessionStateChanged( currentParams.streamState, streamId, ZQTianShan::now(), uparams );					
				}
			}
			catch( const TianShanIce::BaseException& ex )
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seStateChanged, stream[%s] caught exception[%s]: %s"),
					streamId.c_str(), ex.ice_name().c_str(), ex.message.c_str() );
			}
			catch( const Ice::Exception& ex)
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seStateChanged, stream[%s] caught exception[%s]"),
					streamId.c_str(), ex.ice_name().c_str() );
			}
			catch( ... )
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seStateChanged, stream[%s] caught exception"),
					streamId.c_str());
			}
		}
		break;

	case seScaleChanged:
		{
			// 			if( !( currentParams.mask & MASK_SCALE ))
			// 			{
			// 				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seScaleChanged but no stream scale is available"));
			// 				return;
			// 			}

			evName = "seStateChanged";
			try
			{
				TianShanIce::Streamer::SsPlaylistPrx pl = 	getSsStreamProxy(streamId);
				if(pl)
					pl->onSessionSpeedChanged( currentParams.scale, streamId, ZQTianShan::now(), uparams );
			}
			catch( const TianShanIce::BaseException& ex )
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seScaleChanged, stream[%s] caught exception[%s]: %s"),
					streamId.c_str(), ex.ice_name().c_str(), ex.message.c_str() );
			}
			catch( const Ice::Exception& ex)
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seScaleChanged, stream[%s] caught exception[%s]"),
					streamId.c_str(), ex.ice_name().c_str() );
			}
			catch( ... )
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seScaleChanged, stream[%s] caught exception "),
					streamId.c_str());
			}
		}
		break;

	case seProgress:
		{
			if( !( currentParams.mask & MASK_TIMEOFFSET ))
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seProgress but no timeOffset is available"));
				return;
			}
			evName = "seProgress";
			try
			{				
				TianShanIce::Streamer::SsPlaylistPrx pl = 	getSsStreamProxy(streamId  );				
				if(pl)
					pl->onSessionProgress( streamId, currentParams.timeoffset, currentParams.duration, uparams );				
			}
			catch( const TianShanIce::BaseException& ex )
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seProgress, stream[%s] caught exception[%s]: %s"),
					streamId.c_str(), ex.ice_name().c_str(), ex.message.c_str() );
			}
			catch( const Ice::Exception& ex)
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seProgress, stream[%s] caught exception[%s]"),
					streamId.c_str(), ex.ice_name().c_str() );
			}
			catch( ... )
			{
				ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamEvent, "seProgress, stream[%s] caught exception"),
					streamId.c_str());
			}		
		}
		break;

	case seGone:
		{	
			evName = "seGone";
			try
			{				
				TianShanIce::Streamer::SsPlaylistPrx pl = 	getSsStreamProxy(streamId, true );				
				if(pl)
					pl->onSessionExpired( streamId, ZQTianShan::now(), uparams );				
			}
			catch( const Ice::ObjectNotExistException&)
			{
			}
			catch( const TianShanIce::BaseException& ex )
			{
				ENVLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(OnStreamEvent, "seGone, stream[%s] caught exception[%s]: %s"),
					streamId.c_str(), ex.ice_name().c_str(), ex.message.c_str() );
			}
			catch( const Ice::Exception& ex)
			{
				ENVLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(OnStreamEvent, "seGone, stream[%s] caught exception[%s]"),
					streamId.c_str(), ex.ice_name().c_str() );
			}
			catch( ... )
			{
				ENVLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(OnStreamEvent, "seGone, stream[%s] caught unknown exception"),
					streamId.c_str());
			}			
		}
		break;

	default:
		{
			evName = "unknown";
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "OnStreamEvent() unknown event"));
		}
		break;
	}

	int64 timeEnd = ZQ::common::now();
	ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "OnStreamEvent(), took [%lld]ms to send the event[%s] of[%s] out, including db access time"),
		timeEnd - timeStart, evName, streamId.c_str() );
}

class DispatchEventRequest : public ZQ::common::ThreadRequest {
public:
	DispatchEventRequest( ZQ::common::NativeThreadPool& pool, SsServiceImpl* ss, SsEnvironment* e)
		: ZQ::common::ThreadRequest(pool), mSs(ss), env(e) {
	  }
	virtual ~DispatchEventRequest(){ }
	void set( SsServiceImpl::StreamEvent event, const std::string& streamId, 
		const StreamParams& currentParams, const ::TianShanIce::Properties& uparams ) {
			this->event = event;
			this->streamId = streamId;
			this->streamParams = currentParams;
			this->uparams = uparams;
	}

protected:
	int run()
	{
		try {
			mSs->innerOnStreamEvent(event, streamId, streamParams, uparams);
		}
		catch(...) {
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(DispatchEventRequest, "caught unknown exception while dispatching event"));
		}
		return 0;
	}
	void final(int retcode /* =0 */, bool bCancelled /* =false */)
	{
		delete this;
	}

private:
	SsServiceImpl*		mSs;
	SsEnvironment*		env;
	SsServiceImpl::StreamEvent			event;
	std::string			streamId;
	StreamParams		streamParams;
	TianShanIce::Properties uparams;
};

void SsServiceImpl::OnStreamEvent( StreamEvent event, const std::string& streamId, 
								  const StreamParams& currentParams, const ::TianShanIce::Properties& uparams )
{
	DispatchEventRequest* p =  new DispatchEventRequest(mDispatchEventThdPool, this, env);
	p->set( event, streamId, currentParams, uparams );
	p->start();
}

void SsServiceImpl::OnReplicaEvent( SsReplicaEvent event, const std::string& streamerId, const ::TianShanIce::Properties& uparams)
{	
	ZQ::common::MutexGuard gd(mStreamerInfoLocker);
	SsReplicaInfoS::iterator it = mReplicaInfos.find( streamerId );
	if( it == mReplicaInfos.end() )
	{

		ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "OnStreamerEvent() new streamer[%s] is found"), streamerId.c_str());
		refreshStreamer();
		it = mReplicaInfos.find( streamerId);
		if( it == mReplicaInfos.end() )
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "OnStreamerEvent() can't find streamer info with streamerId[%s]"),
				streamerId.c_str()	);
			return ;
		}		
	}

	switch( event )
	{
	case sreInService:
		{			
			ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "OnStreamerEvent() [sreInService] with streamer[%s]"), streamerId.c_str() );
			SsReplicaInfo& infoEx = it->second;
			if ( infoEx.stampBorn == 0 )
			{
				infoEx.stampBorn = ZQTianShan::now();
				ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "OnStreamerEvent() streamer[%s] first up"), streamerId.c_str() );
			}

			if( infoEx.replicaState	!= TianShanIce::stInService )
			{
				infoEx.stampChanged		=	ZQTianShan::now();
				infoEx.replicaState		=	TianShanIce::stInService;
			}

			ZQTianShan::Util::mergeProperty( infoEx.props, uparams, true );
		}
		break;

	case sreOutOfService:
		{
			ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SsServiceImpl, "OnStreamerEvent() [sreOutOfService] with streamer[%s]"), streamerId.c_str() );
			SsReplicaInfo& infoEx = it->second;			
			if( infoEx.replicaState	!= TianShanIce::stInService )
			{
				infoEx.stampChanged		=	ZQTianShan::now();
				infoEx.replicaState		=	TianShanIce::stInService;
			}
			ZQTianShan::Util::mergeProperty( infoEx.props, uparams );
		}
		break;

	default:
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "OnStreamerEvent() invalid event"));
			return;
		}
		break;
	}

	mReplicaReportCond.signal();
}


namespace Util{

MemoryServantLocator::MemoryServantLocator(Ice::ObjectAdapterPtr a, const std::string& dbName )
:mAdapater(a),
mSize(100),
mDbName(dbName)
{
}

::Ice::ObjectPrx MemoryServantLocator::add(const ::Ice::ObjectPtr& obj, const ::Ice::Identity& id)
{
	{
		ZQ::common::MutexGuard gd(mLocker);
		if(mServants.find(id.name) != mServants.end())
		{
			throw Ice::AlreadyRegisteredException("Object already exist", 0);
		}

		mServants.insert( std::map<std::string, Ice::ObjectPtr>::value_type(id.name, obj));
	}

	return mAdapater->createProxy(id);
}


::Ice::ObjectPtr MemoryServantLocator::remove(const ::Ice::Identity& id)
{
	::Ice::ObjectPtr p;
	{
		ZQ::common::MutexGuard gd(mLocker);
		std::map<std::string, Ice::ObjectPtr>::iterator it = mServants.find(id.name);
		if(it != mServants.end() )
		{			
			p = it->second;
			mServants.erase(it);
		}
	}
	return p;
}

std::vector<Ice::Identity> MemoryServantLocator::getIds() const
{
	std::vector<Ice::Identity> ids;
	{
		ZQ::common::MutexGuard gd(mLocker);
		std::map<std::string, Ice::ObjectPtr>::const_iterator it = mServants.begin();
		for( ; it != mServants.end() ; it ++ )
		{
			Ice::Identity id ;id.category = mDbName;
			id.name = it->first;
			ids.push_back(id);
		}
	}

	return ids;
}


void MemoryServantLocator::keep(const ::Ice::Identity&){}	

void MemoryServantLocator::release(const ::Ice::Identity&){}

bool MemoryServantLocator::hasObject(const ::Ice::Identity& id)
{
	ZQ::common::MutexGuard gd(mLocker);
	return mServants.find(id.name) != mServants.end();
}

Freeze::EvictorIteratorPtr MemoryServantLocator::getIterator(const ::std::string&, ::Ice::Int)
{
	return new MemoryServantLocatorIterator(*this);
}

Ice::ObjectPtr MemoryServantLocator::locate(const ::Ice::Current& ic, ::Ice::LocalObjectPtr& cookie)
{
	if( ic.id.category != mDbName )
		return NULL;
	cookie = NULL;
	Ice::ObjectPtr p = NULL;
	{
		ZQ::common::MutexGuard gd(mLocker);
		std::map<std::string, Ice::ObjectPtr>::iterator it = mServants.find(ic.id.name);
		if( it !=mServants.end())
			p =it->second;
	}
	return p;
}

void MemoryServantLocator::finished(const ::Ice::Current&, const ::Ice::ObjectPtr&, const ::Ice::LocalObjectPtr&)
{
}

void MemoryServantLocator::deactivate(const ::std::string&) 
{
}

////////////////////
MemoryServantLocatorIterator::MemoryServantLocatorIterator(MemoryServantLocator& locater)
:mIndex(0)
{
	mIds = locater.getIds();
}

bool MemoryServantLocatorIterator::hasNext()
{
	return mIndex < mIds.size();
}

Ice::Identity MemoryServantLocatorIterator::next()
{
	Ice::Identity id;
	if(hasNext())
	{
		id = mIds[mIndex++];		
	}

	return id;
}

}

}}//namespace ZQ::StreamService

