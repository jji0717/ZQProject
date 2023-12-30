
#include <ZQ_common_conf.h>
#include <math.h>
#include <functional>
#include <TianShanIceHelper.h>
#include <strHelper.h>
#include <TianShanDefines.h>
#include "RtspHeaderDefines.h"
#include "SelectionCommand.h"

#ifdef ZQ_OS_LINUX
#define _vsnprintf		vsnprintf
#endif//ZQ_OS_LINUX


#define MLOG		(*mEnv.getMainLogger())

#if defined ZQ_OS_MSWIN
	#define	REQFMT(x,y) 	"%s/%s/%s/%06X/[%8s]\t"##y,getSessId().c_str(),getCseq().c_str(),getMethod().c_str(),GetCurrentThreadId(),#x
#else
	#define	REQFMT(x,y) 	"%s/%s/%s/%06X/[%8s]\t"y,getSessId().c_str(),getCseq().c_str(),getMethod().c_str(),pthread_self(),#x
#endif	

#if defined ZQ_OS_MSWIN
	#define	COMMFMT(x,y) 	"%s/%s/%s/%06X/[%8s]\t"##y,mIntention.getSessId().c_str(),mIntention.getCseq().c_str(),mIntention.getMethod().c_str(),GetCurrentThreadId(),#x
#else
	#define	COMMFMT(x,y) 	"%s/%s/%s/%06X/[%8s]\t"y,mIntention.getSessId().c_str(),mIntention.getCseq().c_str(),mIntention.getMethod().c_str(),pthread_self(),#x
#endif	

const char* errorCodeTransformer( const int32& error )
{
	switch( error )
	{
	case errorcodeOK:						return RESPONSE_OK;
	case errorcodeBadRequest:				return RESPONSE_BAD_REQUEST;
	case errorcodeUnauthorized:				return RESPONSE_UNAUTHORIZED;
	case errorcodeObjNotFound:				return RESPONSE_Object_NOTFOUND;
	case errorcodeNotAccept:				return REPSONSE_REQUEST_NOT_ACCEPTABLE;
	case errocodeRequestTimeout:			return RESPONSE_REQUEST_TIMEOUT;
	case errorcodeBadParameter:				return RESPONSE_BAD_PARAMETER;
	case errorcodeNotEnoughBandwidth:		return RESPONSE_NOT_ENOUGH_BW;
	case errorcodeSessNotFound:				return RESPONSE_SESSION_NOTFOUND;
	case errorcodeInvalidRange:				return RESPONSE_INVALID_RANGE;
	case errorcodeInternalError:			return RESPONSE_INTERNAL_ERROR;
	case errorcodeNotImplement:				return RESPONSE_NOT_IMPLEMENT;
	case errorcodeInvalidState:				return RESPONSE_INVALID_STATE;
	case errorcodeTrickRestriction:			return RESPONSE_TRICK_RESTRICTION;
	case errorcodeServiceUnavail:			return RESPONSE_SERVICE_UNAVAILABLE;
	case errorcodeNoResponse:				return RESPONSE_SSF_NORESPONSE;
	case errorcodeAssetNotFound:			return RESPONSE_SSF_ASSET_NOT_FOUND;
	case errorcodeSopNotAvail:				return RESPONSE_SSF_SOP_NOT_AVAILABLE;
	case errorcodeUnknownSopGroup:			return RESPONSE_SSF_UNKNOWN_SOPGROUP;
	case errorcodeUnkownSopnames:			return RESPONSE_SSF_UKNOWN_SOPNAMES;
	case errorcodeNotEnoughVolBandwidth:	return RESPONSE_SSF_NO_VOLUMEBANDWIDTH;
	case errorcodeNotEnoughNetworkBandwidth:return RESPONSE_SSF_NO_NETWORKBANDWIDTH;
	case errorcodeInvalidRequest:			return RESPONSE_SSF_INVALID_REQUEST;
#ifdef RESPONSE_OPTIONS_NOT_SUPPORT
	case errorcodeOptionNotSupport:			return RESPONSE_OPTIONS_NOT_SUPPORT;
#endif //RESPONSE_OPTIONS_NOT_SUPPORT
	default:
		return RESPONSE_INTERNAL_ERROR;
	}
}

std::string SelectIntentionParam::toString() const
{
	std::ostringstream oss;
	oss<<"groupname["<<groupName<<"] volume["<<volume<<"] ReqBW["<<requestBW<<"] ";
	std::vector<PlaylistItemInfo>::const_iterator itAsset = playlist.begin();
	for( ; itAsset != playlist.end() ; itAsset++ )
	{
		oss<<"asset["<<itAsset->pid <<"/"<<itAsset->paid<<"] ";
	}
	return oss.str();
}

int32 evaluateString( const std::string& str)
{
	int32 value = 0 ;

	if(str.empty() )	return 0;

	const char* p = str.c_str();
	while( *p )
	{
		value += *((unsigned char*)p);
		p++;
	}
	return ( value % 32767 );
}
using namespace com::izq::am::facade::servicesForIce;

//////////////////////////////////////////////////////////////////////////
//CandidateVolumesBuilder
CandidateVolumesBuilder::CandidateVolumesBuilder( SelectionEnv& env ,NgodResourceManager& resManager, SelectionIntention& intention)
:SelectionCommand( env , intention ),
mResManager( resManager ),
mSortedVolumes( env,intention ,resManager ),
mLastErrorCode( errorcodeOK )
{
	mMaxBandwidth = 0;
}

CandidateVolumesBuilder::~CandidateVolumesBuilder( )
{
}

const ElementInfoS& CandidateVolumesBuilder::getElements( ) const
{
	return mElements;
}

AEInfo3Collection CandidateVolumesBuilder::getElementInfoInTestMode(  const SelectIntentionParam& intention)
{
	return mEnv.getTestModeContent();	
}

com::izq::am::facade::servicesForIce::AEInfo3Collection	 CandidateVolumesBuilder::getElementInfoInTestMode(  const std::string& pid , const std::string& paid , const std::string& sid  )
{
	AEInfo3Collection infos ;	
	PID2ELEMAP::const_iterator it = mEnv.mPid2Elements.find( pid );
	if( it != mEnv.mPid2Elements.end() )
	{
		com::izq::am::facade::servicesForIce::AEInfo3 info = it->second;
		info.name = paid + pid;
		infos.push_back(info);
	}
	if(infos.empty())
		return mEnv.getTestModeContent();
	else
		return infos;
}

void CandidateVolumesBuilder::setLastError( int32 errorCode , const char* fmt , ... )
{
	char szLocalBuffer[1024];
	va_list args;
	va_start(args, fmt);
	int nCount = _vsnprintf( szLocalBuffer, sizeof(szLocalBuffer)-1, fmt, args );
	va_end(args);
	if(nCount == -1)
	{
		szLocalBuffer[ sizeof(szLocalBuffer) - 1 ] = 0;
	}
	else
	{
		szLocalBuffer[nCount] = '\0';
	}
	mLastErrorCode		= errorCode;
	mErrorDescription	= szLocalBuffer;
	MLOG(ZQ::common::Log::L_ERROR, REQFMT(CandidateVolumesBuilder,"%s"), szLocalBuffer );
}

const ElementInfo& CandidateVolumesBuilder::findPrimaryElement( ) const
{
	assert( mElements.size() > 0 );
	ElementInfoS::const_iterator it = mElements.begin();
	for( ; it != mElements.end() ; it++ )
	{
		if( it->primaryElement)
			return *it;
	}
	MLOG(ZQ::common::Log::L_WARNING,REQFMT(CandidateVolumesBuilder,"no primary element is found ,use the first item instead"));
	return mElements[0];
}
void CandidateVolumesBuilder::checkPlaylistCachingStatus( )
{
	///TODO: find out the primary asset/element
	const ElementInfo& primaryElement = findPrimaryElement();
	//take this as the while playlist content status
	PriorityVolumeSet::iterator it(mSortedVolumes);	

	//adjust if it is local playlist
	for( ; !it.isEnd() ; it ++ )
	{
		ContentCachingStatus status = primaryElement.getStatus( it->netId );
		
		it->mContentStatus = status;
		it->mImportingNodeNetId = primaryElement.getImportingNode( it->netId );

		//check all local asset and find if the asset is in InService status
		switch( status )
		{
		case STATUS_CONTENT_CACHING_NULL:
			{
				if( it->bLocalPlaylist )
				{
					it->mContentStatus = STATUS_CONTENT_CACHING_LOCAL;
				}
				else
				{
					it->mContentStatus = STATUS_CONTENT_CACHING_NONE;
				}
			}
			break;;
		case STATUS_CONTENT_CACHING_NONE:		
			{
				it->bLocalPlaylist = false;
			}
			break;
		case STATUS_CONTENT_CACHING_IMPORTING:
			{
				if( !primaryElement.isImportingInVolume(it->netId) )
				{
					it->bLocalPlaylist = true;
					it->mContentStatus = STATUS_CONTENT_CACHING_LOCAL;//set it to inservice status
				}
				else
				{
					
					it->bLocalPlaylist = false;
				}
			}
			break;
		case STATUS_CONTENT_CACHING_LOCAL:
			{
				it->bLocalPlaylist = true;
			}
			break;
		default:
			{
				assert(false);
			}
			break;
		}	
	}	
}

ContentCachingStatus CandidateVolumesBuilder::getPlaylistStatus( const std::string& volumeNetId ) const
{
	return mSortedVolumes.getContentCachingStatus( volumeNetId );
}
std::string CandidateVolumesBuilder::getImportingNodeId( const std::string& volumeNetId ) const
{
	return mSortedVolumes.getImportingNodeId( volumeNetId );
}


com::izq::am::facade::servicesForIce::AEInfo3Collection
CandidateVolumesBuilder::getElementInfoFromContentLib( const std::string& pid , const std::string& paid , const std::string& sid )
{
	AEInfo3Collection aeInfos;
	TianShanIce::Repository::MetaObjectInfos moInfos;

	::TianShanIce::StrValues expectedMetaDataNames;
	expectedMetaDataNames.push_back(SYS_PROP(ScheduledProvisonStart));
	expectedMetaDataNames.push_back(SYS_PROP(ScheduledProvisonEnd));
	expectedMetaDataNames.push_back(SYS_PROP(MaxProvisonBitRate));
	moInfos = mEnv.getContentLibProxy()->locateContentByPIDAndPAID( "", "", pid, paid, expectedMetaDataNames );

	//change MetaObjectInfos to AEInfo3Collection
	for( TianShanIce::Repository::MetaObjectInfos::iterator itMetaObjectInfo = moInfos.begin() ; itMetaObjectInfo != moInfos.end() ; itMetaObjectInfo ++ )
	{	
		std::string name = itMetaObjectInfo->id.substr(0, itMetaObjectInfo->id.find("@"));
		std::string netId = itMetaObjectInfo->id.substr(itMetaObjectInfo->id.find("@") + 1, itMetaObjectInfo->id.find("$"));
		std::string volume = itMetaObjectInfo->id.substr(itMetaObjectInfo->id.find("$") + 1, itMetaObjectInfo->id.size());
		bool bFound = false;
		AEInfo3Collection::iterator itCurrent;
		for( AEInfo3Collection::iterator itAeInfo = aeInfos.begin() ; itAeInfo != aeInfos.end() ; itAeInfo ++ )
		{
			if( itAeInfo->name == name)
			{
				bFound = true;
				itCurrent = itAeInfo;
				break;
			}
		}	
		if(!bFound)
		{
			AEInfo3 aeInfo;
			aeInfo.name = name;
			aeInfo.bandWidth = itMetaObjectInfo->metaDatas.find(SYS_PROP(MaxProvisonBitRate)) != itMetaObjectInfo->metaDatas.end() ? atoi(itMetaObjectInfo->metaDatas[SYS_PROP(MaxProvisonBitRate)].value.c_str()) : 3750000;
			aeInfo.cueIn = itMetaObjectInfo->metaDatas.find(SYS_PROP(ScheduledProvisonStart)) != itMetaObjectInfo->metaDatas.end() ? atoi(itMetaObjectInfo->metaDatas[SYS_PROP(ScheduledProvisonStart)].value.c_str()) : 0;
			aeInfo.cueOut = itMetaObjectInfo->metaDatas.find(SYS_PROP(ScheduledProvisonEnd)) != itMetaObjectInfo->metaDatas.end() ? atoi(itMetaObjectInfo->metaDatas[SYS_PROP(ScheduledProvisonEnd)].value.c_str()) : 0;
			aeInfo.volumeList.push_back(netId + volume);
			if(itMetaObjectInfo->metaDatas.find(SYS_PROP(NasUrls)) != itMetaObjectInfo->metaDatas.end())
				aeInfo.nasUrls.push_back(itMetaObjectInfo->metaDatas[SYS_PROP(NasUrls)].value);
			aeInfos.push_back(aeInfo);
		}
		else
		{
			itCurrent->volumeList.push_back(netId + volume);
			if(itMetaObjectInfo->metaDatas.find(SYS_PROP(NasUrls)) != itMetaObjectInfo->metaDatas.end())
				itCurrent->nasUrls.push_back(itMetaObjectInfo->metaDatas[SYS_PROP(NasUrls)].value);
		}
	}	
	return aeInfos;
}
com::izq::am::facade::servicesForIce::AEInfo3Collection 
CandidateVolumesBuilder::getElementInfoFromLAM( const std::string& pid , const std::string& paid , const std::string& sid )
{
	return mEnv.getLamProxy()->getAEListByPIdPAIdSId( pid , paid , sid );
}

bool CandidateVolumesBuilder::getElementInfo( const SelectIntentionParam& paras )
{
	//get available volumes of this groupName
	MLOG(ZQ::common::Log::L_DEBUG,REQFMT(CandidateVolumesBuilder,"getElementInfo(): %s"), paras.toString().c_str());
	
	ResourceVolumeAttrMap availVolumes;
	if( !mResManager.getAvailVolumes( paras.groupName ,availVolumes )  ||  availVolumes.empty() )
	{
		setLastError( errorcodeInvalidRequest ,"no available volumes in [%s]", paras.identifier.c_str() );
		return false;
	}	

	bool bFirstElement				= true;
	bool bAllElementHasUrls			= true;
	bool bAllElementHasVolume		= true;

	VolumeNameAttrS		candidateVolumes;

	int32 assetValue = 0;

	std::vector<SelectIntentionParam::PlaylistItemInfo>::const_iterator itAsset = paras.playlist.begin();
	for( ; itAsset != paras.playlist.end(); itAsset ++ )
	{//query LAM for every single asset for element information such as content_name bandwidth cuein cueout	
		MLOG(ZQ::common::Log::L_DEBUG,REQFMT(CandidateVolumesBuilder,"query LAM for PID[%s] PAID[%s] SID[%s] primary[%s]"), 
			itAsset->pid.c_str() , itAsset->paid.c_str() , itAsset->sid.c_str() ,
			itAsset->primaryAsset ?"TRUE":"FALSE");

		AEInfo3Collection aeInfos;
		try
		{
			if( mEnv.contentInTestMode() )
			{//test mode
				if( mEnv.mPid2Elements.empty())
					aeInfos = getElementInfoInTestMode( paras );
				else
					aeInfos = getElementInfoInTestMode(  itAsset->pid , itAsset->paid , itAsset->sid  );
			}
			else if( mEnv.GBMode() )
			{
				com::izq::am::facade::servicesForIce::AEInfo3 info;
				info.name = itAsset->paid + itAsset->pid; // suppose pid is empty
				info.cueIn = itAsset->cuein;
				info.cueOut = itAsset->cueout;
				info.bandWidth = itAsset->bandWidth;
				std::string nasUrl = mEnv.getNasUrlPrefix();
				if(nasUrl[nasUrl.size()-1] == '\\')
					nasUrl = nasUrl + info.name;
				else
					nasUrl = nasUrl + "\\" + info.name;
				std::vector<std::string>	urls;
				urls.push_back(nasUrl);
				info.nasUrls = urls;
				aeInfos.push_back(info);
			}
			else if( mEnv.contentLibMode() )
			{
				aeInfos = getElementInfoFromContentLib( itAsset->pid , itAsset->paid , itAsset->sid );
			}
			else
			{
				aeInfos = getElementInfoFromLAM( itAsset->pid , itAsset->paid , itAsset->sid );
			}
			
			for( AEInfo3Collection::const_iterator itAeInfo = aeInfos.begin() ; itAeInfo != aeInfos.end() ; itAeInfo ++ )
			{//log every elements information here
				MLOG(ZQ::common::Log::L_INFO,REQFMT(CandidateVolumesBuilder,"get AE info for PID[%s] PAID[%s] SID[%s]: name[%s] bandwidth[%d] cueIn[%d] cueOut[%d] urls[%s] volumes[%s] attr{%s}"),
					itAsset->pid.c_str() , itAsset->paid.c_str() , itAsset->sid.c_str(),
					itAeInfo->name.c_str(), itAeInfo->bandWidth, itAeInfo->cueIn, itAeInfo->cueOut,
					ZQTianShan::Util::dumpTianShanIceStrValues(itAeInfo->nasUrls).c_str(),
					ZQTianShan::Util::dumpTianShanIceStrValues(itAeInfo->volumeList).c_str() ,
					ZQTianShan::Util::dumpStringMap(itAeInfo->attributes).c_str() );
			}
		}
		catch( const LogicError&)
		{//if failed to query information for a single asset, just return 			
			setLastError( errorcodeAssetNotFound,"query LAM failed, PID[%s] PAID[%s] SID[%s], caught [LogicError] ",
									itAsset->pid.c_str() , itAsset->paid.c_str() , itAsset->sid.c_str()	);
			return false;
		}
		catch( const Ice::Exception& ex )
		{
			setLastError( errorcodeInternalError ,"query LAM/ContentLib failed, PID[%s] PAID[%s] SID[%s], caught [%s] ",
							itAsset->pid.c_str() , itAsset->paid.c_str() ,  itAsset->sid.c_str(), ex.ice_name().c_str() );
			return false;
		}

		//adjust parameter of asset
		//record element information which has been adjusted
		//And we should use the attribute returned by LAM to determine the status of playlist
		AEInfo3Collection::const_iterator itElement = aeInfos.begin();
		for( ; itElement != aeInfos.end() ; itElement ++ )
		{
			ElementInfo eleInfo;

			if( mEnv.contentInTestMode() && itElement->name.empty() )
			{
				eleInfo.name	= itAsset->paid + itAsset->pid;
			}	
			else
			{
				eleInfo.name	= itElement->name;
			}
			eleInfo.pid		= itAsset->pid;
			eleInfo.paid	= itAsset->paid;
			eleInfo.flags	= itAsset->restrictionFlag;
			eleInfo.range   = itAsset->range;
			
			if( itAsset->primaryAsset )
			{
				/// calculate asset value
				/// if we pump these assets in passthrough way, I may depend on asset value to choose a streamer
				assetValue		+= evaluateString( eleInfo.name );
				assetValue		= assetValue % 32767;
			}

			//adjust max bandwidth
			//use the bigger bandwidth
			mMaxBandwidth = mMaxBandwidth > itElement->bandWidth ? mMaxBandwidth : itElement->bandWidth;
			
			// adjust cuein cueout
			// for cuein, get the bigger one
			eleInfo.cueIn			=	itAsset->cuein > itElement->cueIn ? itAsset->cuein : itElement->cueIn;
			// if cueout is 0 , leave it. or else get the smaller one
			eleInfo.cueOut			=	itAsset->cueout == 0  ? ( itElement->cueOut ) :
															( itElement->cueOut == 0 ? itAsset->cueout : (  itAsset->cueout > itElement->cueOut ? itElement->cueOut : itAsset->cueout ) );
			//copy urls into our record
			eleInfo.urls			=	itElement->nasUrls;

			if( itAsset->primaryAsset)
			{
				if( eleInfo.urls.empty() )
				{//if current element does not have urls, treat that all playlist element can't be streamed off from remote
					bAllElementHasUrls = false;
				}
				if( itElement->volumeList.empty() )
				{//if current element does not have volume list , 
					bAllElementHasVolume = false;
				}
			}
			else
			{//do nothing here
				//just a memo
				//由于非primaryitem需要被当做localitem，而在实际运行中需要cdn的时候由vstrm去做选择				
			}

			if( itAsset->primaryAsset )
			{
				//narrow volume list by intersect two volume list returned by different element
				if( bFirstElement )
				{//first element, record its volume list, and it will be used to intersect with another volume list
					candidateVolumes = parseVolumeNameString( itElement->volumeList );
					bFirstElement = false;
					//record volume information
					eleInfo.volumeAttr = candidateVolumes; 
				}
				else
				{				
					//narrow the volume list by intersect the volume list of two elements\
					//we may get a empty result vector, but that's ok if all elements have urls and we can stream content from remote content library
					VolumeNameAttrS tempVolumeList;

					//TODO: 由于我们所说的volume实际上是netid/partition所构成的，所以在做intersect的时候还需要特殊考虑
					//TODO: 采用新的方法来筛选volume??

					VolumeNameAttrS tmpVolumeNames = parseVolumeNameString( itElement->volumeList );
					eleInfo.volumeAttr = tmpVolumeNames;

					//use our man-made intersection algorithm to filter out the useful volumes
					intersectVolumes( candidateVolumes , tmpVolumeNames , tempVolumeList );

					//take narrowed volume list as final result, or maybe go on with another intersection
					candidateVolumes = tempVolumeList;				
				}
			}
			else
			{
				MLOG(ZQ::common::Log::L_DEBUG,REQFMT(CandidateVolumesBuilder,"do not use volume information from element[%s] which belong to PID[%s] PAID[%s] due to primary[%s]"),
					itElement->name.c_str() , itAsset->pid.c_str() , itAsset->paid.c_str() , itAsset->primaryAsset?"TRUE":"FALSE");
			}

			//set it as primary element if the relative asset if a primary asset
			eleInfo.primaryElement = itAsset->primaryAsset;

			eleInfo.updateContentStatus( itElement->attributes );			
			mElements.push_back( eleInfo );//append the element to the end of the playlist
					
			//log the result we have made on the element
			MLOG(ZQ::common::Log::L_INFO, REQFMT(CandidateVolumesBuilder,"adjusted cueIn/cueOut of content[%s]:  SETUP[%lld/%lld] LAM[%d/%d], taking cueIn[%lld] cueOut[%lld], bandwidth[%d] nasUrl[%s] volumeList[%s]"),
				itElement->name.c_str(),
				itAsset->cuein, itAsset->cueout,
				itElement->cueIn, itElement->cueOut,
				eleInfo.cueIn, eleInfo.cueOut,
				itElement->bandWidth,
				ZQTianShan::Util::dumpTianShanIceStrValues( itElement->nasUrls ).c_str(),
				ZQTianShan::Util::dumpTianShanIceStrValues( itElement->volumeList ).c_str() );
		}		
	}

	SelectIntentionParam& writableParams = const_cast<SelectIntentionParam&>(paras);
	writableParams.assetValue = assetValue;

	if( mElements.size() <= 0 )
	{		
		setLastError( errorcodeAssetNotFound ,"no elements available");
		return false;
	}

	//Now, we have already gotten a narrowed volume list but not the final result
	//filter out the volume which is not exist in availVolumes, after this action, we can get the final first class volume list result as expect

	if( !candidateVolumes.empty() )
	{//add the candidate volume into first class volume cab, these volume has the to priority in streamer selection
		mSortedVolumes.addFirstClassVolumes( availVolumes , candidateVolumes );
	}
	
	if( bAllElementHasUrls )
	{//whole playlist can be streamed off from remote content library
		// add volume which support remote NAS streaming
		// these volumes will be sorted by its cache level
		const ResourceVolumeAttrMap& nasVolumes = mResManager.getSupportNasVolumes( paras.groupName );
		mSortedVolumes.addTouristClassVolumes( nasVolumes );
	}

	if( !mSortedVolumes.isValid() )
	{		
		setLastError( errorcodeAssetNotFound ,"no available volume found" );
		return false;
	}
	else
	{
		/// adjust volume sequence according to requested volume
		/// the way I adjust the volume sequence is put the requested volume at the begin of the volume list
		mSortedVolumes.adjustFirstClassVolumes( paras.volume );

		checkPlaylistCachingStatus();

		MLOG(ZQ::common::Log::L_INFO,REQFMT(CandidateVolumesBuilder,"got candidate volumes: %s"),mSortedVolumes.getCandidateVolumename().c_str() );

		//TODO: should I print the candidate volume name here ?
		return true;
	}	
}

VolumeNameAttrS CandidateVolumesBuilder::parseVolumeNameString( const std::string& volumeString ) const
{
	VolumeNameAttrS rets;
	VolumeNameAttr ret;
	if( parseVolumeName(volumeString , ret ) )
	{
		rets.push_back(ret);
	}
	return rets;
}

VolumeNameAttrS CandidateVolumesBuilder::parseVolumeNameString( const std::vector<std::string>& volumesString ) const
{
	VolumeNameAttrS rets;
	VolumeNameAttr ret;
	std::vector<std::string>::const_iterator it = volumesString.begin();
	for( ; it != volumesString.end() ; it ++ )
	{
		if( parseVolumeName(*it, ret) )
		{
			rets.push_back(ret);
		}
	}	
	return rets;
}

struct VolumeNameAttrCmp 
{
	bool operator()( const VolumeNameAttr& a, const VolumeNameAttr& b ) const
	{
		return a.netId < b.netId;
	}
};
void CandidateVolumesBuilder::intersectVolumes( const VolumeNameAttrS& volumesA, const VolumeNameAttrS& volumesB, VolumeNameAttrS& result )
{
	result.clear();

	VolumeNameAttrS A = volumesA;
	VolumeNameAttrS B = volumesB;

	//step 1
	//sort volume name
	std::sort( A.begin() , A.end(),VolumeNameAttrCmp() );
	std::sort( B.begin() , B.end(),VolumeNameAttrCmp() );
	
	VolumeNameAttrS::const_iterator itA = A.begin();
	VolumeNameAttrS::const_iterator itB = B.begin();
	
	while ( ( itA != A.end() ) && ( itB != B.end() ) )
	{
		if( itA->netId < itB->netId )
		{
			*itA++;
		}
		else if( itA->netId == itB->netId )
		{
			result.push_back( *itA );
			itA++;
			itB++;
		}
		else
		{
			itB++;
		}
	}
}

void CandidateVolumesBuilder::attachIntention( SelectionIntention& intention  )
{
	mSeq				= intention.getCseq();
	mMethod				= intention.getMethod();
	mSessId				= intention.getSessId();

	//set max bandwidth to requestBW, and this value may be adjusted after we get elements information from LAM
	mMaxBandwidth		= intention.getParameter().requestBW;
}

bool CandidateVolumesBuilder::build( const SelectIntentionParam& paras )
{
	if( !getElementInfo( paras ))
	{
		return false;
	}	
	return true;
}

//////////////////////////////////////////////////////////////////////////
///PriorityVolumeSet
/***********************************************************************
SortPriorityVolume class provider a simple unique method for user to 
access volumes used to select streamer
***********************************************************************/

PriorityVolumeSet::iterator::iterator( PriorityVolumeSet& volumes )
:mVolumes(volumes),
mbInFirstClass(true)
{
	if( mVolumes.mFirstClassVolumes.size() > 0 )
	{
		mIt = mVolumes.mFirstClassVolumes.begin();
		mbInFirstClass = true;
	}
	else
	{
		mIt = mVolumes.mTouristClassVolumes.begin();
		mbInFirstClass = false;
	}
	
}
PriorityVolumeSet::iterator::~iterator()
{
}
void PriorityVolumeSet::iterator::operator++(int)
{
	if( mbInFirstClass )
	{
		if( mIt == mVolumes.mFirstClassVolumes.end() )
		{
			mbInFirstClass = false;
			mIt = mVolumes.mTouristClassVolumes.begin();
		}
		else
		{
			mIt++;
			if( mIt == mVolumes.mFirstClassVolumes.end() )
			{
				mbInFirstClass = false;
				mIt = mVolumes.mTouristClassVolumes.begin();
			}
		}
	}
	else
	{
		mIt++;
	}	
}
bool PriorityVolumeSet::iterator::isEnd( ) const
{
	if( !mbInFirstClass)
		return mIt == mVolumes.mTouristClassVolumes.end();
	else	
		return false;	
}

PriorityVolumeSet::PriorityVolumeSet(SelectionEnv& env , SelectionIntention& intension, NgodResourceManager&	resManager)
:mEnv(env),
mItVolume( mFirstClassVolumes.end() ),
mCurVolumeCacheLevel(0xFFFFFFFF),
mbInFirstClass(false),
mIntention(intension),
mResManager(resManager)
{
}
PriorityVolumeSet::~PriorityVolumeSet( )
{
}

PriorityVolumeAttrSet::iterator PriorityVolumeSet::begin() 
{
	return mFirstClassVolumes.begin();
}

PriorityVolumeAttrSet::iterator PriorityVolumeSet::end() 
{
	return mTouristClassVolumes.end();
}

int PriorityVolumeSet::getVolumeCacheLevel( const std::string& volumeNetId ) const
{
	ResourceVolumeAttr attr;
	if(mResManager.getVolumeAttr( volumeNetId, attr ) )
	{
		return attr.level;
	}
	return 99999;
}
std::string PriorityVolumeSet::getCandidateVolumename( ) const
{
	std::ostringstream oss;
	PriorityVolumeAttrSet::const_iterator itFirstClass = mFirstClassVolumes.begin();
	for( ; itFirstClass != mFirstClassVolumes.end() ; itFirstClass++ )
	{
		oss<<"["<<itFirstClass->netId<<"|"<<itFirstClass->level<<"] ";
	}
	PriorityVolumeAttrSet::const_iterator itTourist = mTouristClassVolumes.begin();
	for( ; itTourist != mTouristClassVolumes.end() ; itTourist ++ )
	{
		oss<<"["<<itTourist->netId<<"|"<<itTourist->level<<"] ";
	}
	return oss.str();
}
ContentCachingStatus PriorityVolumeSet::getContentCachingStatus( const std::string& volumeNetId ) const
{
	VolumeAttrEx v;
	v.netId = volumeNetId;
	v.level = getVolumeCacheLevel(volumeNetId);
	PriorityVolumeAttrSet::const_iterator it = mFirstClassVolumes.find(v);
	if( it != mFirstClassVolumes.end() )
		return it->mContentStatus;
	it = mTouristClassVolumes.find(v);
	if( it != mTouristClassVolumes.end() )
		return it->mContentStatus;
	return STATUS_CONTENT_CACHING_NULL;
}

std::string PriorityVolumeSet::getImportingNodeId( const std::string& volumeNetId ) const
{
	VolumeAttrEx v;
	v.netId = volumeNetId;
	v.level = getVolumeCacheLevel(volumeNetId);
	PriorityVolumeAttrSet::const_iterator it = mFirstClassVolumes.find(v);
	if( it != mFirstClassVolumes.end() )
		return it->mImportingNodeNetId;
	it = mTouristClassVolumes.find(v);
	if( it != mTouristClassVolumes.end() )
		return it->mImportingNodeNetId;
	return std::string("");
}

void PriorityVolumeSet::adjustFirstClassVolumes( const std::string& requestedVolume )
{
	if( requestedVolume.empty()  )
		return;
	PriorityVolumeAttrSet::iterator it = mFirstClassVolumes.begin();
	for( ; it != mFirstClassVolumes.end() ; it++ )
	{
		if( it->netId == requestedVolume )
		{
			VolumeAttrEx tmp = *it;
			mFirstClassVolumes.erase( it );
			tmp.level = 0;
			mFirstClassVolumes.insert( tmp );
			MLOG(ZQ::common::Log::L_INFO,COMMFMT(adjustFirstClassVolumes,"adjust volumes by request volume[%s]"), requestedVolume.c_str() );
			return;
		}
	}
	MLOG(ZQ::common::Log::L_WARNING, COMMFMT(adjustFirstClassVolumes,"can't find request volume[%s] in LAM's returned value") , requestedVolume.c_str() );
}

void PriorityVolumeSet::addFirstClassVolumes(const ResourceVolumeAttrMap& volumeAttrMap , const VolumeNameAttrS& candidateVolNames )
{
	VolumeNameAttrS::const_iterator itName = candidateVolNames.begin();
	for( ; itName != candidateVolNames.end() ; itName++ )
	{
		ResourceVolumeAttrMap::const_iterator itVolume = volumeAttrMap.find( itName->netId );
		if( itVolume == volumeAttrMap.end() )
			continue;
		VolumeAttrEx attr;
		attr.netId				= itVolume->second.netId;
		attr.partitions			= itVolume->second.partitions;
		attr.level				= itVolume->second.level;
		attr.bSupportNas		= itVolume->second.bSupportNas;
		attr.bSupportCache		= itVolume->second.bSupportCache;
		attr.bAllPartitions		= itVolume->second.bAllPartitions;
		attr.bLocalPlaylist		= true;// true because this is the first class volumes

		mFirstClassVolumes.insert( attr );
	}
}

void PriorityVolumeSet::addTouristClassVolumes( const ResourceVolumeAttrMap& volumeAttrMap )
{
	ResourceVolumeAttrMap::const_iterator itVolume = volumeAttrMap.begin();
	for( ; itVolume != volumeAttrMap.end() ; itVolume ++ )
	{		
		VolumeAttrEx attr;
		attr.netId				= itVolume->second.netId;
		attr.partitions			= itVolume->second.partitions;
		attr.level				= itVolume->second.level;
		attr.bSupportNas		= itVolume->second.bSupportNas;
		attr.bSupportCache		= itVolume->second.bSupportCache;
		attr.bAllPartitions		= itVolume->second.bAllPartitions;
		attr.bLocalPlaylist		= false;// true because this is the first class volumes

		if( mFirstClassVolumes.find(attr) != mFirstClassVolumes.end() )
			continue;//do not add into tourist class volumes if it exist in first class volumes
		mTouristClassVolumes.insert( attr );
	}
}


bool PriorityVolumeSet::findFirstVolumeset( ) 
{
	if( mFirstClassVolumes.size() > 0 )
	{
		//current iterator point to first class set
		mItVolume = mFirstClassVolumes.begin();
		mbInFirstClass = true;

		//get current volume's cache level
		mCurVolumeCacheLevel = mItVolume->level;
		return true;
	}
	else if ( mTouristClassVolumes.size() > 0 )
	{
		////current iterator point to tourist class set because first class has no available volumes
		mItVolume = mTouristClassVolumes.begin();
		mbInFirstClass = false;

		//get current volume's cache level
		mCurVolumeCacheLevel = mItVolume->level;
	}
	else
	{
		mItVolume = mTouristClassVolumes.end();
	}
	return mItVolume != mTouristClassVolumes.end();
}

bool PriorityVolumeSet::findNextVolumeset( PriorityVolumeAttrSet& volumeset , bool& bLocalPlaylist )
{
	if( !mbInFirstClass && mItVolume == mTouristClassVolumes.end() )
		return false;

	volumeset.clear();
	if( mbInFirstClass )
	{//actually , all volumes in the first class cab has the same priority
		// select all volumes in the first class cab as a volume set which will be used in streamer selection
		while( mItVolume != mFirstClassVolumes.end() )
		{
			volumeset.insert( *mItVolume );
			mItVolume++;
		}
		
		mbInFirstClass = false;

		if( mTouristClassVolumes.size() > 0 )
		{
			//reset mCurVolumeCacheLevel and mItVolume if tourist class cab has volumes in there
			mItVolume = mTouristClassVolumes.begin();
			mCurVolumeCacheLevel = mItVolume->level;
		}
		else
		{
			mItVolume = mTouristClassVolumes.end();
		}	
		
		if( volumeset.size() > 0 )
		{ //same level volume has same bLocalPlaylist property
			bLocalPlaylist = volumeset.begin()->bLocalPlaylist;
			return true;
		}
	}

	if( !mbInFirstClass )
	{
		//select volumes in tourist class cab, these volumes are classified by its level
		while( ( mItVolume != mTouristClassVolumes.end() ) && ( mCurVolumeCacheLevel == mItVolume->level ) )
		{
			volumeset.insert( *mItVolume );
			mItVolume++;
		}
		if( mItVolume != mTouristClassVolumes.end() )
		{//reset level value to next volume level so that we can get volumes of next level in next run
			mCurVolumeCacheLevel = mItVolume->level;
		}
		if( volumeset.size() > 0 )
		{
			bLocalPlaylist = volumeset.begin()->bLocalPlaylist;
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
///StreamerSelection
StreamerSelection::StreamerSelection( SelectionEnv& env ,NgodResourceManager& resManager, SelectionIntention& intention )
:SelectionCommand(env,intention),
mVolumesBuilder(env,resManager,intention),
mResManager(resManager),
mItStreamer(mCandidateStreamers.end()),
mLastErrorCode( errorcodeOK ),
mbCommitted(false),
mbHasValidStreamerSelected(false),
mIntention(intention)
{
}

StreamerSelection::~StreamerSelection( )
{
	if( !mbCommitted )
	{//if the selection if not committed, just release resource of the streamer just selected
		releaseResource();
	}
}

void StreamerSelection::commit()
{
	mbCommitted = true;
	if( isSelectedStreamerValid() )
	{//we get a streamer and we successfully apply a stream on it, confirm it to resource manager to let it know
		mResManager.confirmResource( mResourceRequest , getSelectedStreamerNetId() , mbLocalPlaylist );		
		if( !mbLocalPlaylist )
			mEnv.getAssetStackManager().registerSession( mVolumesBuilder.getElements() , getSelectedStreamerNetId() );
	}
}

bool StreamerSelection::findFirstStreamer()
{
	//find first streamer
	//actually , these function find all available volumes and prepare the resource to findNextStreamer
	mVolumesBuilder.attachIntention( mIntention );

	if( !mVolumesBuilder.build( mIntention.getParameter() ) )
	{//if failed to find volumes , return false
		//and we should not go on with the rest steps
		setLastError( mVolumesBuilder.getLastError() ,mVolumesBuilder.getErrorMsg().c_str() );
		return false;
	}

	//we get available volumes
	PriorityVolumeSet& volumes = mVolumesBuilder.getVolumeSet();

	//initialize resource request
	mResourceRequest.identifier			= mIntention.getParameter().identifier;
	mResourceRequest.requestBW			= (int32)mIntention.getParameter().requestBW;

	mResourceRequest.method				= mIntention.getMethod();
	mResourceRequest.cseq				= mIntention.getCseq();
	mResourceRequest.sessionId			= mIntention.getSessId();

	//this code must return true
	return volumes.findFirstVolumeset();	
}

void convertVolumeSetToVolumeNetIdset( const PriorityVolumeAttrSet& volumeSet, std::set<std::string>& volumeNames )
{
	volumeNames.clear();
	PriorityVolumeAttrSet::const_iterator it = volumeSet.begin();
	for( ; it != volumeSet.end() ; it ++ )
	{
		volumeNames.insert( it->netId );
	}
}

struct StreamerWeightSortCmp
{
	bool operator()( const ResourceStreamerAttrEx& a, const ResourceStreamerAttrEx& b ) const
	{
		return a.streamerWeight > b.streamerWeight;
	}
};

/*
findNextStreamer()
1. 首先，如果上一次有选中的streamer，那么需要把该streamer的记录从可选记录中去除
2. 如果bSkipToNextVolume为true，那么直接清空当前的streamer记录，去取下一个volumeset的streamer
3. 如果当前的streamer为空，去取下一个volumeset的streamer
4. 如果已经无streamer可用,报错	
*/

std::string streamerNetIdsToString( const ResourceStreamerAttrExS& streamers )
{
	std::ostringstream oss;
	ResourceStreamerAttrExS::const_iterator it = streamers.begin();
	for ( ; it != streamers.end() ; it++ )
	{
		oss << it->netId << "<" << it->streamerWeight <<"> " ;
	}
	return oss.str();
}

bool StreamerSelection::getCandidateStreamers( bool& bLocalPlaylist )
{
	bool bFindOk = false;
	
	while( true )
	{//don't be surprised here because we can quit the loop if no more volumes can be found to use
		PriorityVolumeAttrSet volumeAttrs;
		
		PriorityVolumeSet& volumes = mVolumesBuilder.getVolumeSet();
		//fine available volumes 
		if( !volumes.findNextVolumeset( volumeAttrs , bLocalPlaylist ) )
		{//no more volumes , return false			
			if(!mbHasValidStreamerSelected)
			{
				setLastError( errorcodeNotEnoughVolBandwidth , "no more available volume" );
			}
			return false;
		}
		
		//find available volumes, reset resource request parameter
		mResourceRequest.bNeedImportChannel	= !bLocalPlaylist;
		mResourceRequest.volumeNetIds.clear();
		convertVolumeSetToVolumeNetIdset( volumeAttrs , mResourceRequest.volumeNetIds );

		MLOG(ZQ::common::Log::L_INFO, COMMFMT(StreamerSelection,"getCandidateStreamers() try candidate volumes [%s]"), ZQTianShan::Util::dumpStringSets(mResourceRequest.volumeNetIds).c_str() );

		//find suitable streamer from resource manager
		if( !mResManager.getStreamersFromSopAndVolume( mResourceRequest , mCandidateStreamers ) )
		{// oops! try another volumes because we can't find any suitable streamer for these volumes
			
			setLastError( errorcodeNotEnoughNetworkBandwidth,"failed to get available streamer according to sop[%s] volumes[%s]",
						mResourceRequest.identifier.c_str() ,
						ZQTianShan::Util::dumpStringSets(mResourceRequest.volumeNetIds).c_str() );
			mItStreamer = mCandidateStreamers.end();
			//try another volumes, so we use continue , not break or return
			continue;
		}
		MLOG(ZQ::common::Log::L_DEBUG,COMMFMT(StreamerSelection,"getCandidateStreamers() gotten candidate streamer [%s] through volume[%s]"),
			streamerNetIdsToString(mCandidateStreamers).c_str() , ZQTianShan::Util::dumpStringSets(mResourceRequest.volumeNetIds).c_str() );
		//we successfully get suitable streamers, quit the loop and set bFindOk to true
		bFindOk = true;
		break;
	}
	
	//we just only get available streamers
	//reset mItStreamer to an invalid position because we have not selected a streamer yet 	
	mItStreamer = mCandidateStreamers.end();

	return bFindOk;
}


// int upgradeWeight( int weight )
// {
// 	static int threashold = 9900 ;
// 	int cost = MAX_WEIGHT - weight;
// 	return (MAX_WEIGHT - (int)(pow((long double)(cost/1000),4)));	
// }
// 
// int downgradeWeight( int weight )
// {
// 	static int threashold = 9900 ;
// 	int cost = MAX_WEIGHT - weight;	
// 	return (MAX_WEIGHT - (threashold + cost/(MAX_WEIGHT-threashold)));
// }

int StreamerSelection::caclNewWeight( int weight , bool bUpgrade )
{
	if( !mEnv.mbEnableAssetStack )
		return weight;
	
	static int threshold = mEnv.mAssetStackAdjustWeight;

	int cost = MAX_WEIGHT - weight;
	if( bUpgrade )
	{
		cost = (int)(pow((long double)(cost/1000),4));
	}
	else
	{
		cost = threshold + cost/(MAX_WEIGHT-threshold);
	}
	return MAX_WEIGHT - cost;
}
void StreamerSelection::adjustWeightByAssetValue( )
{
	if( mEnv.mAssetStackStartMode >= 1 )
	{
		MLOG(ZQ::common::Log::L_INFO,COMMFMT(StreamerSelection,"adjustWeightByAssetValue() select node by load accroding to configuration"));
		return;
	}
	if( mCandidateStreamers.size() <= 0 ) return;

	std::set<std::string> nodes;
	for( ResourceStreamerAttrExS::iterator  itStreamer = mCandidateStreamers.begin();  itStreamer != mCandidateStreamers.end() ; itStreamer ++ )
	{
		nodes.insert( itStreamer->nodeId );
	}	

	//get asset value;
	int32 assetValue = mIntention.getAssetValue();
	std::string preferNodeNetId;
	if(nodes.size() >= 1 ) 
	{
		int index = (int)(assetValue % nodes.size());
		std::set<std::string>::const_iterator it = nodes.begin();
		for( int i = 0 ;i < index ; i ++ )
			it++;
		if( it != nodes.end() )
			preferNodeNetId = *it;
	}
	if( preferNodeNetId.empty() )
	{
		MLOG(ZQ::common::Log::L_ERROR,COMMFMT(StreamerSelection,"adjustWeightByAssetValu() logical error"));
		return;
	}
	MLOG(ZQ::common::Log::L_DEBUG, COMMFMT(StreamerSelection,"adjustWeightByAssetValue() adjust weight: node[%s]  assetvalue[%d]"),
		preferNodeNetId.c_str() , assetValue);
	size_t i = 0;
	for( ResourceStreamerAttrExS::iterator  itStreamer = mCandidateStreamers.begin();  itStreamer != mCandidateStreamers.end() ; itStreamer ++ )	
	{
		if( itStreamer->nodeId == preferNodeNetId )
		{
			itStreamer->streamerWeight = caclNewWeight( itStreamer->streamerWeight , true );
		}
		else
		{
			itStreamer->streamerWeight = caclNewWeight( itStreamer->streamerWeight , false );
		}		
	}
}

void StreamerSelection::adjustWeightByImportingHost( )
{
	std::set<std::string> importingNodes;
	for( ResourceStreamerAttrExS::iterator itStreamer = mCandidateStreamers.begin();  itStreamer != mCandidateStreamers.end() ; itStreamer ++ )
	{//walk through all candidate streamers
		//if the streamer belongs to the node of "ImportEdgeNode", amplify it's weight
		std::string importingNode = mVolumesBuilder.getImportingNodeId(itStreamer->volumeNetId);
		if( !importingNode.empty() ) importingNodes.insert( importingNode );
		if ( itStreamer->nodeId == importingNode )
		{
			itStreamer->streamerWeight = caclNewWeight( itStreamer->streamerWeight ,true );
		}
		else
		{
			itStreamer->streamerWeight = caclNewWeight( itStreamer->streamerWeight , false);
		}
	}
	MLOG(ZQ::common::Log::L_DEBUG,COMMFMT(StreamerSelection,"adjustWeightByImportingHost() adjust streamer weight by ImportingHost[%s]"),
		ZQTianShan::Util::dumpStringSets(importingNodes).c_str() );
}

void StreamerSelection::adjustWeight()
{
	bool bImporting = false;
	bool bAllLocal = true;
	//walk all candidate volumes, check the content status in the volume and maybe adjust associated streamer's weight
	ResourceStreamerAttrExS::iterator itStreamer = mCandidateStreamers.begin();
	for( ; itStreamer != mCandidateStreamers.end() ; itStreamer ++ )
	{//walk through all candidate streamers		
		const std::string volumeNetId = itStreamer->volumeNetId;
		ContentCachingStatus contentStatus = mVolumesBuilder.getPlaylistStatus( volumeNetId );
		switch ( contentStatus )//actually this is playlist status
		{
		case STATUS_CONTENT_CACHING_NULL:
			{
				
			}
			break;
		case STATUS_CONTENT_CACHING_NONE:
			{
				bAllLocal = false;
			}
			break;
		case STATUS_CONTENT_CACHING_IMPORTING:
			{
				bImporting = true;
				bAllLocal = false;					
			}
			break;
		case STATUS_CONTENT_CACHING_LOCAL:			
			break;
		default:
			break;
		}
	}
	if( !bAllLocal )
	{
		if( bImporting )
		{
			MLOG(ZQ::common::Log::L_DEBUG,REQFMT(StreamerSelection,"adjustWeightByImportingHost"));
			adjustWeightByImportingHost();
		}
		else
		{			
			adjustWeightByAssetValue();
		}
	}
}

bool StreamerSelection::commitStreamerForResource( bool bLocalPlaylist)
{

	////adjust streamer weight
	adjustWeight();	

	///sort the candidate streamers, 
	bool bCommitOk = false;
	while( !bCommitOk )
	{
		if( mCandidateStreamers.size() <= 0 )
		{
			MLOG(ZQ::common::Log::L_DEBUG,REQFMT(StreamerSelection,"no more candidate streamers, try another volume"));
			return false;
		}

		MLOG(ZQ::common::Log::L_DEBUG,COMMFMT(StreamerSelection,"commitStreamerForResource() trying to get a streamer after weight re-adjusted in [%s]"),
			streamerNetIdsToString( mCandidateStreamers ).c_str() );

		// try to select a streamer
		// 
		int64 totalWeight		= 0;
		//sum the total weight
		{
			for( ResourceStreamerAttrExS::iterator itStreamer = mCandidateStreamers.begin() ; itStreamer != mCandidateStreamers.end() ; itStreamer++ )
			{
				//take the smaller one of Streamer Weight and importChannel weight		
				totalWeight += itStreamer->streamerWeight;
			}
			if( totalWeight <= 0 )
			{//should never be here
				//if a streamer's weight <= 0, the streamer should never be returned
				assert(false);
				return false;
			}
		}		
		int seed = (int)(rand() % totalWeight);
		std::sort( mCandidateStreamers.begin() , mCandidateStreamers.end() , StreamerWeightSortCmp() );

		ResourceStreamerAttrExS::iterator itStreamer = mCandidateStreamers.begin();
		while ( ( itStreamer != mCandidateStreamers.end() ) && (itStreamer->streamerWeight <= seed) )
		{//select a streamer by it's weight and random seed
			seed -= itStreamer->streamerWeight;
			itStreamer++;
		}
		if( itStreamer == mCandidateStreamers.end() )
		{			
			//should never be here
			assert( false );
			abort();
		}	

		if( !mResManager.allocateResource( mResourceRequest , itStreamer->netId ))
		{	
			MLOG(ZQ::common::Log::L_WARNING,REQFMT(StreamerSelection,"failed to commit resource for streamer[%s]"),itStreamer->netId.c_str() );
			mCandidateStreamers.erase( itStreamer );
			mItStreamer = mCandidateStreamers.end();			
			continue;
		}
		//set mItStreamer to the streamer selected so that we can get any information from it if we want
		mItStreamer = itStreamer;

		//get volume attribute with current selected streamer, this is a shortcut to get volume information for using
		mResManager.getVolumeAttr( mItStreamer->volumeNetId , mSelectedVolumeAttr );

		//record if this is a local cached playlist or not
		mSelectedVolumeAttr.bLocalPlaylist = bLocalPlaylist;

		//set bCommitOk to true , so we can return because we already get a useful streamer
		bCommitOk = true;
	}
	return bCommitOk;
}

void StreamerSelection::setLastError( int32 errorCode , const char* fmt , ... )
{	
	char szLocalBuffer[1024];
	va_list args;
	va_start(args, fmt);
	int nCount = _vsnprintf( szLocalBuffer, sizeof(szLocalBuffer)-1, fmt, args );
	va_end(args);
	if(nCount == -1)
	{
		szLocalBuffer[ sizeof(szLocalBuffer) - 1 ] = 0;
	}
	else
	{
		szLocalBuffer[nCount] = '\0';
	}
	mLastErrorCode		= errorCode;
	mErrorDescription	= szLocalBuffer;
}

bool StreamerSelection::findNextStreamer( bool bSkipToNextVolume , bool bAddPenalty )
{
	// remove streamer of last selection if available
	releaseResource( bAddPenalty ,bSkipToNextVolume );	

	// detect if current candidate streamers are available and bSkipToNextVolme 
	bool bQueryForNextVolumeset = ( mCandidateStreamers.size() <= 0 ) ;

	mbLocalPlaylist = true;

	bool bCommitOk = false;

	while( !bCommitOk )
	{
		if( bQueryForNextVolumeset )
		{
			if( !getCandidateStreamers(mbLocalPlaylist) )
			{
				return false;
			}
		}
		if( commitStreamerForResource( mbLocalPlaylist ) )
		{
			bCommitOk = true;
			mbHasValidStreamerSelected = true;
			break;
		}
		
		//no available streamers for current volumes, try next volumes
		bQueryForNextVolumeset = true;
	}
	if( !bCommitOk )
	{
		//failed to get a available streamer for use, reset mItStreamer to an invalid position and return false
		mItStreamer = mCandidateStreamers.end();
		return false;
	}

	//actually this code should always return true	
	return mItStreamer != mCandidateStreamers.end();
}
///get selected volume information
const VolumeAttrEx& StreamerSelection::getSelectedVolumeAttr( ) const
{
	return mSelectedVolumeAttr;
}

const ElementInfoS& StreamerSelection::getElements() const
{
	return mVolumesBuilder.getElements();
}
///get selected streamer information
const ResourceStreamerAttrEx& StreamerSelection::getSelectedStreamerAttr( ) const
{
	assert( isSelectedStreamerValid() );
	return *mItStreamer;
}

const std::string StreamerSelection::getSelectedImportChannelName( ) const
{
	assert( isSelectedStreamerValid() );
	return mbLocalPlaylist ? "" : mItStreamer->importChannelName ;
}
const std::string& StreamerSelection::getSelectedStreamerNetId( ) const
{
	assert( isSelectedStreamerValid() );
	return mItStreamer->netId;
}
const std::string& StreamerSelection::getSelectedStreamerEndpoint( ) const
{
	assert( isSelectedStreamerValid() );
	return mItStreamer->endpoint;
}
const std::string& StreamerSelection::getSelectedVolumeName( ) const
{
	assert( isSelectedStreamerValid() );
	return mItStreamer->volumeNetId;
}
int64 StreamerSelection::getAdjustedBandwidth( ) const
{
	return mVolumesBuilder.getMaxBandwidth();
}

struct RemoveStreamersOfCurrentVolume 
{
	RemoveStreamersOfCurrentVolume( const std::string& strVolumeNetId )
		:mVolumeNetId(strVolumeNetId) { }	
	bool operator()( const ResourceStreamerAttrEx& a ) const
	{
		if( a.volumeNetId == mVolumeNetId ){	return true; }
		else {	return false;	}
	}
	std::string		mVolumeNetId;
};

void StreamerSelection::releaseResource( bool bAddPenalty , bool bSkipToNextVolume )
{
	if( mItStreamer !=mCandidateStreamers.end() )
	{
		mResManager.releaseResource( mResourceRequest , mItStreamer->netId , bAddPenalty , mEnv.maxPenaltyValue() );
		if( bSkipToNextVolume )
		{//collect all streamer relative to current selected volume
			ResourceStreamerAttrExS::iterator itDel = std::remove_if( mCandidateStreamers.begin() , mCandidateStreamers.end() , RemoveStreamersOfCurrentVolume(mSelectedVolumeAttr.netId) );
			mCandidateStreamers.erase( itDel , mCandidateStreamers.end() );
		}
		else
		{
			mCandidateStreamers.erase( mItStreamer );
		}
		mItStreamer = mCandidateStreamers.end();
	}
}

bool StreamerSelection::isSelectedStreamerValid( ) const
{
	return (mItStreamer != mCandidateStreamers.end());
}

TianShanIce::Streamer::StreamSmithAdminPrx StreamerSelection::getStreamerProxy( ) const
{
	if(!isSelectedStreamerValid())
		return NULL;
	return mItStreamer->streamServicePrx;
}
