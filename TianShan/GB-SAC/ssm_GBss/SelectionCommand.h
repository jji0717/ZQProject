
#ifndef ssm_ngod_setup_command_header_file_h__
#define ssm_ngod_setup_command_header_file_h__

#include <set>
#include <list>
#include <vector>
#include "StreamSmithAdmin.h"
#include "LAMFacade.h"
#include "SelectionResourceManager.h"

#define PENALTY_ENABLE_MASK_PLAY	1
#define PENALTY_ENABLE_MASK_PAUSE	2
#define PENALTY_ENABLE_MASK_GETPAR	4

const int32 errorcodeOK						= 200;
const int32 errorcodeBadRequest				= 400;
const int32 errorcodeUnauthorized			= 401;
const int32 errorcodeObjNotFound			= 404;
const int32 errorcodeNotAccept				= 406;
const int32 errocodeRequestTimeout			= 408;
const int32 errorcodeBadParameter			= 451;
const int32 errorcodeNotEnoughBandwidth		= 453;
const int32 errorcodeSessNotFound			= 454;
const int32 errorcodeInvalidState			= 455;
const int32 errorcodeInvalidRange			= 457;
const int32 errorcodeInternalError			= 500;
const int32 errorcodeNotImplement			= 501;
const int32 errorcodeServiceUnavail			= 503;
const int32 errorcodeOptionNotSupport		= 551;
const uint errorcodeTrickRestriction 		= 403;

const int32 errorcodeNoResponse				= 770;
const int32 errorcodeAssetNotFound			= 771;
const int32 errorcodeSopNotAvail			= 772;
const int32 errorcodeUnknownSopGroup		= 773;
const int32 errorcodeUnkownSopnames			= 774;
const int32 errorcodeNotEnoughVolBandwidth	= 775;
const int32 errorcodeNotEnoughNetworkBandwidth=776;
const int32 errorcodeInvalidRequest			= 777;

const char* errorCodeTransformer( const int32& error );

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
#define OUT
#define INOUT

struct SelectIntentionParam 
{	
	IN	std::string				identifier;			//sop name or sop group name
	IN	std::string				groupName;			//group name, for NGOD this should be equal to sop name.For GBss , this should be the dummy group name
	IN	std::string				volume;				//request volume
	IN	int64					requestBW;			//request bandwidth counted in byte
	
	struct PlaylistItemInfo 
	{
		IN		std::string			pid;
		IN		std::string			paid;
		IN		std::string			sid;
		
		IN		int32               ordinal;
		
		INOUT   int32               bandWidth;
		INOUT	int64				cuein;				//if there are more than 1 elements in the asset, what the cueout value should be ?
		INOUT	int64				cueout;
		IN		int64				restrictionFlag;	//restriction flag used for ff/fw forbidden thing
		IN      std::string         range;              //for StreamResources

		IN		bool				primaryAsset;
		
		PlaylistItemInfo()
		{
			ordinal				= 0;
			bandWidth           = 0;
			cuein				= 0;
			cueout				= 0;
			restrictionFlag		= 0;
			primaryAsset		= true;
		}
	};

	///
	int32				assetValue;

	std::vector<PlaylistItemInfo> playlist;	
	std::string toString() const;
	
};

class SelectionIntention 
{
public:
	SelectionIntention( SelectionEnv& env , const std::string& sessId , const std::string& cseq , const std::string& method )
		:mSessId(sessId),
		mCseq(cseq),
		mMethod(method)
	{

	}
	virtual ~SelectionIntention( )
	{

	}

public:

	SelectIntentionParam&			getParameter( ) 
	{
		return mPara;
	}

	inline const std::string&		getSessId( ) const
	{
		return mSessId;
	}

	inline const std::string&		getCseq( ) const
	{
		return mCseq;
	}

	inline const std::string&		getMethod( ) const
	{
		return mMethod;
	}
	
	void							setSessId( const std::string& sessId )
	{
		mSessId = sessId;
	}
	void							setCseq( const std::string& cseq )
	{
		mCseq = cseq;
	}
	void							setMethod( const std::string& method  )
	{
		mMethod = method;
	}
	int32							getAssetValue( ) const
	{
		return mPara.assetValue;
	}

private:

	SelectIntentionParam			mPara;	
	std::string						mSessId;
	std::string						mCseq;
	std::string						mMethod;
};


struct CandidateStreamer 
{
	std::string				volumeName;				// name of volume relative to current streamer
	int						volumePriority;			// volume priority
	std::string				streamerNetId;			// streamer net Id
	std::string				importChannelName;		// if a streamer does not have any import channel associated with , ignore it if playlist is not local
	bool					bLocalPlaylist;			// if all playlist items can be streamed out from local storage, the playlist is treated as local playlist

	int						weight;

	CandidateStreamer()
	{
		weight			= 0;						//default to disable the streamer
		bLocalPlaylist	= false;	
	}
};


typedef std::list<CandidateStreamer> CandidateStreamerS;



struct  VolumeAttrEx : public ResourceVolumeAttr 
{
	mutable bool					bLocalPlaylist;
	mutable ContentCachingStatus	mContentStatus;//content ÔÚvolumeÉÏµÄ×´Ì¬
	mutable std::string				mImportingNodeNetId;
	VolumeAttrEx()
	{
		bLocalPlaylist	= false;
		mContentStatus	= STATUS_CONTENT_CACHING_NULL;
	}
};
struct PriorityVolumeCmp
{
	bool operator()( const VolumeAttrEx& a, const VolumeAttrEx& b ) const
	{
		if( a.level < b.level )
		{
			return true;
		}
		else if ( a.level == b.level )
		{
			return a.netId < b.netId;
		}
		else
		{
			return false;
		}
	}
};

typedef std::set< VolumeAttrEx, PriorityVolumeCmp > PriorityVolumeAttrSet;


class PriorityVolumeSet
{
public:
	PriorityVolumeSet( SelectionEnv& env , SelectionIntention& intention ,NgodResourceManager&	resManager);
	virtual ~PriorityVolumeSet( );

public:

	bool				isValid( ) const
	{
		return (mFirstClassVolumes.size() >0 || mTouristClassVolumes.size() > 0 );
	}
	
	void				addFirstClassVolumes( const ResourceVolumeAttrMap& volumeAttrMap , const VolumeNameAttrS& candidateVolNames );

	void				addTouristClassVolumes( const ResourceVolumeAttrMap& volumeAttrMap );

	///get volumes at the same priority level
	/// return false if no more volume available
	bool				findNextVolumeset( PriorityVolumeAttrSet& volumeset , bool& bLocalPlaylist ) ;

	///this function reset the iterator to begin, so that you can use findNextVolumeset to get the result you want
	///If there is no volumes available, false if returned
	bool				findFirstVolumeset( ) ;

	void				adjustFirstClassVolumes( const std::string& requestedVolume );

	ContentCachingStatus	getContentCachingStatus( const std::string& volumeNetId ) const;

	std::string				getImportingNodeId( const std::string& volumeNetId ) const;

	int					getVolumeCacheLevel( const std::string& volumeNetId ) const;

	std::string			getCandidateVolumename( ) const;

	class iterator
	{
	public:		
		iterator( PriorityVolumeSet& volumes );
		virtual ~iterator();
		void operator++(int);
		bool isEnd( ) const;
		PriorityVolumeAttrSet::iterator& operator->() 
		{
			return mIt;
		}
	private:
		PriorityVolumeSet& mVolumes;
		PriorityVolumeAttrSet::iterator mIt;
		bool mbInFirstClass;
	};

	PriorityVolumeAttrSet::iterator begin() ;
	PriorityVolumeAttrSet::iterator end() ;

private:
	
	///first class volumes		--> these volumes are specified by LAM and must be used firstly
	PriorityVolumeAttrSet					mFirstClassVolumes;

	///tourist class volumes	--> these volumes are candidates if first class can't satisfy the request
	PriorityVolumeAttrSet					mTouristClassVolumes;

	PriorityVolumeAttrSet::const_iterator	mItVolume;
	int										mCurVolumeCacheLevel;
	bool									mbInFirstClass;

	SelectionEnv&							mEnv;

	SelectionIntention&						mIntention;

	NgodResourceManager&					mResManager;
};

class SelectionCommand 
{
public:
	SelectionCommand( SelectionEnv& env , SelectionIntention& intention )
	:mEnv(env),
	mMethod( intention.getMethod() ),
	mSeq(intention.getCseq()),
	mSessId(intention.getSessId())
	{
	}
	virtual ~SelectionCommand(void)
	{

	}

protected:

	inline const std::string&	getSessId() const
	{
		return mSessId;
	}
	inline const std::string&	getCseq() const
	{
		return mSeq;
	}
	inline const std::string&	getMethod() const
	{
		return mMethod;
	}

protected:
	SelectionEnv&		mEnv;
	std::string			mMethod;
	std::string			mSeq;
	std::string			mSessId;
};

class CandidateVolumesBuilder : public SelectionCommand
{
public:
	CandidateVolumesBuilder( SelectionEnv& env ,NgodResourceManager& resManager, SelectionIntention& intention );
	virtual ~CandidateVolumesBuilder( );

	void						attachIntention( SelectionIntention& intention  );

	bool						build( const SelectIntentionParam& paras );

	const ElementInfoS&			getElements( ) const;

	int64						getMaxBandwidth( ) const
	{
		return mMaxBandwidth;
	}

	PriorityVolumeSet&			getVolumeSet( )
	{
		return mSortedVolumes;
	}
	
	int32						getLastError( ) const
	{
		return mLastErrorCode;
	}
	const std::string&			getErrorMsg( ) const
	{
		return mErrorDescription;
	}

	ContentCachingStatus		getPlaylistStatus( const std::string& volumeNetId ) const;
	
	std::string					getImportingNodeId( const std::string& volumeNetId ) const;

protected:

	bool						getElementInfo( const SelectIntentionParam& paras  ) ;	

	com::izq::am::facade::servicesForIce::AEInfo3Collection			getElementInfoFromContentLib(  const std::string& pid , const std::string& paid , const std::string& sid  );
	com::izq::am::facade::servicesForIce::AEInfo3Collection			getElementInfoFromLAM( const std::string& pid , const std::string& paid , const std::string& sid );
	com::izq::am::facade::servicesForIce::AEInfo3Collection			getElementInfoInTestMode(  const SelectIntentionParam& paras );
	com::izq::am::facade::servicesForIce::AEInfo3Collection			getElementInfoInTestMode(  const std::string& pid , const std::string& paid , const std::string& sid  );

	void						setLastError( int32 errorCode , const char* fmt , ... );


	void						checkPlaylistCachingStatus( );

	const ElementInfo&			findPrimaryElement( ) const;

private:


	VolumeNameAttrS				parseVolumeNameString( const std::string& volumeString ) const;

	VolumeNameAttrS				parseVolumeNameString( const std::vector<std::string>& volumesString ) const;	

	void						intersectVolumes( const VolumeNameAttrS& volumesA, const VolumeNameAttrS& volumesB, VolumeNameAttrS& result );

private:
	NgodResourceManager&		mResManager;
	CandidateStreamerS			mStreamerCandidates;
	ElementInfoS				mElements;	
	int64						mMaxBandwidth;
	PriorityVolumeSet			mSortedVolumes;
	int32						mLastErrorCode;
	std::string					mErrorDescription;
};

class StreamerSelection : public SelectionCommand
{
public:
	StreamerSelection( SelectionEnv& env , NgodResourceManager& resManager, SelectionIntention& intention );
	virtual ~StreamerSelection( );

public:

	///find first streamer is used to build available volumes and necessary resource
	bool										findFirstStreamer( );

	///find next streamer is used find the most suitable streamer 	
	bool										findNextStreamer( bool bSkipToNextVolume = false ,bool addPenalty = false );

	void										commit();

	///get elements information from StreamerSelection
	const ElementInfoS&							getElements() const;

	///get adjusted bandwidth, you should take this bandwidth as final bandwidth which can be passed to stream service
	int64										getAdjustedBandwidth( ) const;

	///get selected volume information
	const VolumeAttrEx&							getSelectedVolumeAttr( ) const;
	
	///get selected streamer information
	const ResourceStreamerAttrEx&				getSelectedStreamerAttr( ) const;

	//////////////////////////////////////////////////////////////////////////
	///get result information
	const std::string							getSelectedImportChannelName( ) const;
	const std::string&							getSelectedStreamerNetId( ) const;
	const std::string&							getSelectedStreamerEndpoint( ) const;
	const std::string&							getSelectedVolumeName( ) const;
	//////////////////////////////////////////////////////////////////////////

	int32										getLastError( ) const
	{
		return mLastErrorCode;
	}
	const std::string&							getErrorMsg( ) const
	{
		return mErrorDescription;
	}

public:

	///detect current selected streamer is valid or not
	bool										isSelectedStreamerValid( ) const;
	
	TianShanIce::Streamer::StreamSmithAdminPrx	getStreamerProxy( ) const;


protected:

	bool										getCandidateStreamers( bool& bLocalPlaylist );

	bool										commitStreamerForResource( bool bLocalPlaylist );

	void										setLastError( int32 errorCode , const char* fmt , ... );
	
	void										releaseResource( bool bAddPenalty = false , bool bSkipToNextVolume = false );

	void										clearStreamerInCurrentVolume( );

	void										adjustWeight();

	void										adjustWeightByAssetValue( );

	void										adjustWeightByImportingHost( );

	int											caclNewWeight( int weight , bool bUpgrade );
private:

	CandidateVolumesBuilder						mVolumesBuilder;

	NgodResourceManager&						mResManager;

	SelectionIntention&							mIntention;

private:
	ResourceStreamerAttrExS						mCandidateStreamers;
	ResourceStreamerAttrExS::iterator			mItStreamer;
	
	VolumeAttrEx								mSelectedVolumeAttr;
	
	bool										mbLocalPlaylist;

	NgodResourceManager::StreamerResourcePara	mResourceRequest;

	int32										mLastErrorCode;
	std::string									mErrorDescription;	

	bool										mbCommitted;
	bool										mbHasValidStreamerSelected;//indicate that there is at least one streamer was successfully selected , what ever we use it or not
};

#endif//ssm_ngod_setup_command_header_file_h__

