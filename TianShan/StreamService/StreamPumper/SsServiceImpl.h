#ifndef __ZQ_TianShan_StreamService_StreamServiceImpl_h__
#define __ZQ_TianShan_StreamService_StreamServiceImpl_h__


#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include "Playlist.h"
#include "SsEnvironment.h"
#include "EventSender.h"
#include "renewTicket.h"





namespace ZQ
{
namespace StreamService
{
namespace Util
{
class MemoryServantLocator;
class MemoryServantLocatorIterator : public Freeze::EvictorIterator
{
public:
	MemoryServantLocatorIterator(MemoryServantLocator& locater);
	virtual bool hasNext();
	virtual ::Ice::Identity next();
private:
	std::vector<Ice::Identity> mIds;
	size_t						mIndex;
};
typedef IceUtil::Handle<MemoryServantLocatorIterator> MemoryServantLocatorIteratorPtr;

class MemoryServantLocator : public Freeze::Evictor
{
public:	

	MemoryServantLocator( Ice::ObjectAdapterPtr a , const std::string& name);

	virtual void setSize(::Ice::Int size){mSize = size;}
	virtual ::Ice::Int getSize(){return mSize;}
	virtual ::Ice::ObjectPrx add(const ::Ice::ObjectPtr&, const ::Ice::Identity&);	
	virtual ::Ice::ObjectPtr remove(const ::Ice::Identity&);
	virtual void keep(const ::Ice::Identity&);	
	virtual void release(const ::Ice::Identity&);	
	virtual bool hasObject(const ::Ice::Identity&);	
	virtual ::Freeze::EvictorIteratorPtr getIterator(const ::std::string&, ::Ice::Int);

	virtual ::Ice::ObjectPrx addFacet(const ::Ice::ObjectPtr&, const ::Ice::Identity&, const ::std::string&){return NULL;}
	virtual ::Ice::ObjectPtr removeFacet(const ::Ice::Identity&, const ::std::string&) {return NULL;}
	virtual void keepFacet(const ::Ice::Identity&, const ::std::string&) {}
	virtual void releaseFacet(const ::Ice::Identity&, const ::std::string&) {}
	virtual bool hasFacet(const ::Ice::Identity&, const ::std::string&) {return false;}

	std::vector<Ice::Identity> getIds() const;

protected:
	virtual ::Ice::ObjectPtr locate(const ::Ice::Current&, ::Ice::LocalObjectPtr&) ;

	virtual void finished(const ::Ice::Current&, const ::Ice::ObjectPtr&, const ::Ice::LocalObjectPtr&);

	virtual void deactivate(const ::std::string&) ;

private:
	Ice::ObjectAdapterPtr		mAdapater;
	ZQ::common::Mutex			mLocker;	
	std::map<std::string , Ice::ObjectPtr>  mServants;
	int					mSize;//dummy
	std::string			mDbName;
};
typedef IceUtil::Handle<MemoryServantLocator> MemoryServantLocatorPtr;
}//namespace ZQ::StreamService::Util



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifdef IN
#undef IN
#endif

#ifdef OUT
#undef OUT
#endif

#ifdef INOUT
#undef INOUT
#endif

#define IN
#define	OUT
#define INOUT

#define		ERR_RETURN_SUCCESS					0
#define		ERR_RETURN_INVALID_PARAMATER		-1
#define		ERR_RETURN_INVALID_STATE_OF_ART		-2
#define		ERR_RETURN_SERVER_ERROR				-3
#define		ERR_RETURN_NOT_SUPPORT				-4
#define		ERR_RETURN_NOT_IMPLEMENT			-5
#define		ERR_RETURN_OBJECT_NOT_FOUND			-6 //Ice::ObjectNotExist will be thrown out
#define		ERR_RETURN_ASSET_NOTFOUND			-7 // TianShanIce::InvalidParameter with error code errcodeAssetNotFound


#define MASK_CONTENT_STATE			( 1 << 0 )
#define MASK_TIMEOFFSET				( 1 << 1 )
#define MASK_SCALE					( 1 << 2 )
#define MASK_CONTENT_DURATION		( 1 << 3 )	//content play duration
#define MASK_STATE					( 1 << 4 )	//This is stream' state
#define MASK_STREAM_STATE			( 1 << 4 )
#define MASK_SESSION_RESTORE		( 1 << 5 )

#define MASK_TIMEOFFSET_WANTED		( 1 << 10 )
#define MASK_CONTENT_STATE_WANTED	( 1 << 11 )
#define MASK_SCALE_WANTED			( 1 << 12 )
#define MASK_CONTENT_DURATION_WANTED ( 1 << 13 )
#define MASK_STREAM_STATE_WANTED	( 1 << 14 )

struct StreamParams
{
	//mask use to specify which member should be filled
	///< parameter member mask, this member let porting layer know which value the lib concerned
	/// after porting layer executed command, porting layer should set mask to let lib know which member is valid
	int32								mask; 

	///< timeoffset in milliseconds
	int64								timeoffset;

	///< stream play duration in milliseconds
	int64								duration;

	///< stream scale
	float								scale;

	TianShanIce::Streamer::StreamState	streamState;

	TianShanIce::Storage::ContentState	contentState;

	StreamParams( )
	{
		memset(this, 0, sizeof(StreamParams));
	}
};


typedef std::vector<TianShanIce::Streamer::PlaylistItemSetupInfo>	PlaylistItemSetupInfos;

typedef std::list<std::string>	PortIds;

struct SsReplicaInfo : public TianShanIce::Replica 
{
	bool					bHasPorts;
	std::list<std::string>	ports;//streaming ports, for streamsmith
	std::string				streamerType; // streamer type, weiwoo need this
	bool					bStreamReplica; // true if this replica is a streamer, false if not
	SsReplicaInfo()
	{
		bHasPorts			= false;
		priority			= 50;
		replicaState		= TianShanIce::stOutOfService;
		maxPrioritySeenInGroup = 1;
		stampBorn			= 0;
		stampChanged		= 0;
		bStreamReplica		= true;
	}
};

typedef std::map<std::string , SsReplicaInfo> SsReplicaInfoS;


class SsContext
{
public:
	virtual ~SsContext() {};

	virtual const std::string& id( ) const = 0 ;//get the context identity in string format

	virtual void	updateContextProperty( const std::string& key , const std::string& value ) = 0;
	virtual void	updateContextProperty( const std::string& key , int32 value ) = 0;
	virtual void	updateContextProperty( const std::string& key , int64 value ) = 0;
	virtual void	updateContextProperty( const std::string& key , float value ) = 0;

	virtual void	updateContextProperty( const TianShanIce::Properties& props ) = 0;

	virtual bool	hasContextProperty( const std::string& key ) const = 0;

	virtual std::string getContextProperty( const std::string& key ) const = 0;

	virtual const TianShanIce::Properties& getContextProperty( ) const = 0; //use this in your own risk

	virtual const TianShanIce::SRM::ResourceMap& getContextResources( ) const =  0; //use this in your own risk

	virtual void  updateContextResources( const TianShanIce::SRM::ResourceMap& res ) = 0;

	virtual	void	removeProperty( const std::string& key ) = 0;

	virtual std::string getStreamingPort ( ) const = 0 ;
};


class SsServiceImpl : public TianShanIce::Streamer::StreamServiceAdmin , public ZQ::common::NativeThread
{
public:
	typedef enum _StreamEvent
	{
		seNew,
		seStateChanged,
		seScaleChanged,
		seProgress,
		seGone,
	} StreamEvent;

	SsServiceImpl( SsEnvironment* environment, const std::string& serviceId );
	~SsServiceImpl(void);
public:
	///start stream service instance
	///@return true if start ok, vice versa
	///@param dbPath database path 
	///@param eventChannelEndpoint , endpoint of the event channel which is used to publish stream event through
	///@param replicaSubscriberEndpoint , endpoint of the replica subscriber 
	///										replicaSubsciber is a instance which listen to streamer replica's event and 
	///										may use the event information to decide use this streamer or not
	bool					start(	const std::string& dbPath ,
									const std::string& eventChannelEndpoint , 
									const std::string& replicaSubscriberEndpoint,
									const std::string& serviceNetId,
									const std::string& serviceName );

	///stop stream service instance
	void					stop( );

public:
	
	virtual ::TianShanIce::Streamer::SsPlaylistS listSessions(const ::Ice::Current& = ::Ice::Current()) ;
	
	TianShanIce::Streamer::PlaylistIDs		listPlaylists(const ::Ice::Current& = ::Ice::Current()) ;

	TianShanIce::Streamer::PlaylistPrx		openPlaylist(const ::std::string&, const ::TianShanIce::Streamer::SpigotBoards&, bool, const ::Ice::Current& = ::Ice::Current());
	
	TianShanIce::Streamer::StreamPrx		createStreamByResource(const ::TianShanIce::SRM::ResourceMap&, const TianShanIce::Properties& props, const ::Ice::Current& = ::Ice::Current()) ;

	TianShanIce::Streamer::StreamPrx		createStream(const ::TianShanIce::Transport::PathTicketPrx&, const ::Ice::Current& = ::Ice::Current()) ;

	TianShanIce::Streamer::StreamPrx		openStream( const std::string& id , const Ice::Current& = ::Ice::Current() );
	
	TianShanIce::Streamer::StreamerDescriptors listStreamers(const ::Ice::Current& = ::Ice::Current()) ;
	
	std::string								getNetId(const ::Ice::Current& = ::Ice::Current()) const ;

	std::string								getAdminUri(const ::Ice::Current& = ::Ice::Current()) ;

	TianShanIce::State						getState(const ::Ice::Current& = ::Ice::Current()) ;	

	void									queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr&, 
																const ::std::string&, const ::std::string&,
																bool, const ::Ice::Current& = ::Ice::Current()) ;
	
	SsEventManager&							getEventSender( )
	{
		return	mEventSenderManager;
	}
	
	RenewTicketCenter&						getRenewTicketCenter( )
	{
		return mTicketRenewCenter;
	}

	TianShanIce::Transport::PathTicketPrx	getPathTicket( const std::string& contextKey );//trick for CDN

	//helper function
	bool									isObjectExist( const Ice::Identity& id );

	void									registerStreamId(  const std::string& streamId , const std::string& contextKey );	

	size_t									sessionCount() const;

private:

	int										run(void);
	
	friend class SsStreamImpl;

	bool									openDatabase( const std::string& dbPath );

	void									closeDatabase( );

	bool									addServantToDB( TianShanIce::Streamer::SsPlaylistPtr s , const Ice::Identity& id);

	void									removeServantFromDB( const Ice::Identity& id  , const TianShanIce::Properties& resource );

	void									updateIceProperty(Ice::PropertiesPtr iceProperty , 
																const std::string& key ,  
																const std::string& value );	
	uint32									reportReplica( );

	void									restoreSessions( );

	
	
	void									unregisterStreamId( const std::string& streamId );	
	void									unregisterStreamIdByContextKey( const std::string& contextKey);

	void									collectStreamerReplicas( TianShanIce::Replicas& reps );

	TianShanIce::Streamer::SsPlaylistPrx	getSsStreamProxy( const std::string& streamSessId ,bool bRemove = false ) ;

private:	

	TianShanIce::Streamer::StreamPrx		createStreamByStreamer( const std::string& streamId, const ::TianShanIce::SRM::ResourceMap& resource , const std::string& strStreamerNetId);
	
	TianShanIce::Streamer::StreamPrx		doCreateStream( const std::string& streamId,
															const TianShanIce::SRM::ResourceMap& srmResources,
															TianShanIce::Transport::StreamLinkPrx streamLink,
															const std::string& streamerNetId,
															TianShanIce::Transport::PathTicketPrx ticket ,
															const TianShanIce::Properties& props );
	
	bool									allocateStreamingResource( const std::string& streamerId ,
																		const TianShanIce::SRM::ResourceMap& srmResources ,
																		TianShanIce::Properties& streamingRes );

	void									releaseStreamingResource( const TianShanIce::Properties& streamingRes );


	bool									reuseStreamPort( const std::string& contextKey );

	bool									refreshStreamer( );

	void									innerOnStreamEvent( StreamEvent event, const std::string& streamId, const StreamParams& currentParams, const ::TianShanIce::Properties& uparams);

	friend class DispatchEventRequest;
private:

	typedef std::map<std::string , std::string>	STRINGMAP;
	//               stream id     context key
	STRINGMAP					mStreamToContextMap;

	typedef std::set<std::string>		STRINGSET;
	typedef std::map<std::string , STRINGSET>	CONTEXTTOSTREAMMAP;
	CONTEXTTOSTREAMMAP			mContextToStreamMap;

	ZQ::common::Mutex			mSCMapLocker;

	
	Ice::Identity				localId;

	TianShanIce::State			serviceState;

	std::string					strAdminUrl;

	std::string					strNetId;

	SsEnvironment*				env;

	Freeze::EvictorPtr			mStreamEvictor;

	IceEventSender				mEventSender;

	SsEventManager				mEventSenderManager;

	std::string					mReplicaSubscriberEP;

	ZQ::common::Mutex			mReplicaMutex;

	bool						mbQuit;

	RenewTicketCenter			mTicketRenewCenter;

	
	SsReplicaInfoS				mReplicaInfos;
	ZQ::common::Mutex			mStreamerInfoLocker;
	ZQ::common::Cond			mReplicaReportCond;

	ZQ::common::NativeThreadPool	mDispatchEventThdPool;

public:

	//configuration for SsServiceImpl running
	//	
	std::string					strCheckpointPeriod;
	std::string					strDbRecoverFatal;
	std::string					strSavePeriod;
	std::string					strSaveSizeTrigger;
	Ice::Int					iEvictorStreamSize;
	Ice::Int					iReplicaReportInterval;
	bool						bShowDBOperationTimeCost;

// protected:
// 
// 	///virtual function 
// 	///porting layer can override these function if it need
// 	virtual	void			onRestoreStart( ){;}
// 
// 	virtual void			onRestoreEnd( const std::vector<Ice::Identity>& ids ){;}

public:

	///if stream expired , call this routine
	///@param event stream event	
	///@param streamId stream's identifier
	///@param currenParams stream's attribute
	///@param uparams reserved for future use
	void OnStreamEvent( StreamEvent event, const std::string& streamId, const StreamParams& currentParams, const ::TianShanIce::Properties& uparams);

	typedef enum _SsReplicaEvent
	{
		sreInService,
		sreOutOfService,
		sreAttributeChanged,
	} SsReplicaEvent;

	///if a streamer's state changed, call this routine
	///@param event streamer event
	///@param streamerId streamer's identifier
	///@param uparams reserved for future use
	void OnReplicaEvent( SsReplicaEvent event, const std::string& replicaId, const ::TianShanIce::Properties& uparams);

public:
	
	//porting layer should implement routines below
	/**************************************************************************************************/	
	///list all streamers	
	///return if porting layer can't list streamer , return false , lib will refuse to continue startup
	/// other wise , return true
	///@if the streamer is good to stream , set streamerState to InService , other wise , set it to OutOfService	
	static bool	listAllReplicas( SsServiceImpl& ss, OUT SsReplicaInfoS& infos );


	///porting layer should confirm that the streamer port can be allocated to a new stream
	///If the port can be used , return true , other wise , return false
	///@param streamerId , streamer's identifier
	///@param portId , streamer port's identifier 
	///@param resource , resource used to create stream
	static bool allocateStreamResource(	SsServiceImpl& ss, 
											IN const std::string& streamerReplicaId ,
											IN const std::string& portId ,
											IN const TianShanIce::SRM::ResourceMap&	resource );

	///this routine let porting layer known that lib is going to release a stream port
	static bool releaseStreamResource( SsServiceImpl& ss, SsContext& ctx,
											IN const std::string& streamerReplicaId,
											OUT TianShanIce::Properties& feedback  );

	/**************************************************************************************************/

	///contextKey is a unique id which can be thought as a TianShanIce::Stream's identity
	///Validate item , return true if item is valid , return false if item can be accepted
	///@param info , setup information
	static int32	doValidateItem( SsServiceImpl& ss, 
									IN SsContext& ctx,
									INOUT TianShanIce::Streamer::PlaylistItemSetupInfo& info );

	///commit stream
	///@return true if success , vice versa
	///@param items , all playlist items
	///@param contextKey , this key is unique 
	///@param streamPortId , streamer port's identifier
	///@param items , all items belong to this playlist
	///@param crResources , client request resource map
	///@playlistParams , playlist's parameter
	///						this parameter is used to afford per playlist's atribute to porting layer
	///						and porting layer can put parameters into this parameter to let lib know
	/// STREAMINGRESOURCE_STREAMING_SOUCREIP		===>> Streaming source Ip
	/// STREAMINGRESOURCE_STREAMING_SOURCEPORT	===>> Streaming source Port
	///	 please see the macro definition below

	static int32 doCommit( SsServiceImpl& ss, 
								IN SsContext& ctx,								
								IN PlaylistItemSetupInfos& items,
								IN TianShanIce::SRM::ResourceMap& requestResources );

	///load stream and ready to play
	///@return true is success , vice versa
	///@param itemInfo , the item information use to load , if only stream wide operation is permit , 
	///					this parameter can be ignored
	///@param timeoffset , item npt relative to begin of this item
	///@param scale , stream's scale ,do not change scale if scale == 0.0f
	///@param playlistParams, parameters use to setup a streaming session
	///		following are the keys for setting up a streaming session
	///			PROP_DESTINATION_IPADDRESS	--- destination ip address can be IPv4 or IPv6 
	///			PROP_DESTINATION_UDPPORT		--- destination udp port 
	///			PROP_DESTINATION_MACADDRESS	--- destination mac address
	///			PROP_MUXRATE_MAX				--- MUX stream's max bitrate
	///			please see the macro definition below
	static int32 doLoad(	SsServiceImpl& ss,
								SsContext& contextKey,								
								IN const TianShanIce::Streamer::PlaylistItemSetupInfo& itemInfo, 
								IN int64 timeoffset,
								IN float scale,								
								INOUT StreamParams& ctrlParams,
								OUT std::string&	streamId );

	///play the stream specified with streamId and StreamAttribute must be returned from porting layer
	///@return true if success , false if failed
	static int32	doPlay(	 SsServiceImpl& ss ,
								SsContext& ctx,								
								IN const std::string& streamId,
								IN int64 timeOffset,
								IN float scale,
								INOUT StreamParams& ctrlParams );

	///pause the stream specified with streamId
	static int32	doPause(	 SsServiceImpl& ss ,
								SsContext& ctx,								
								IN const std::string& streamId , 						
								INOUT StreamParams& ctrlParams );

	///resume the stream specified with streamId
	static int32	doResume(  SsServiceImpl& ss ,
								SsContext& ctx,								
								IN const std::string& streamId ,						
								INOUT StreamParams& ctrlParams );

	///reposition a stream, this method can only be invoked when service support playlist	
	static int32	doReposition( SsServiceImpl& ss ,
								SsContext& ctx,								
								IN const std::string& streamId ,
								IN int64 timeOffset,
								IN const float& scale,
								INOUT StreamParams& ctrlParams );

	///change the stream's scale specified with streamId
	///@param newScale , new scale
	///@param attr , out value , must be filled according to mask
	static int32	doChangeScale(	 SsServiceImpl& ss ,
										SsContext& ctx,										
										IN const std::string& streamId ,								
										IN float newScale,
										INOUT StreamParams& ctrlParams );

	///destroy the stream according to streamId
	static int32	doDestroy(	 SsServiceImpl& ss ,
									SsContext& ctx,
									IN const std::string& streamId );
									

	///get stream's attribute
	///porting layer should return stream's attribute according to StreamAttribute.mask
	///@param contextKey unique key
	///@param streamId , the stream's identifier 
	///@param info , PlaylistItemSetupInfo associated with current stream 
	///@param mask , mask for 
	static int32	doGetStreamAttr(	 SsServiceImpl& ss ,
									IN	SsContext& ctx,
									IN	const std::string& streamId,
									IN  const TianShanIce::Streamer::PlaylistItemSetupInfo& info,
									OUT StreamParams& ctrlParams );	

};
#define STREAMINGRESOURCE(x)	"StreamingResource."#x


///Streamer device Id such as spigot00
///value type ==> string
#define		STREAMINGRESOURCE_STREAMERID_KEY					STREAMINGRESOURCE(StreamerDeviceId)

///Stream port , this is a virtual channel logically used for streaming
///value type ==> string
#define		STREAMINGRESOURCE_STREAMPORT_KEY					STREAMINGRESOURCE(StreamChannelPort)

///ip of streamer source address from where the stream is being pumped
///value type ==> string
#define		STREAMINGRESOURCE_STREAMING_SOUCREIP				STREAMINGRESOURCE(SourceIpAddress)

///port of streamer source address from where the stream is being pumped
///value type ==> Ice::Int
#define		STREAMINGRESOURCE_STREAMING_SOURCEPORT				STREAMINGRESOURCE(SourceUdpPort)

///ip of destination address 
///value type : string
#define		STREAMINGRESOURCE_DESTINATION_IPADDRESS				STREAMINGRESOURCE(DestinationIpAddress)

///port of destination address
///value type : Ice::Int
#define		STREAMINGRESOURCE_DESTINATION_UDPPORT				STREAMINGRESOURCE(DestinationUdpPort)

///mac of destination address
///value type : string
#define		STREAMINGRESOURCE_DESTINATION_MACADDRESS			STREAMINGRESOURCE(DestinationMacAddress)

///max bit rate of mux stream
///value type : Ice::Int
#define		STREAMINGRESOURCE_MUXRATE_MAX						STREAMINGRESOURCE(StreamMuxMaxBitRate)

#define		STREAMING_ATTRIBUTE_STREAM_SESSION_ID				STREAMINGRESOURCE(AttributeStreamSessionId)

}}//namespace ZQ::StreamService

#endif // __ZQ_TianShan_StreamService_StreamServiceImpl_h__
