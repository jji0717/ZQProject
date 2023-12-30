
#ifndef _ssm_ngod_resource_manager_header_file_h__
#define _ssm_ngod_resource_manager_header_file_h__

#include <set>
#include <map>
#include <vector>
#include <string>
#include <Locks.h>
#include <NativeThread.h>
#include "LAMFacade.h"
#include "ContentReplicaEx.h"
#include "RemoteAssetStack.h"

/*
ResourceManager是干什么的呢？
是这样的，它呢，主要就是管理一下Sop , streamer, volume和ImportChannel
所有的资源申请以及释放都是由这个Manager来管理的

但是有一个问题就是client给出一个选择范围，然后ResourceManager从这个范围中选择一个可以
使用的streamer，然后建立一个stream。如果建立stream失败，那么会为这个streamer打上一个penalty标签。
一直需要等到penalty标签消失以后，该streamer上才能创建新的stream
如果此后在这个建立的stream上面render失败的话会将这个streamer上的资源释放掉

嗯，讲的真是清楚，哈哈！赞！
*/

#include <map>
#include <set>
#include <vector>
#include <string>

#include "StreamSmithAdmin.h"


struct ResourceVolumeAttr 
{
	std::string									netId;
	std::set<std::string>						partitions;		// (netId/partition) stand for a specified volume , * for all partition	
	int											level;			// cache level
	bool										bSupportNas;	// true if the volume support cache content from remote content library, vice versa	
	bool										bSupportCache;	// if this value is true, current volume can be selected when rtsp message 's volume == library
	bool										bAllPartitions;	// hack for all partitions

	ResourceVolumeAttr()
	{
		bAllPartitions	= false;
		level			= 50; //middle level
		bSupportCache	= false;
		bSupportNas		= false;
	}	
};
struct  ResourceVolumeAttrCmp
{
	bool operator()( const ResourceVolumeAttr& a, const ResourceVolumeAttr& b ) const
	{
		return a.netId < b.netId;
	}
};

typedef std::map< std::string, ResourceVolumeAttr > ResourceVolumeAttrMap;


struct  VolumeNameAttr
{
	/*
	NOTICE: Now , we only care about netId in our deployment, may be in the future we must check partition as well
	*/
	std::string		netId;
	std::string		partition;
	bool			bDefaultPartition;

	VolumeNameAttr()
	{
		bDefaultPartition = false;
	}

	VolumeNameAttr( const VolumeNameAttr& a)
	{
		netId				= a.netId;
		partition			= a.partition;
		bDefaultPartition		= a.bDefaultPartition;
	}
	VolumeNameAttr& operator = ( const VolumeNameAttr& a )
	{
		netId				= a.netId;
		partition			= a.partition;
		bDefaultPartition		= a.bDefaultPartition;
		return *this;
	}

	bool operator == ( const VolumeNameAttr& a) const
	{
		return netId == a.netId;
	}

	bool operator != ( const VolumeNameAttr& a) const
	{
		return netId != a.netId;
	}
	bool operator < ( const VolumeNameAttr& a ) const
	{
		return netId < a.netId;
	}
	bool operator > ( const VolumeNameAttr& a ) const
	{
		return netId > a.netId;
	}

	std::string toString( ) const
	{
		std::string str;
		str = netId + "/" + ( bDefaultPartition ? "$" : partition );
		return str;
	}

};

typedef std::vector<VolumeNameAttr> VolumeNameAttrS;


struct ResourceImportChannelAttr 
{
	std::string									netId;
	int64										confMaxBw;			//max bandwidth count in byte
	int64										confMaxSessCount;	//max concurrent streams count
	int64										usedBw;
	int64										usedSessCount;
	int64										reportUsedBW;		//usedBW reported by stream service
	int64										reportMaxBW;		//totalBW reported by stream service
	int64										reportUsedSessCount;//usedSessCount reported by stream service

	ResourceImportChannelAttr()
	{
		confMaxBw						= 0;
		confMaxSessCount				= 0;
		usedBw							= 0;
		usedSessCount					= 0;
		reportUsedBW					= 0;
		reportMaxBW						= 0;
		reportUsedSessCount				= 0;
	}
};
typedef std::map< std::string , ResourceImportChannelAttr > ResourceImportChannelAttrMap;
//            ImportChannel name          attr

struct ResourceStreamerAttr 
{
	std::string									netId;
	std::string									endpoint;			//stream service's endpoint
	std::string									nodeId;
	unsigned long                               sourceIP;
	bool										bReplicaStatus;		//true if replica report normally and its status is ok, vice versa
	int64										lastReplicaUpdate;	//timestamp of last update of replica
	int64										maxBw;				
	int64										usedBw;
	int64										maxSessCount;
	int64										usedSessCount;
	TianShanIce::Streamer::StreamSmithAdminPrx	streamServicePrx;
	std::string									streamServiceEndpoint; // for log use only
	
	std::string									importChannelName;		//name of import channel relative to current streamer

	int32										penalty;

	int64										statisticsRemoteSessCount;
	int64										statisticsTotalSessCount;
	int64										statisticsFailedSessCount;
	int64										statisticsUsedSessCount;
	
	
	std::string									volumeNetId;		//volume which relative to current streamer
	std::set<std::string>						availPartition;		//avail partitions relative to current streamer
	bool										bAllPartitionAvail;	//true if all partition can be used for current streamer

	bool										bMaintainEnable;	//used for maintenance

	ResourceStreamerAttr()
	{
		bReplicaStatus					= false;
		lastReplicaUpdate				= 0;
		maxBw							= 0;
		usedBw							= 0;
		maxSessCount					= 0;
		usedSessCount					= 0;
		streamServicePrx				= NULL;
		
		statisticsRemoteSessCount		= 0;
		statisticsTotalSessCount			= 0;
		statisticsUsedSessCount			= 0;		
		statisticsFailedSessCount		= 0;

		bAllPartitionAvail				= false;
		penalty							= 0;
		bMaintainEnable					= true;
		
		sourceIP						= 0;

	}
};

typedef std::map< std::string , ResourceStreamerAttr > ResourceStreamerAttrMap;
//               streamer netId    streamerAttr



typedef std::map< std::string , ResourceStreamerAttrMap> SOPS;
//                sopname       streamerAttrMap

struct ResourceStreamerKey 
{
	std::string								netId;				//net id of the streamer
	bool									bAvail;				//true if ( streamServicePrx != NULL && bMaintainEnable && bReplicaStatus )
	int										streamerCost;		//streamer cost 
	int										importChannelCost;	//cost relative import channel
};

#define MAX_WEIGHT	10000

struct ResourceStreamerAttrEx : public ResourceStreamerAttr
{
	int					streamerWeight;
	
	ResourceStreamerAttrEx();
	ResourceStreamerAttrEx( const ResourceStreamerAttrEx& a );
	ResourceStreamerAttrEx( const ResourceStreamerAttr& a);
	ResourceStreamerAttrEx& operator=( const ResourceStreamerAttrEx& a );
	ResourceStreamerAttrEx& operator=( const ResourceStreamerAttr& a );

};
typedef std::vector< ResourceStreamerAttrEx > ResourceStreamerAttrExS;


enum ContentCachingStatus
{
	STATUS_CONTENT_CACHING_NULL				= 0,
	STATUS_CONTENT_CACHING_NONE				= 1,
	STATUS_CONTENT_CACHING_IMPORTING		= 2,
	STATUS_CONTENT_CACHING_LOCAL			= 3
};


/*Element information*/
struct ElementInfo 
{
	std::string					pid;
	std::string					paid;	//refer to pid && paid 

	std::string					name;
	int64						cueIn;
	int64						cueOut;
	int64						flags;	//this can be restriction flags
	std::string                 range;  //for StreamResources
	bool						primaryElement;//belonging to primary asset

	std::vector<std::string>	urls;
	VolumeNameAttrS				volumeAttr; // which volumes the content belong to

	std::map<VolumeNameAttr,ContentCachingStatus> mContentStatus;
	std::map<VolumeNameAttr , std::string>	mImportingNode;	

	ElementInfo();

	void	updateContentStatus( const std::map<std::string,std::string>& contentAttributes );

	ContentCachingStatus getStatus( const std::string& volumeNetId ) const;

	std::string	getImportingNode( const std::string& volumeNetId ) const;

	bool	isImportingInVolume( const std::string& volumeNetId ) const;
	
	std::string fullContentName( const std::string& volumeName ) const;	
};

typedef std::vector<ElementInfo> ElementInfoS;
/*
SelectionEnv
*/

typedef std::map<std::string,com::izq::am::facade::servicesForIce::AEInfo3 > PID2ELEMAP;

class SelectionEnv
{
public:
	SelectionEnv(void)
		:mLogger(NULL),
		mEventLogger(NULL),
		lamProxy(NULL),
		contentLibPrxoy(NULL),
		mbContentLibMode(false),
		mbContentTestMode(false),
		mbGBMode(false),
		mNasUrlPrefix(""),
		mbStreamerReplicaTestMode(false),
		mbIcReplicaTestMode(false),
		mRemoteAssetStack(*this),
		mbEnableAssetStack(false),
		mAssetStackAdjustWeight(9995),
		mAssetStackStartMode(0)
	{
	}
	virtual ~SelectionEnv(void)
	{
	}
	
	inline ZQ::common::Log*		getMainLogger( )
	{
		return mLogger;
	}

	inline ZQ::common::Log*		getEventLogger()
	{
		return mEventLogger;
	}

	inline com::izq::am::facade::servicesForIce::LAMFacadePrx getLamProxy( )
	{
		return lamProxy;
	}

	inline TianShanIce::Repository::ContentLibPrx getContentLibProxy( )
	{
		return contentLibPrxoy;
	}

	inline bool					contentInTestMode( ) const
	{
		return	mbContentTestMode;
	}
	inline bool					contentLibMode( ) const
	{
		return mbContentLibMode;
	}
	inline bool					GBMode( ) const
	{
		return mbGBMode;
	}
	inline std::string			getNasUrlPrefix( ) const
	{
		return mNasUrlPrefix;
	}
	const ::com::izq::am::facade::servicesForIce::AEInfo3Collection&			getTestModeContent( )	const
	{
		return mTestModeContent;
	}

	inline bool					streamerReplicaInTestMode( ) const
	{
		return mbStreamerReplicaTestMode;
	}
	inline bool					importChannelReplicaInTestMode( ) const
	{
		return mbIcReplicaTestMode;
	}
	inline int32				replicaUpdateInterval( ) const
	{
		return mReplicaUpdateInterval;
	}
	inline int32				maxPenaltyValue( ) const
	{
		return mMaxPenaltyValue;
	}
	RemoteAssetStack&			getAssetStackManager( )
	{
		return mRemoteAssetStack;
	}

public:
	
	ZQ::common::Log*												mLogger;
	ZQ::common::Log*												mEventLogger;
	com::izq::am::facade::servicesForIce::LAMFacadePrx				lamProxy;
	TianShanIce::Repository::ContentLibPrx						    contentLibPrxoy;
	bool															mbContentLibMode;

	bool															mbContentTestMode;
	bool															mbGBMode;
	std::string														mNasUrlPrefix;
	com::izq::am::facade::servicesForIce::AEInfo3Collection			mTestModeContent;
	PID2ELEMAP														mPid2Elements;

	bool															mbPublishLog;

	bool															mbStreamerReplicaTestMode;
	bool															mbIcReplicaTestMode;

	int32															mReplicaUpdateInterval; //in milliseconds
	int32															mMaxPenaltyValue;
	
	
	bool															mbEnableAssetStack;
	int32															mAssetStackTimeShiftWindow;
	int32															mAssetStackAdjustWeight;
	int32															mAssetStackStartMode;
	RemoteAssetStack												mRemoteAssetStack;
};



//////////////////////////////////////////////////////////////////////////
///NgodResourceManager
class NgodResourceManager : public ZQ::common::NativeThread , public ZQ::common::Mutex
{
public:
	NgodResourceManager( SelectionEnv& env );
	virtual ~NgodResourceManager(void);

	//////////////////////////////////////////////////////////////////////////
	//These functions in this section can only be invoked when initialize service
	//DO NOT invoke it on the fly
	bool									initResourceManager( bool bNeedWarmup = true );
	void									updateSopData( const SOPS& sops );
	void									updateVolumesData( const ResourceVolumeAttrMap& volumes );
	void									updateImportChannelData( const ResourceImportChannelAttrMap& ics );

	bool									getSopData( SOPS& sops , std::string& measuredSince ) const;
	bool									getImportChannelData( ResourceImportChannelAttrMap& ics ) const;

	///for maintenance 
	///groupName , for NGOD this is the sop name , for GBss this is dummy group name
	void									maintainEnable( const std::string& groupName , const std::string& streamerNetId , bool enable );

	//////////////////////////////////////////////////////////////////////////
	void									updateReplica(const TianShanIce::Replicas& reps);

	///stop resource manager
	void									stop( );

public:

	// apply penalty value to a streamer
	// if the penalty value of a streamer is greater than 0 , the streamer is disabled and can't be selected to create a stream on
	void									applyPenalty( const std::string& streamerNetId , int32 penaltyValue );

	// update streamer replica status
	void									updateStreamerReplica( const std::string& streamerNetId, bool status );

	// update import channel replica status
	void									updateImportChannelReplica( const std::string& importChannelName , int64 bwUsed , int64 bwTotal ,int32 runningSess );

public:

	/// get available volume through mask
	/// for ComCast NGOD identifier is sopname, for GBss identifier is sourceIp
	/// use hasAvailVolumes to detect if there is available volume of that identifier before used getAvailVolumes
	bool									hasAvailVolumes( const std::string& identifier ) const;
	const ResourceVolumeAttrMap&			getAvailVolumes( const std::string& identifier ) const;	
	bool									getAvailVolumes( const std::string& identifier , ResourceVolumeAttrMap& volumes ) const;

	bool									hasSupportNasVolumes( const std::string& identfier ) const;
	const ResourceVolumeAttrMap&			getSupportNasVolumes( const std::string& identifier ) const;
	bool									getSupportNasVolumes( const std::string& identifier,ResourceVolumeAttrMap& volumes ) const;

	bool									getVolumeAttr( const std::string& volumeName , ResourceVolumeAttr& attr ) const;
	
	struct StreamerResourcePara 
	{
		std::string				identifier;		//sopname or sourceIp
		std::set<std::string>	volumeNetIds;	//available volume netIds for selecting streamer, this can be empty if used to release resource
		int32					requestBW;
		bool					bNeedImportChannel;

		//FOR LOG message
		std::string				method;
		std::string				cseq;
		std::string				sessionId;

		bool					bRestore;//set true if in restore stage

		StreamerResourcePara()
		{
			bNeedImportChannel	= false;
			requestBW			= 0;
			bRestore			= false;
		}
	};

	void									resetStatisticsCounter();

	///confirm resource would like to update streamer statistics information
	void									confirmResource( const StreamerResourcePara& para, const std::string& streamerNetId ,bool bLocalPlaylist);

	bool									getStreamersFromSopAndVolume( const StreamerResourcePara& para , ResourceStreamerAttrExS& rets );
	
	bool									allocateResource( const StreamerResourcePara& para, const std::string& streamerNetId );
	
	void									releaseResource( const StreamerResourcePara& para , const std::string& streamerNetId , bool bFailed = false , int32 penaltyValue = 0 );

	
	bool									isStreamerReplicaAvail( const std::string& streamerNetId ) const;

protected:
	
	void									processImportChannelReplica( const TianShanIce::Replica& rep );
	void									processStreamerReplica( const TianShanIce::Replica& rep );

	int32									evaluateStreamer( const StreamerResourcePara& para , const ResourceStreamerAttr& streamerAttr );
	int32									evaludateImportChannel( const StreamerResourcePara& para , const ResourceStreamerAttr& streamerAttr );
	bool									checkStreamerConfiguration( const StreamerResourcePara& para , const ResourceStreamerAttr& streamerAttr  );

	
	////////////////////////////////////////////////////////////////////////////////////////////////
	/**********************************************************************************************/
	typedef std::map< std::string , ResourceStreamerAttrMap::iterator > StreamerIterMap;
	
	virtual StreamerIterMap					findByIdentifier( const std::string& identifier ) ;

	/**********************************************************************************************/
	////////////////////////////////////////////////////////////////////////////////////////////////

private:

	int									run( );

	bool								mbCheckThreadQuit;
	ZQ::common::Cond					mCond;

protected:

	SOPS								mSops;
	std::string							mStrMeasuredSince;

	ResourceImportChannelAttrMap		mImportChannles;

	ResourceVolumeAttrMap				mVolumes;

	typedef std::map< std::string, ResourceVolumeAttrMap > VolumeMapMap;
	VolumeMapMap						mAvailVolumesBySop;
	VolumeMapMap						mNasVolumes;

	typedef std::vector< ResourceStreamerAttrMap::iterator > StreamerIteratorSet;
	typedef std::map< std::string , StreamerIteratorSet > StreamerIteratorSetMap;

	StreamerIteratorSetMap				mStreamerSymLinks;

	SelectionEnv&						mEnv;

};

bool		parseVolumeName( const std::string& volumeName , VolumeNameAttr& attr);

#endif//_ssm_ngod_resource_manager_header_file_h__

