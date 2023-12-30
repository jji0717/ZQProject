#ifndef _STREAM_SMITH_ADMIN_IMPLEMENT_H__
#define	_STREAM_SMITH_ADMIN_IMPLEMENT_H__

#include "FailStorePlInfo.h"
#include <TsSRM.h>
#include "StreamSmithAdmin.h"
#include "PlaylistInternalUse.h"
#include <IceUtil/IceUtil.h>
//#include <playlist.h>
#include <tstransport.h>
#include <StreamSmithModuleEx.h>

namespace ZQ
{
namespace StreamSmith
{
class PlaylistManager;	
class StreamSmithI:public TianShanIce::Streamer::StreamSmithAdmin,public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:
	StreamSmithI(IPlaylistManager* pMan,ZQADAPTER_DECLTYPE adapter,char* srvNetID,const std::vector<int>& edgeCards);
	~StreamSmithI();
public:
	::TianShanIce::Streamer::PlaylistIDs listPlaylists(const ::Ice::Current&);
	   
    ::TianShanIce::Streamer::PlaylistPrx openPlaylist(const ::std::string& guid,const ::TianShanIce::Streamer::SpigotBoards& edgeCards, bool bCreate, const ::Ice::Current& = ::Ice::Current() ) ;

	::TianShanIce::Streamer::StreamPrx createStream(const ::TianShanIce::Transport::PathTicketPrx& ticket, const ::Ice::Current& ic= ::Ice::Current());
	
	::TianShanIce::Streamer::StreamPrx createStreamByResource(const ::TianShanIce::SRM::ResourceMap& res, const TianShanIce::Properties& props, const ::Ice::Current& = ::Ice::Current());

	::TianShanIce::Streamer::StreamPrx openStream(const ::std::string&, const ::Ice::Current& /* = ::Ice::Current */);
	
	::std::string	ShowMemory(const ::Ice::Current& = ::Ice::Current());

    ::std::string getAdminUri(const ::Ice::Current& = ::Ice::Current());
    
    ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current());

	::TianShanIce::Streamer::StreamerDescriptors listStreamers(const ::Ice::Current& ic= ::Ice::Current());
	
    ::std::string getNetId(const ::Ice::Current& ic= ::Ice::Current()) const;
    
	void queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr&, const ::std::string&, const ::std::string&, bool, const ::Ice::Current& = ::Ice::Current()) ;
private:
	::TianShanIce::Streamer::StreamPrx createStreamByServiceGroup(const ::TianShanIce::SRM::ResourceMap& res,
																	const ::Ice::Current&);
	::TianShanIce::Streamer::StreamPrx createStreamByStreamer(const TianShanIce::SRM::ResourceMap& res,
																	TianShanIce::Variant& varStreamer,
																	const ::Ice::Current&);
	void createStreamBatch_async(const ::TianShanIce::Streamer::AMD_StreamSmithAdmin_createStreamBatchPtr&, const ::TianShanIce::Streamer::StreamBatchRequest&, const ::Ice::Current& = ::Ice::Current());

private:
	
	IPlaylistManager*							_plManager;
	ZQADAPTER_DECLTYPE							_pAdpter;
	std::string									_srvNetID;
	
	::TianShanIce::Streamer::SpigotBoards		_edgeCards;
	
};

}}//namespace

#endif//_STREAM_SMITH_IMPLEMENT_H__