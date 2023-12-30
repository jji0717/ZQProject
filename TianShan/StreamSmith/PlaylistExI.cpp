
#include <ZQ_common_conf.h>
#include "PlaylistExI.h"
#include <StreamSmithSite.h>
#include <Log.h>
#include <TianShanIceHelper.h>

#include "Exception.h"


#ifdef _DEBUG
	#include "adebugmem.h"
#endif




namespace ZQ
{
namespace StreamSmith
{


class amdCallBase : public ZQ::common::ThreadRequest
{
public:
	amdCallBase( PlaylistExI::Ptr pl ,const Ice::Current& c )
		:_plExiPtr(pl),
		_iceCurrent(c),
		ZQ::common::ThreadRequest( StreamSmithSite::m_pPlayListManager->mAmdThreadpool ) {		
	}

	virtual ~amdCallBase( ){}

	bool		start() 
	{
		ZQ::common::NativeThreadPool& pool =  StreamSmithSite::m_pPlayListManager->mAmdThreadpool;
		int pendingSize = pool.pendingRequestSize();
		int poolSize = pool.size();
		int activeCount = pool.activeCount();
		
		int maxPendingSize = gStreamSmithConfig.outOfServiceConf.maxPendingRequest;
		if( maxPendingSize <= 0 )
			maxPendingSize = pool.size() * 2;	
		maxPendingSize = maxPendingSize < 10 ? 10 : maxPendingSize;

		if( pendingSize >= poolSize || pendingSize >= maxPendingSize)
		{
			glog(ZQ::common::Log::L_DEBUG,CLOGFMT(amdCallBase,"amd threadpool status: pending request[%d] active thread[%d/%d]"),
				pendingSize, activeCount, poolSize );			
		}
		if( pendingSize > maxPendingSize ) 
		{
			throw TianShanIce::ServerError("StreamSmith",EXT_ERRCODE_SERVICEUNAVAIL,"too much pending requests");
		}

		return ZQ::common::ThreadRequest::start();
	}

	virtual int run( ){return 0;}

	void final(int retcode /* =0 */, bool bCancelled /* =false */)
	{
		delete this;
	}
protected:
	PlaylistExI::Ptr						_plExiPtr;	
	Ice::Current							_iceCurrent;
};

class amdPlayAsync : public amdCallBase
{
public:
	amdPlayAsync( const ::TianShanIce::Streamer::AMD_Stream_playPtr callback ,  PlaylistExI::Ptr pl ,const Ice::Current& c )
		:_callback(callback),amdCallBase(pl,c)
	{
	}
protected:
	virtual int run( );
private:
	const ::TianShanIce::Streamer::AMD_Stream_playPtr _callback;
};

class amdSeekStreamAsync : public amdCallBase
{
public:
	amdSeekStreamAsync( const TianShanIce::Streamer::AMD_Stream_seekStreamPtr& callback , const Ice::Long& offset , const Ice::Int& startPos , PlaylistExI::Ptr pl ,const Ice::Current& c  )
		:_callback(callback),
		_offset(offset),
		_startPos(startPos),
		amdCallBase(pl,c)
	{

	}
	virtual ~amdSeekStreamAsync(){}
protected:
	virtual int run( );
private:
	const TianShanIce::Streamer::AMD_Stream_seekStreamPtr	_callback;
	Ice::Long												_offset;
	Ice::Int												_startPos;
};

class amdPlayExAsync : public amdCallBase
{
public:
	amdPlayExAsync( const ::TianShanIce::Streamer::AMD_Stream_playExPtr& callback ,  
		Ice::Float newSpeed,::Ice::Long offset, ::Ice::Short from , 
		const ::TianShanIce::StrValues& expectedProps,
		PlaylistExI::Ptr pl,
		const ::Ice::Current& c)
		:_callback(callback),
		_newSpeed(newSpeed),
		_offset(offset),
		_from(from),
		_expectedProps(expectedProps),
		amdCallBase(pl,c)
	{

	}
	virtual ~amdPlayExAsync( ){}
protected:
	virtual int run( );
private:
	const ::TianShanIce::Streamer::AMD_Stream_playExPtr				_callback;
	Ice::Float														_newSpeed;
	Ice::Long														_offset;
	Ice::Short														_from;
	TianShanIce::StrValues											_expectedProps;
};


class amdSkipToItemAsync : public amdCallBase
{
public:
	amdSkipToItemAsync( const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr& callback , 
		Ice::Int where , bool bPlay , PlaylistExI::Ptr pl , const Ice::Current& c)
		:_callback(callback),
		_where(where),
		_bPlay(bPlay),
		amdCallBase(pl,c)
	{

	}
	virtual ~amdSkipToItemAsync( ){}
protected:
	virtual int		run( );
private:	
	const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr					_callback ;
	Ice::Int																	_where;
	bool																		_bPlay;
};

class amdSeekToPositionAsync : public amdCallBase
{
public:
	amdSeekToPositionAsync( const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr& callback, 
		::Ice::Int UserCtrlNum, ::Ice::Int timeOffset, ::Ice::Int startPos, 
		PlaylistExI::Ptr pl, const ::Ice::Current& c)
		:_callback(callback),
		_userCtrlNum(UserCtrlNum),
		_timeOffset(timeOffset),
		_startPos(startPos),
		amdCallBase(pl,c)
	{

	}
	virtual ~amdSeekToPositionAsync( ){;}
protected:
	virtual int run();
private:
	const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr				_callback;
	Ice::Int																	_userCtrlNum;
	Ice::Int																	_timeOffset;
	Ice::Int																	_startPos;

};

class amdPlayItemAsync : public amdCallBase
{
public:
	amdPlayItemAsync(const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr& callback, 
		::Ice::Int UserCtrlNum, ::Ice::Int timeOffset, ::Ice::Short from, ::Ice::Float newSpeed,
		const ::TianShanIce::StrValues& expectedProps, PlaylistExI::Ptr pl,  const ::Ice::Current& c)
		:_callback(callback),_UserCtrlNum(UserCtrlNum),
		_timeOffset(timeOffset),_from(from),_newSpeed(newSpeed),
		_expectedProps(expectedProps),
		amdCallBase(pl,c)
	{

	}
	virtual ~amdPlayItemAsync(){}
protected:
	virtual int run();
private:
	const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr						_callback;
	::Ice::Int																	_UserCtrlNum;
	::Ice::Int																	_timeOffset;
	::Ice::Short																_from;
	::Ice::Float																_newSpeed;
	::TianShanIce::StrValues													_expectedProps;
};

#ifdef _ICE_INTERFACE_SUPPORT

using namespace ZQ::common;
#define __STR__(x) #x
#define STR_(x) __STR__(x)
#define		LOCAL_ERROR_STRING(x)	ZQ::StreamSmith::Playlist*							_pList;\
									(ZQ::StreamSmith::Playlist*) _pList=StreamSmithSite::m_pPlayListManager->find(ZQ::common::Guid(attr.Guid.c_str()));\
									if(!_pList)\
									{\
									glog(Log::L_INFO, " (line:%d [SESSID:%s])null playlist instance",__LINE__,attr.Guid.c_str());\
									ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1001,"Can't find playlist %s",attr.Guid.c_str());\
									##x;}
#define		PLSESSID(x,y) 	"[Playlist] SESSID(%s)Thread[%10u][%16s]\t"##y,attr.Guid.c_str(),GetCurrentThreadId(),#x
//#define		TEMP_ERROR_STRING(x)	Playlist* pTem_pList=StreamSmithSite::m_pPlayListManager->find(ZQ::common::Guid(attr.Guid.c_str()));\
//									if(!pTempList)\
//									{throw ZQ::common::Exception("NULL playlist pointer");\
//									glog(Log::L_INFO, STR_(__LINE__)" null playlist instance");##x;}			


PlaylistExI::PlaylistExI ()
{
	/*_pList=NULL;*/
	_sessionPrx=NULL;
	_bPlaylistShouldDestroy=false;
	_pathTicketPrx=NULL;
//	_pList=StreamSmithSite::m_pPlayListManager->find(ZQ::common::Guid(attr.Guid.c_str()));
//	if(!_pList)
//	{
//		glog(Log::L_ERROR,"PlaylistExI::PlaylistExI ()##Can't get playlist instance with guid=%s",attr.Guid.c_str());
//	}
}
PlaylistExI::PlaylistExI(const Ice::Identity& Iceid,
						 const ::std::string& strGuid,
						 const ::TianShanIce::Streamer::PlaylistAttr& plAttr,
						 ZQADAPTER_DECLTYPE objAdapter	)
						 :PlaylistEx(Iceid,strGuid,plAttr)
{
	_objApdater=objAdapter;
	//updateAttr(attr);
	{	
	try
	{
		guid=plAttr.Guid;
		attr.Guid=plAttr.Guid;
		attr.StreamSmithSiteName=plAttr.StreamSmithSiteName;
		attr.ResourceGuid=plAttr.ResourceGuid;
		attr.ClientSessionID=plAttr.ClientSessionID;
		attr.endPoint=plAttr.endPoint;
		attr.MaxRate=plAttr.MaxRate;
		attr.MinRate=plAttr.MinRate;
		attr.NowRate=plAttr.NowRate;
		attr.destIP=plAttr.destIP;
		attr.destMac=plAttr.destMac;
		attr.destPort=plAttr.destPort;
		attr.vstrmPort=plAttr.vstrmPort;
		attr.programNumber=plAttr.programNumber;
		attr.playlistState=plAttr.playlistState;
		attr.currentCtrlNum=plAttr.currentCtrlNum;
		attr.vstrmSessID=plAttr.vstrmSessID;
		attr.streamPID = plAttr.streamPID;
		attr.property	= plAttr.property;
	}
	catch (...)
	{
	}
	}
	ident=Iceid;	
// 	_pList=StreamSmithSite::m_pPlayListManager->find(ZQ::common::Guid(strGuid.c_str()));
// 	if(!_pList)
// 	{
// 		glog(Log::L_DEBUG,"[PlaylistExI] Can't find playlist through guid=%s",strGuid.c_str());
// 		_pList=NULL;
// 	}
	_sessionPrx=NULL;
	_pathTicketPrx=NULL;
	_bPlaylistShouldDestroy=false;	
}
PlaylistExI::~PlaylistExI ()
{	
	IceUtil::RWRecMutex::WLock sync(*this);
}

int amdPlayAsync::run( )
{
	bool bRet = true;
	try
	{
		bRet = _plExiPtr->play( _iceCurrent );
	}
	catch(const Ice::Exception& ex)
	{
		_callback->ice_exception(ex);
		return -1;
	}
	catch(...)
	{
		TianShanIce::ServerError err;
		_callback->ice_exception(err);
		return -1;
	}
	
	_callback->ice_response(bRet);

	return 0;
}

#define AMDCALL( x ) \
amdCallBase* pT = 0;\
try{ \
	pT = new x;\
	pT->start();\
} catch(  const TianShanIce::BaseException& ex) {\
	if( pT) delete pT;\
	callback->ice_exception(ex);\
}\
catch( const std::exception& ) {\
	if( pT) delete pT;\
	callback->ice_exception( TianShanIce::ServerError());\
}

void PlaylistExI::play_async(const ::TianShanIce::Streamer::AMD_Stream_playPtr& callback, const ::Ice::Current& c)
{
	PlaylistExI::Ptr p = this;
	AMDCALL( amdPlayAsync(callback,p,c) );
}

int amdSeekStreamAsync::run()
{
	Ice::Long	lRet = 0;
	try
	{
		lRet = _plExiPtr->seekStream( _offset , _startPos ,_iceCurrent );
	}
	catch(const Ice::Exception& ex )
	{
		_callback->ice_exception(ex);
		return -1;
	}
	catch(...)
	{
		TianShanIce::ServerError err;
		_callback->ice_exception(err);
		return -1;
	}
	_callback->ice_response(lRet);
	return 0;
}
void PlaylistExI::seekStream_async(const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr& callback, ::Ice::Long offset, ::Ice::Int startPos, const ::Ice::Current& c) 
{
	PlaylistExI::Ptr p = this;
	AMDCALL( amdSeekStreamAsync(callback , offset , startPos , p , c ));
}


int amdPlayExAsync::run( )
{
	TianShanIce::Streamer::StreamInfo retInfo;
	try
	{
		retInfo = _plExiPtr->playEx( _newSpeed , _offset , _from , _expectedProps ,_iceCurrent );
	}
	catch( const Ice::Exception& ex)
	{
		_callback->ice_exception(ex);
		return -1;
	}
	catch(...)
	{
		TianShanIce::ServerError err;
		_callback->ice_exception(err);
		return -1;
	}
	_callback->ice_response(retInfo);
	return 0;
}

void PlaylistExI::playEx_async(const ::TianShanIce::Streamer::AMD_Stream_playExPtr& callback, ::Ice::Float newSpeed , ::Ice::Long offset, ::Ice::Short from , const ::TianShanIce::StrValues& expectedProps, const ::Ice::Current& c) 
{
	AMDCALL( amdPlayExAsync( callback , newSpeed , offset, from , expectedProps , this , c));
}

int amdSkipToItemAsync::run( )
{
	bool bRet = true;
	try
	{
		bRet = _plExiPtr->skipToItem( _where , _bPlay , _iceCurrent );
	}
	catch( const Ice::Exception& ex)
	{
		_callback->ice_exception(ex);
		return -1;
	}
	catch(...)
	{
		TianShanIce::ServerError err;
		_callback->ice_exception(err);
		return -1;
	}
	_callback->ice_response(bRet);
	return 0;
}
void PlaylistExI::skipToItem_async(const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr& callback, ::Ice::Int where, bool bPlay, const ::Ice::Current& c) 
{
	AMDCALL(amdSkipToItemAsync( callback , where , bPlay , this , c ));
}

int amdSeekToPositionAsync::run()
{
	bool bRet = true;
	try
	{
		bRet = _plExiPtr->seekToPosition( _userCtrlNum , _timeOffset , _startPos , _iceCurrent );
	}
	catch(const Ice::Exception& ex)
	{
		_callback->ice_exception(ex);
		return -1;
	}
	catch(...)
	{
		TianShanIce::ServerError err;
		_callback->ice_exception(err);
		return -1;
	}
	_callback->ice_response(bRet);

	return 0;
}

void PlaylistExI::seekToPosition_async(const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr& callback, ::Ice::Int UserCtrlNum, ::Ice::Int timeOffset, ::Ice::Int startPos, const ::Ice::Current& c) 
{
	AMDCALL( amdSeekToPositionAsync( callback , UserCtrlNum , timeOffset , startPos , this , c ));
}

int amdPlayItemAsync::run()
{
	TianShanIce::Streamer::StreamInfo retInfo;
	try
	{
		retInfo = _plExiPtr->playItem( _UserCtrlNum , _timeOffset , _from , _newSpeed , _expectedProps , _iceCurrent );
	}
	catch( const Ice::Exception& ex)
	{
		_callback->ice_exception(ex);
		return -1;
	}
	catch(...)
	{
		TianShanIce::ServerError err;
		_callback->ice_exception(err);
		return -1;

	}
	_callback->ice_response(retInfo);
	return 0;
}

void PlaylistExI::playItem_async(const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr& callback, ::Ice::Int UserCtrlNum, ::Ice::Int timeOffset, ::Ice::Short from, ::Ice::Float newSpeed, const ::TianShanIce::StrValues& expectedProps, const ::Ice::Current& c) 
{
	AMDCALL( amdPlayItemAsync(callback , UserCtrlNum , timeOffset ,from , newSpeed , expectedProps , this , c));
}


void PlaylistExI::commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr& admPtr, const ::Ice::Current&/* = ::Ice::Current()*/) 
{	
	LOCAL_ERROR_STRING(return);	
	if(!_pList->commit())
	{
		TianShanIce::ServerError err;
		err.message		= "failed to commit stream";
		err.errorCode	= _pList->lastExtErrorCode();
		admPtr->ice_exception( err );
		return;
	}
	glog(Log::L_INFO ,PLSESSID(commit,"playlist commit") );	
	admPtr->ice_response();
}

//prepare encryption data
//at now ,not only encryptiondata
bool PlaylistExI::translatePreEncryptionData(ZQ::common::Variant& varEcm , TianShanIce::ValueMap& valMap)
{
	TianShanIce::Variant varTianShan ;
	
	if (valMap.find("Item_PID")!=valMap.end()) 
	{
		varTianShan = valMap["Item_PID"];
		if( varTianShan.ints.size() > 0 )
			varEcm.set(VSTRM_ITEM_PID,(unsigned short)(varTianShan.ints[0]));
	}	
	
	//get storage library url if it's exist
	if ( valMap.find("storageLibraryUrl") != valMap.end()   ) 
	{		
		varTianShan = valMap["storageLibraryUrl"];
		if (varTianShan.strs.size() > 0 )
		{
			ZQ::common::Variant urls;
			urls.setSize( static_cast<int>( varTianShan.strs.size() ) );
			TianShanIce::StrValues::const_iterator itURL = varTianShan.strs.begin();
			int iIndex= 0 ;
			for( ; itURL != varTianShan.strs.end() ; itURL ++ )
			{
				urls.set( iIndex , *itURL );
				iIndex ++;
			}
			varEcm[STORAGE_LIBRARY_URL] = urls;
			glog(Log::L_INFO,
				PLSESSID(translatePreEncryptionData,"get storage library URL [%s]"),
				varTianShan.strs[0].c_str() );		
		}
		else
		{
			glog(ZQ::common::Log::L_DEBUG , PLSESSID(translatePreEncryptionData,"no storage library is found"));
		}
	}
	else
	{
		glog(ZQ::common::Log::L_DEBUG , PLSESSID(translatePreEncryptionData,"no storage library is found"));
	}

	if (valMap.find("providerId") != valMap.end() && valMap.find("providerAssetId") != valMap.end() )
	{
		TianShanIce::Variant varProviderId,varAssetId;
		varProviderId.type = TianShanIce::vtStrings;
		varAssetId.type = TianShanIce::vtStrings;
		
		varProviderId	= valMap["providerId"];
		varAssetId		= valMap["providerAssetId"];
		
		if( varProviderId.strs.size() > 0 &&
			!varProviderId.strs[0].empty() &&
			varAssetId.strs.size() > 0 &&
			!varAssetId.strs[0].empty() )
		{
			glog(Log::L_INFO,
				PLSESSID(translatePreEncryptionData,"get providerId[%s] assetId[%s]"),
				varProviderId.strs[0].c_str(),
				varAssetId.strs[0].c_str() );
			varEcm[ITEMDATA_PROVIDERID]		= varProviderId.strs[0];
			varEcm[ITEMDATA_PROVIDERASSETID]= varAssetId.strs[0];
		}
		else
		{
			glog(Log::L_INFO,
				PLSESSID(translatePreEncryptionData,"no providerId and ProviderAssetId"));
		}
	}
	else
	{
		glog(Log::L_INFO,
				PLSESSID(translatePreEncryptionData,"no providerId and ProviderAssetId"));
	}

	

	varTianShan = valMap["Tianshan-ecm-data:preEncryption-Enable"];
	if(! (varTianShan.type == TianShanIce::vtInts && varTianShan.ints.size()>0 && varTianShan.ints[0] == 1) )
	{
		glog(Log::L_INFO,PLSESSID(translatePreEncryptionData,"No pre-encryption data"));
		varEcm[ENCRYPTION_ENABLE] = 0;
		return  true;
	}
	varEcm[ENCRYPTION_ENABLE] = 1;	
	//set vendor
	varEcm[ENCRYPTION_VENDOR] = VSTRM_ENCRYPTION_VENDOR_MOTOROLA;

	//get progNum 
	varTianShan = valMap["Tianshan-ecm-data:programNumber"];
	//calculate PID
	//(((prgId) << 4) + 15) 
	if ( varTianShan.type != TianShanIce::vtInts || varTianShan.ints.size()<=0 ) 
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1011,PLSESSID(translatePreEncryptionData,"Invalid Tianshan-ecm-data:programNumber data ,should be vtInts") );
	}
	int pid = varTianShan.ints[0];
	pid=(pid<<4)+15;
	varEcm.set(ENCRYPTION_ECM_PID, pid);

	int pnOffsetCount  =0; 
	{
		//get programnumber offset
		varTianShan = valMap["Tianshan-ecm-data:keyoffsets"];
		if( varTianShan.type != TianShanIce::vtInts || varTianShan.ints.size()<=0 )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1012,PLSESSID(translatePreEncryptionData,"Invalid Tianshan-ecm-data:keyoffsets data ,should be vtInts"));
		}
		char pnoffsetName[256];
		ZQ::common::Variant varPNOffset;
		for( int i=0 ; i<(int)varTianShan.ints.size() ; i++ )
		{
			sprintf(pnoffsetName,"%s%d",ENCRYPTION_PNOFFSETPREFIX,i);
			varPNOffset.set(pnoffsetName,(int)varTianShan.ints[i]);
		}
		pnOffsetCount = static_cast<int>( varTianShan.ints.size());
		varEcm.set(ENCRYPTION_PNOFFSETPREFIX,varPNOffset);
		varEcm.set(ENCRYPTION_DATACOUNT,pnOffsetCount);
		
	}

	{
		//get key--encryption data
		varTianShan = valMap["Tianshan-ecm-data:keys"];
		if(varTianShan.type != TianShanIce::vtStrings || varTianShan.strs.size()<=0 )		
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1013,PLSESSID(translatePreEncryptionData,"Invalid Tianshan-ecm-data:keys data ,should be vtStrings"));
		}
		
		char keysName[256];
		ZQ::common::Variant varkeys;
		for( int i=0 ; i<(int)varTianShan.strs.size() ; i++ )
		{
			sprintf(keysName,"%s%d",ENCRYPTION_DATAPREFIX,i);
			varkeys.set(keysName,varTianShan.strs[i]);
		}
		varEcm.set(ENCRYPTION_DATAPREFIX,varkeys);
		if (pnOffsetCount!=(int)varTianShan.strs.size()) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1014,PLSESSID(translatePreEncryptionData,"Invalid Tianshan-ecm-data ,key-offset count is not the same as key count"));
		}
	}

	//set cycle and freq
	{
		varTianShan = valMap["Tianshan-ecm-data:Cycle_1"];
		if(varTianShan.type!=TianShanIce::vtInts || varTianShan.ints.size()<=0)
		{
			glog(ZQ::common::Log::L_INFO,PLSESSID(translatePreEncryptionData,"no cycle_1 data,use configuration value [%d]"),gStreamSmithConfig.lEncryptionCycle1);
			varEcm.set(ENCRYPTION_CYCLE1,(int)gStreamSmithConfig.lEncryptionCycle1);
		}
		else
		{
			glog(ZQ::common::Log::L_INFO,PLSESSID(translatePreEncryptionData,"find cycle_1 [%d]"),varTianShan.ints[0]);
			varEcm.set(ENCRYPTION_CYCLE1,(int)varTianShan.ints[0]);
		}

		varTianShan = valMap["Tianshan-ecm-data:Cycle_2"];
		if(varTianShan.type!=TianShanIce::vtInts || varTianShan.ints.size()<=0)
		{
			glog(ZQ::common::Log::L_INFO,PLSESSID(translatePreEncryptionData,"no cycle_2 data,use configuration value [%d]"),gStreamSmithConfig.lEncryptionCycle2);
			varEcm.set(ENCRYPTION_CYCLE2,(int)gStreamSmithConfig.lEncryptionCycle2);
		}
		else
		{
			glog(ZQ::common::Log::L_INFO,PLSESSID(translatePreEncryptionData,"find cycle_2 [%d]"),varTianShan.ints[0]);
			varEcm.set(ENCRYPTION_CYCLE2,(int)varTianShan.ints[0]);
		}

		varTianShan = valMap["Tianshan-ecm-data:Freq_1"];		
		if(varTianShan.type!=TianShanIce::vtInts || varTianShan.ints.size()<=0)
		{
			glog(ZQ::common::Log::L_INFO,PLSESSID(translatePreEncryptionData,"no freq_1 data,use configuration value [%d]"),gStreamSmithConfig.lEncryptionFreq1);
			varEcm.set(ENCRYPTION_FREQ1,(int)gStreamSmithConfig.lEncryptionFreq1);
		}
		else
		{
			glog(ZQ::common::Log::L_INFO,PLSESSID(translatePreEncryptionData,"find freq_1 [%d]"),varTianShan.ints[0]);
			varEcm.set(ENCRYPTION_FREQ1,(int)varTianShan.ints[0]);
		}

		varTianShan = valMap["Tianshan-ecm-data:Freq_2"];
		if(varTianShan.type!=TianShanIce::vtInts || varTianShan.ints.size()<=0)
		{
			glog(ZQ::common::Log::L_INFO,PLSESSID(translatePreEncryptionData,"no freq_2 data,use configuration value [%d]"),gStreamSmithConfig.lEncryptionFreq2);
			varEcm.set(ENCRYPTION_FREQ2,(int)gStreamSmithConfig.lEncryptionFreq1);
		}
		else
		{
			glog(ZQ::common::Log::L_INFO,PLSESSID(translatePreEncryptionData,"find freq_2 [%d]"),varTianShan.ints[0]);
			varEcm.set(ENCRYPTION_FREQ2,(int)varTianShan.ints[0]);
		}
	}	
	return true;
}


::std::string PlaylistExI::GetGuid()
{
	IceUtil::RWRecMutex::RLock sync(*this);

	return attr.Guid;
}
::TianShanIce::Streamer::PlaylistAttr PlaylistExI::getAttr(const ::Ice::Current& ic/*= ::Ice::Current()*/)
{
	IceUtil::RWRecMutex::RLock sync(*this);
	return attr;
}
void PlaylistExI::updateAttr (const ::TianShanIce::Streamer::PlaylistAttr& plAttr, 
							  const ::Ice::Current& /* = ::Ice::Current( */)
{
	IceUtil::RWRecMutex::WLock sync(*this);
	std::string strTemp="";
	strTemp=strTemp;
	try
	{
		guid				=	plAttr.Guid;
		attr.Guid			=	plAttr.Guid;
		attr.StreamSmithSiteName=plAttr.StreamSmithSiteName;
		attr.ResourceGuid	=	plAttr.ResourceGuid;
		attr.ClientSessionID=	plAttr.ClientSessionID;
		attr.endPoint		=	plAttr.endPoint;
		attr.MaxRate		=	plAttr.MaxRate;
		attr.MinRate		=	plAttr.MinRate;
		attr.NowRate		=	plAttr.NowRate;
		attr.destIP			=	plAttr.destIP;
		attr.destMac		=	plAttr.destMac;
		attr.destPort		=	plAttr.destPort;
		attr.vstrmPort		=	plAttr.vstrmPort;
		attr.programNumber	=	plAttr.programNumber;
		attr.playlistState	=	plAttr.playlistState;
		attr.currentCtrlNum	=	plAttr.currentCtrlNum;
		attr.vstrmSessID	=	plAttr.vstrmSessID;
		attr.streamPID		=	plAttr.streamPID;
		attr.property		=	plAttr.property;
	}
	catch (...)
	{
	}
}
//bool PlaylistExI::setProgramNumber(::Ice::Int programNumber, const ::Ice::Current& ic )
void PlaylistExI::allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx& prx, const ::Ice::Current& ic) 
{
	IceUtil::RWRecMutex::RLock sync(*this);
	LOCAL_ERROR_STRING(return);

}

void PlaylistExI::destroy(const ::Ice::Current& c) 
{
	::TianShanIce::Properties feedback; // simply throw away feedback returned from destory2()
	 destroy2(feedback, c);
}

void PlaylistExI::destroy2(::TianShanIce::Properties& feedback, const ::Ice::Current& ic)
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	Ice::Context ctx = ic.ctx;
	glog(Log::L_DEBUG, "playlist[%s] destroy() enter, caller[%s]", guid.c_str(), ctx["caller"].c_str());
	LOCAL_ERROR_STRING(return);

	try
	{
		_pList=StreamSmithSite::m_pPlayListManager->find(ZQ::common::Guid(attr.Guid.c_str()));		
		if (_pList)
			_pList->destroy();
	}
	catch (...)
	{
	}

	glog(Log::L_INFO, "playlist[%s] destroy() leave, caller[%s]", guid.c_str(), ctx["caller"].c_str());
}

::std::string PlaylistExI::lastError(const ::Ice::Current& ic) const
{
	IceUtil::RWRecMutex::RLock sync(*this);
	//LOCAL_ERROR_STRING(return "");
	LOCAL_ERROR_STRING(return "");
	try
	{
		return _pList->lastError();
	}
	catch (...)
	{
		return "";
	}
}
void PlaylistExI::setPID (Ice::Int pid,const Ice::Current& )
{
	LOCAL_ERROR_STRING(return );
	_pList->setStreamPID(pid);
}
::std::string PlaylistExI::getId(const ::Ice::Current& ) const
{
	IceUtil::RWRecMutex::RLock sync(*this);
	LOCAL_ERROR_STRING(return "");
	ZQ::common::Guid	uid=_pList->getId();	
	if(uid.isNil())
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1021,"Can't get playlist UID");
	}
	try
	{
		char	szBuf[128];
		ZeroMemory(szBuf,sizeof(szBuf));
		uid.toString(szBuf,sizeof(szBuf));
		return ::std::string(szBuf);
	}
	catch (...)
	{
		return "";
	}
}

::Ice::Int PlaylistExI::insert(::Ice::Int ctrlNum, const ::TianShanIce::Streamer::PlaylistItemSetupInfo& info,
							   ::Ice::Int whereInsert, const ::Ice::Current& )
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return -1);

		IPlaylist::Item item;
		item._criticalStart=(time_t)info.criticalStart;
		item._currentUserCtrlNum=ctrlNum;
		strncpy(item._fileName,info.contentName.c_str(),254);
		item._flags=(uint32)info.flags;
		item._forceNormal=info.forceNormal;
		item._inTimeOffset=(uint32)info.inTimeOffset;
		item._outTimeOffset=(uint32)info.outTimeOffset;
		item._spliceIn=info.spliceIn;
		item._spliceOut=info.spliceOut;
		item._whereInsert=whereInsert;

		TianShanIce::ValueMap valMap = info.privateData;
		translatePreEncryptionData(item._var,valMap);

		CtrlNum res=_pList->insert(item);
		if(res==INVALID_CTRLNUM)
		{
			switch(_pList->lastErrCode())
			{
			case  ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1031,_pList->lastError());
				break;
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",1032,_pList->lastError());
				break;
			default:
				ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1033,_pList->lastError());
				break;
			}
			
		}
		return res;

	
}
bool PlaylistExI::getInfo(::Ice::Int mask, ::TianShanIce::ValueMap& varOut , const ::Ice::Current&)
{	
	//IceUtil::RWRecMutex::RLock sync(*this);

	LOCAL_ERROR_STRING(return false);

	ZQ::common::Variant var;

	glog(Log::L_DEBUG,"guid[%s]enter get info",guid.c_str());
	long InternalMask=-1;
	switch(mask)
	{
	case TianShanIce::Streamer::infoDVBCRESOURCE:
		InternalMask=IPlaylist::infoDVBCRESOURCE;
		break;
	case TianShanIce::Streamer::infoPLAYPOSITION:
		InternalMask=IPlaylist::infoPLAYPOSITION;
		break;
	case TianShanIce::Streamer::infoSTREAMNPTPOS:
		InternalMask=IPlaylist::infoPLAYNPTPOS;
		break;
	case TianShanIce::Streamer::infoSTREAMSOURCE:
		InternalMask=IPlaylist::infoSTREAMSOURCE;
	default:
		break;
	}
	InternalMask |= 0xF0000000;//make sure this call is come from remote
	if(!_pList->getInfo(InternalMask,var))
		return false;
	switch(mask)
	{
	case TianShanIce::Streamer::infoSTREAMSOURCE:
		{
			if(!var.valid())
				return false;
			TianShanIce::Variant	res;
			varOut.clear();

			res.strs.clear();
			res.bRange = false;
			res.strs.push_back(std::string(var[EventField_StreamSourceIp]));
			res.type = TianShanIce::vtStrings;
			varOut["StreamingSourceIp"] = res;

			res.ints.clear();
			res.bRange = false;
			res.ints.push_back(var[EventField_StreamSourcePort]);
			res.type = TianShanIce::vtInts;
			varOut["StreamingSourcePort"] = res;

		}
		break;
	case TianShanIce::Streamer::infoDVBCRESOURCE:
		{
			//check if the var is right 
			if(!var.valid())
				return false;
			
			TianShanIce::Variant	res;
			varOut.clear();

			///push playlist guid
			res.strs.clear();
			res.bRange = false;
			res.strs.push_back(std::string( var[IPlaylist::RES_GUID] ) );
			res.type=TianShanIce::vtStrings;
			varOut["Guid"]=res;
			
			res.ints.clear();
			res.bRange = false;
			res.ints.push_back(int(var[IPlaylist::RES_FRENQENCY]));
			res.type=TianShanIce::vtInts;
			varOut["Frequency"]=res;

			res.ints.clear();
			res.bRange = false;
			res.ints.push_back(int(var[IPlaylist::RES_PROGRAMNUMBER]));
			res.type=TianShanIce::vtInts;
			varOut["ProgramNumber"]=res;
			
			res.strs.clear();
			res.bRange = false;
			res.strs.push_back(std::string(var[IPlaylist::RES_DESTIP]));
			res.type=TianShanIce::vtStrings;
			varOut["DestIP"]=res;

			res.ints.clear();
			res.bRange = false;
			res.ints.push_back(int(var[IPlaylist::RES_DESTPORT]));
			res.type=TianShanIce::vtInts;
			varOut["DestPort"]=res;
			
			res.ints.clear();
			res.bRange = false;
			res.ints.push_back(int(var[IPlaylist::RES_CHANNEL]));
			res.type=TianShanIce::vtInts;
			varOut["Channel"]=res;

			res.ints.clear();
			res.bRange = false;
			res.ints.push_back(int(var[IPlaylist::RES_QAMMODE]));
			res.type=TianShanIce::vtInts;
			varOut["QamMode"]=res;
		}
		break;
	case TianShanIce::Streamer::infoPLAYPOSITION:
		{
			if(!var.valid())
				return false;

			
			TianShanIce::Variant res;

			res.bRange = false;

			
			res.ints.clear();
			res.ints.push_back(  int(var[EventField_ItemIndex]) );
			res.type=TianShanIce::vtInts;
			varOut["index"]=res;

			res.ints.clear();
			res.ints.push_back(  int(var[EventField_ItemOffset]) );
			res.type=TianShanIce::vtInts;
			varOut["itemOffset"]=res;


			res.ints.clear();
			res.ints.push_back(  int(var[EventField_UserCtrlNum]) );
			res.type=TianShanIce::vtInts;
			varOut["ctrlnumber"]=res;

			res.ints.clear();
			res.bRange = false;
			res.ints.push_back(int(var[EventField_CurrentTimeOffset] ) );
			res.type=TianShanIce::vtInts;
			varOut["playposition"]=res;		
			
			res.ints.clear();
			res.bRange = false;
			res.ints.push_back(int(var[EventField_TotalTimeOffset]));
			res.type=TianShanIce::vtInts;
			varOut["totalplaytime"]=res;

			res.strs.clear();
			res.bRange = false;
			res.strs.push_back((std::string(var[EventField_PlayScale])));
			res.type=TianShanIce::vtStrings;
			varOut["scale"]=res;

		}
		break;
	case TianShanIce::Streamer::infoSTREAMNPTPOS:
		{
			if(!var.valid())
				return false;
			
			TianShanIce::Variant res;

			res.bRange = false;


			res.ints.clear();
			res.ints.push_back(  int(var[EventField_ItemIndex]) );
			res.type=TianShanIce::vtInts;
			varOut["index"]=res;

			res.ints.clear();
			res.ints.push_back(  int(var[EventField_ItemOffset]) );
			res.type=TianShanIce::vtInts;
			varOut["itemOffset"]=res;

			res.strs.clear();
			res.strs.push_back(  std::string(var[EventField_PlaylistGuid]) );
			res.type=TianShanIce::vtStrings;
			varOut["playlistid"]=res;
			
			res.ints.clear();
			res.bRange = false;
			res.ints.push_back(int(var[EventField_CurrentTimeOffset] ) );
			res.type=TianShanIce::vtInts;
			varOut["playposition"]=res;		
			
			res.ints.clear();
			res.bRange = false;
			res.ints.push_back( int(var[EventField_PreviousePlayPosPrimary]));
			res.type = TianShanIce::vtInts;
			varOut["PreviousPlayPosPrimary"]=res;

			res.ints.clear();
			res.bRange = false;
			res.ints.push_back(int(var[EventField_TotalTimeOffset]));
			res.type=TianShanIce::vtInts;
			varOut["totalplaytime"]=res;

			res.strs.clear();
			res.bRange = false;
			res.strs.push_back((std::string(var[EventField_PlayScale])));
			res.type=TianShanIce::vtStrings;
			varOut["scale"]=res;			
		}
		break;
	default:
		glog(Log::L_ERROR,"invalid mask =%d when get info",mask);
		break;
	}
	glog(Log::L_DEBUG,"guid[%s] leave getinfo",guid.c_str());
	return true;
}
bool PlaylistExI::allocDVBCResource(::Ice::Int serviceGroupID, ::Ice::Int bandWidth, const ::Ice::Current& )
{
	IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return false);
	
		ZQ::common::Variant var;
		if(!_pList->allocateResource(serviceGroupID,var,bandWidth))
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1041,"allocDVBCResource() Not enough minerals");
			return false;
		}
		return true;

}
void PlaylistExI::setPLaylistData(const TianShanIce::ValueMap& valMap ,  const ::Ice::Current&/* = ::Ice::Current()*/ )
{
	LOCAL_ERROR_STRING(return);
	std::string		pokeHoleSessId;
	ZQTianShan::Util::getValueMapDataWithDefault( valMap ,"PokeSessionID","", pokeHoleSessId );
	bool bRet = true;
	if( !pokeHoleSessId.empty() )
	{
		bRet = _pList->setPokeHoleSessionID( pokeHoleSessId );
	}
	_pList->setPlaylistAttributes( valMap);
	if (!bRet) 
	{
		switch(_pList->lastErrCode())
		{
		case  ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1052,_pList->lastError());
			break;
		case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",1053,_pList->lastError());
			break;
		case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_SERVER_ERROR:
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1054,_pList->lastError());
			break;
		default:
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1055,_pList->lastError());
			break;
		}
	}
}
void PlaylistExI::setSourceStrmPort(::Ice::Int port, const ::Ice::Current&/* = ::Ice::Current()*/)
{
	/*IceUtil::RWRecMutex::WLock sync(*this);*/
	LOCAL_ERROR_STRING(return);
	_pList->setStreamServPort((unsigned short)port);
}

Ice::Int PlaylistExI::pushBackEx(::Ice::Int baseCtrlNum,
								 const ::TianShanIce::Streamer::PlaylistItemSetupInfoS& newItems,
								 const ::Ice::Current& c)
{
	Ice::Int	iCount = static_cast<Ice::Int>(newItems.size());
	for( Ice::Int i = 0 ; i < iCount ; i ++  )
	{
		pushBack(baseCtrlNum + i , newItems[i] , c) ;
	}
	return baseCtrlNum;
}
::Ice::Int PlaylistExI::pushBack(::Ice::Int UserctrlNum, const ::TianShanIce::Streamer::PlaylistItemSetupInfo& info,
								const ::Ice::Current& )
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return -1);

		IPlaylist::Item item;
		item._criticalStart=(time_t)info.criticalStart;
		item._currentUserCtrlNum=UserctrlNum;
		strncpy(item._fileName,info.contentName.c_str(),255);
		item._flags=(uint32)info.flags;
		item._forceNormal=info.forceNormal;
		item._inTimeOffset=(uint32)info.inTimeOffset;
		item._outTimeOffset=(uint32)info.outTimeOffset;
		item._spliceIn=info.spliceIn;
		item._spliceOut=info.spliceOut;
		item._whereInsert=0;
		//#ifdef _DEBUG_TEST
		//	_pList->setDestMac("1:2:3:4:6:5");	
		//#endif
		TianShanIce::ValueMap valMap = info.privateData;
		translatePreEncryptionData(item._var,valMap);
		CtrlNum res=_pList->push_back(item);
		if(res==INVALID_CTRLNUM)
		{
			switch(_pList->lastErrCode())
			{
			case  ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1061,_pList->lastError());
				break;
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",1071,_pList->lastError());
				break;
			default:
				ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1081,_pList->lastError());
				break;
			}
		}
		return res;
	
}

::Ice::Int PlaylistExI::size(const ::Ice::Current& ) const
{
	IceUtil::RWRecMutex::RLock sync(*this);
	//LOCAL_ERROR_STRING(return -1);
	LOCAL_ERROR_STRING(return -1);
	try
	{
		return _pList->size();
	}
	catch (...)
	{
		return -1;
	}
}

::Ice::Int PlaylistExI::left(const ::Ice::Current&) const
{
	IceUtil::RWRecMutex::RLock sync(*this);
	//LOCAL_ERROR_STRING(return -1);
	LOCAL_ERROR_STRING(return -1);
	try
	{
		return _pList->left();
	}
	catch (...)
	{
		return -1;
	}
}

bool PlaylistExI::empty(const ::Ice::Current& )const
{
	IceUtil::RWRecMutex::RLock sync(*this);
	LOCAL_ERROR_STRING(return false);
	try
	{
		return _pList->empty();
	}
	catch (...)
	{
		return false;
	}
}

::Ice::Int PlaylistExI::current(const ::Ice::Current&) const
{
	IceUtil::RWRecMutex::RLock sync(*this);	
	LOCAL_ERROR_STRING(return -1);
	try
	{
		return _pList->current();
	}
	catch (...)
	{
		return -1;
	}
}

void PlaylistExI::erase(::Ice::Int where, const ::Ice::Current& ) 
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	//LOCAL_ERROR_STRING(return);
	LOCAL_ERROR_STRING(return);	

		if(INVALID_CTRLNUM==_pList->erase(where))
		{//something wrong,just throw a expcetion
			switch(_pList->lastErrCode())
			{
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",1091,_pList->lastError());
				break;
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1092,_pList->lastError());
				break;
			default:
				ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1093,_pList->lastError());
				break;
			}
		}

}

::Ice::Int PlaylistExI::flushExpired(const ::Ice::Current& )
{
	IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return -1);
	try
	{
		return _pList->flush_expired();
	}
	catch (...)
	{
		return -1;
	}
}

bool PlaylistExI::clearPending(bool, const ::Ice::Current&)
{
	IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return false);
	try
	{
		return _pList->clear_pending();
	}
	catch (...) 
	{
		return false;
	}
}

bool PlaylistExI::isCompleted(const ::Ice::Current& )
{
	IceUtil::RWRecMutex::RLock sync(*this);
	LOCAL_ERROR_STRING(return true);
	try
	{
		return _pList->isCompleted();
	}
	catch (...)
	{
		return true;
	}
}

::Ice::Int PlaylistExI::findItem(::Ice::Int ctrlNum, ::Ice::Int from, const ::Ice::Current& )
{
	IceUtil::RWRecMutex::RLock sync(*this);
	LOCAL_ERROR_STRING(return -1);
	try
	{
		return _pList->findItem(ctrlNum,from);
	}
	catch (...)
	{
		return -1;
	}
}

bool PlaylistExI::distance(::Ice::Int to, ::Ice::Int from, ::Ice::Int& dist, const ::Ice::Current& )
{
	IceUtil::RWRecMutex::RLock sync(*this);
	LOCAL_ERROR_STRING(return false);
	try
	{
		return _pList->distance(&dist,to,from);
	}
	catch (...)
	{
		return false;
	}
}
#define STRSWITCH() if(0){
#define STRCASE(x)	} else if(::strncmp( it->c_str() , x ,strlen(x) ) == 0 ){
#define STRENDCASE() }

std::string	PlaylistExI::dumpExpectProps(  const ::TianShanIce::StrValues& props, ULONG& flag ) const
{
	char	szBuf[1024] = {0};	
	int		iPos = 0;
	int		iSize = sizeof(szBuf) - 1;
	char* pTemp =szBuf;
	TianShanIce::StrValues::const_iterator it = props.begin();
	for( ; it != props.end() ; it ++ )
	{
		iPos = snprintf( pTemp  , iSize , " %s " , it->c_str( ) );
		iSize -= iPos;
		pTemp += iPos;
		STRSWITCH()
		STRCASE("CURRENTPOS")
			flag |=	GET_NPT_CURRENTPOS;
		STRCASE("ITEM_CURRENTPOS")
			flag |= GET_ITEM_CURRENTPOS;
		STRCASE("TOTALPOS")
			flag |= GET_NPT_TOTALPOS;
		STRCASE("ITEM_TOTALPOS")
			flag |= GET_ITEM_TOTALPOS;
		STRCASE("SPEED")
			flag |= GET_SPEED;
		STRCASE("STATE")
			flag |= GET_STATE;
		STRCASE("USERCTRLNUM")
			flag |= GET_USERCTRLNUM;
		STRENDCASE()
	}
	
	return szBuf;

}
::TianShanIce::Streamer::StreamInfo PlaylistExI::playItem(::Ice::Int UserCtrlNum, ::Ice::Int offset,
											 ::Ice::Short from, ::Ice::Float newSpeed, 
											 const ::TianShanIce::StrValues& expectedProps, 
											 const ::Ice::Current& c/*= ::Ice::Current()*/)
{
	::TianShanIce::Streamer::StreamInfo		result;
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return result);
	StreamControlResultInfo info;
	info.extraProperties = c.ctx;
	ZQTianShan::Util::mergeProperty( result.props , c.ctx );

		glog( ZQ::common::Log::L_DEBUG , "guid[%s] Enter playItem with ctrlNum [%u] speed[%f] offset[%d] from[%d]: %s",
			ident.name.c_str(), UserCtrlNum, newSpeed , offset , (int)from,
			dumpExpectProps( expectedProps , info.flag ).c_str() );

		if( !_pList->exPlayItem( UserCtrlNum, newSpeed ,static_cast<LONG>( offset ), from , info) )
		{
			switch(_pList->lastErrCode())
			{
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",1101,_pList->lastError());
				break;
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1102,_pList->lastError());
				break;
			default:
				ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1103,_pList->lastError());
				break;
			}
			return result;
		}

		switch(info.plState) 
		{
		case IPlaylist::PLAYLIST_SETUP:
			result.state	= TianShanIce::Streamer::stsSetup;
			break;
		case IPlaylist::PLAYLIST_PLAY:
			result.state	= TianShanIce::Streamer::stsStreaming;
			break;
		case IPlaylist::PLAYLIST_PAUSE:
			result.state	= TianShanIce::Streamer::stsPause;
			break;
		case IPlaylist::PLAYLIST_STOP:
			result.state	= TianShanIce::Streamer::stsStop;
			break;
		default:
			result.state	= TianShanIce::Streamer::stsStop;
			break;
		}
		char szBuf[128];
		if( info.flag & GET_NPT_CURRENTPOS )
		{
			sprintf( szBuf , "%u" , info.timeOffset);
			result.props["CURRENTPOS"]	=	szBuf;
		}
		if( info.flag & GET_ITEM_CURRENTPOS )
		{
			sprintf( szBuf , "%u" , info.itemTimeOffset );
			result.props["ITEM_CURRENTPOS"] = szBuf;
		}

		if( info.flag & GET_NPT_TOTALPOS )
		{
			sprintf( szBuf ,"%d" , info.totalOffset );
			result.props["TOTALPOS"] = szBuf;
		}
		if ( info.flag & GET_ITEM_TOTALPOS )
		{
			sprintf( szBuf,"%u" ,info.itemTotalOffset );
			result.props["ITEM_TOTALPOS"] = szBuf;
		}

		if( info.flag & GET_SPEED )
		{
			sprintf( szBuf , "%f" , info.speed );
			result.props["SPEED"] = szBuf;
		}
		if( info.flag & GET_USERCTRLNUM )
		{
			sprintf( szBuf ,"%u" , info.userCtrlNum );
			result.props["USERCTRLNUM"] = szBuf;
		}
		result.ident = ident;

		return result;

}
TianShanIce::Streamer::StreamInfo PlaylistExI::playEx(::Ice::Float newSpeed , ::Ice::Long offset ,
													  ::Ice::Short from, const ::TianShanIce::StrValues& expectedProps, 
													  const ::Ice::Current& c/*= ::Ice::Current()*/)
{
	TianShanIce::Streamer::StreamInfo result;
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return result);
		
	StreamControlResultInfo info;
	info.extraProperties = c.ctx;
	
	ZQTianShan::Util::mergeProperty( result.props , c.ctx );
		
	glog( ZQ::common::Log::L_DEBUG , "guid[%s] Enter playEx with speed[%f] offset[%lld] from[%d]: %s",
			ident.name.c_str(), newSpeed , offset , (int)from,
			dumpExpectProps( expectedProps , info.flag ).c_str() );
		
		if( !_pList->exPlay( newSpeed ,static_cast<ULONG>( offset ), from , info) )
		{
			switch(_pList->lastErrCode())
			{
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",_pList->lastExtErrorCode(),_pList->lastError());
				break;
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",_pList->lastExtErrorCode(),_pList->lastError());
				break;
			default:
				ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",_pList->lastExtErrorCode(),_pList->lastError());
				break;
			}
			return result;
		}
		
		switch(info.plState) 
		{
		case IPlaylist::PLAYLIST_SETUP:
			result.state	= TianShanIce::Streamer::stsSetup;
			break;
		case IPlaylist::PLAYLIST_PLAY:
			result.state	= TianShanIce::Streamer::stsStreaming;
			break;
		case IPlaylist::PLAYLIST_PAUSE:
			result.state	= TianShanIce::Streamer::stsPause;
			break;
		case IPlaylist::PLAYLIST_STOP:
			result.state	= TianShanIce::Streamer::stsStop;
			break;
		default:
			result.state	= TianShanIce::Streamer::stsStop;
			break;
		}
		char szBuf[128];
		if( info.flag & GET_NPT_CURRENTPOS )
		{
			sprintf( szBuf , "%u" , info.timeOffset);
			result.props["CURRENTPOS"]	=	szBuf;
		}
		
		if( info.flag & GET_ITEM_CURRENTPOS )
		{
			sprintf( szBuf , "%u" , info.itemTimeOffset );
			result.props["ITEM_CURRENTPOS"] = szBuf;
		}

		if( info.flag & GET_NPT_TOTALPOS )
		{
			sprintf( szBuf ,"%d" , info.totalOffset );
			result.props["TOTALPOS"] = szBuf;
		}
		
		if ( info.flag & GET_ITEM_TOTALPOS )
		{
			sprintf( szBuf,"%u" ,info.itemTotalOffset );
			result.props["ITEM_TOTALPOS"] = szBuf;
		}

		if( info.flag & GET_SPEED )
		{
			sprintf( szBuf , "%f" , info.speed );
			result.props["SPEED"] = szBuf;
		}
		if( info.flag & GET_USERCTRLNUM )
		{
			sprintf( szBuf ,"%u" , info.userCtrlNum );
			result.props["USERCTRLNUM"] = szBuf;
		}
		result.ident = ident;

		return result;
}

TianShanIce::Streamer::StreamInfo PlaylistExI::pauseEx(const ::TianShanIce::StrValues& expectedProps, const ::Ice::Current&/* = ::Ice::Current()*/) 
{
	TianShanIce::Streamer::StreamInfo result;
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return result);
	
		StreamControlResultInfo info;

		glog( ZQ::common::Log::L_DEBUG , "guid[%s] Enter pauseEx flag[%s]",
			ident.name.c_str(), 
			dumpExpectProps( expectedProps , info.flag ).c_str() );

		if( !_pList->exPause( info ) )
		{
			switch(_pList->lastErrCode())
			{
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",1111,_pList->lastError());
				break;
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1112,_pList->lastError());
				break;
			default:
				ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1113,_pList->lastError());
				break;
			}
			return result;
		}

		switch(info.plState) 
		{
		case IPlaylist::PLAYLIST_SETUP:
			result.state	= TianShanIce::Streamer::stsSetup;
			break;
		case IPlaylist::PLAYLIST_PLAY:
			result.state	= TianShanIce::Streamer::stsStreaming;
			break;
		case IPlaylist::PLAYLIST_PAUSE:
			result.state	= TianShanIce::Streamer::stsPause;
			break;
		case IPlaylist::PLAYLIST_STOP:
			result.state	= TianShanIce::Streamer::stsStop;
			break;
		default:
			result.state	= TianShanIce::Streamer::stsStop;
			break;
		}
		char szBuf[128];
		if( info.flag & GET_NPT_CURRENTPOS )
		{
			sprintf( szBuf , "%u" , info.timeOffset);
			result.props["CURRENTPOS"]	=	szBuf;
		}
		
		if( info.flag & GET_ITEM_CURRENTPOS )
		{
			sprintf( szBuf , "%u" , info.itemTimeOffset );
			result.props["ITEM_CURRENTPOS"] = szBuf;
		}

		if( info.flag & GET_NPT_TOTALPOS )
		{
			sprintf( szBuf ,"%d" , info.totalOffset );
			result.props["TOTALPOS"] = szBuf;
		}
		
		if ( info.flag & GET_ITEM_TOTALPOS )
		{
			sprintf( szBuf,"%u" ,info.itemTotalOffset );
			result.props["ITEM_TOTALPOS"] = szBuf;
		}

		if( info.flag & GET_SPEED )
		{
			sprintf( szBuf , "%f" , info.speed );
			result.props["SPEED"] = szBuf;
		}
		if( info.flag & GET_USERCTRLNUM )
		{
			sprintf( szBuf ,"%u" , info.userCtrlNum );
			result.props["USERCTRLNUM"] = szBuf;
		}
		result.ident = ident;

		return result;
}
  

bool PlaylistExI::play(const ::Ice::Current&)
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return false);
		StreamControlResultInfo info;
		if( !_pList->exPlay( 1.0f , 0 , 0 , info ) )
		{
			switch(_pList->lastErrCode())
			{
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",0,_pList->lastError());
				break;
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",0,_pList->lastError());
				break;
			default:
				ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",0,_pList->lastError());
				break;
			}
			return false;
		}
		else
		{
			return true;
		}

}

bool PlaylistExI::setSpeed(::Ice::Float newSpeed, const ::Ice::Current& )
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return false);

		if(!_pList->setSpeed(newSpeed))
		{
			switch(_pList->lastErrCode())
			{
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",1141,_pList->lastError());
				break;
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1142,_pList->lastError());
				break;
			default:
				ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1143,_pList->lastError());
				break;
			}
			return false;
		}
		else
		{
			return true;
		}

}

bool PlaylistExI::pause(const ::Ice::Current& )
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return false);
	
		StreamControlResultInfo info;
		if( !_pList->exPause( info ) )
		{
			switch(_pList->lastErrCode())
			{
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",1151,_pList->lastError());
				break;
			case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1152,_pList->lastError());
				break;
			default:
				ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1153,_pList->lastError());
				break;
			}
			return false;
		}
		else
		{
			return true;
		}
}

bool PlaylistExI::resume(const ::Ice::Current&)
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return false);
	
	StreamControlResultInfo info;
	if( !_pList->exPlay( 0.0f , 0, 0, info ) )
	{
		switch(_pList->lastErrCode())
		{
		case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",0,_pList->lastError());
			break;
		case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",0,_pList->lastError());
			break;
		default:
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",0,_pList->lastError());
				break;
		}
		return false;
	}
	else
	{
		return true;
	}

}

bool PlaylistExI::skipToItem(::Ice::Int where, bool bPlay, const ::Ice::Current& )
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return false);
	try
	{
		return _pList->skipToItem(where,bPlay);
	}
	catch (...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1171,_pList->lastError());
		return false;
	}
}

::TianShanIce::Streamer::StreamState PlaylistExI::getCurrentState(const ::Ice::Current&) const
{
	IceUtil::RWRecMutex::RLock sync(*this);
	//LOCAL_ERROR_STRING(return TianShanIce::Streamer::stsSetup);
	LOCAL_ERROR_STRING(return TianShanIce::Streamer::stsSetup);
	glog(Log::L_INFO,PLSESSID(getCurrentState,"Enter getCurrentState"));

	IPlaylist::State st;
	try
	{
		st=_pList->getCurrentState();
		switch(st)
		{
		case IPlaylist::PLAYLIST_SETUP:
			return TianShanIce::Streamer::stsSetup;
		case IPlaylist::PLAYLIST_PLAY:
			return TianShanIce::Streamer::stsStreaming;
		case IPlaylist::PLAYLIST_PAUSE:
			return TianShanIce::Streamer::stsPause;
		case IPlaylist::PLAYLIST_STOP:
			return TianShanIce::Streamer::stsStop;			 
		default:
			return TianShanIce::Streamer::stsStop;
		}
	}
	catch (...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1181,"Can't get current stream state");
		return TianShanIce::Streamer::stsStop;
	}

}

::TianShanIce::SRM::SessionPrx PlaylistExI::getSession(const ::Ice::Current& )
{
	IceUtil::RWRecMutex::RLock sync(*this);
	LOCAL_ERROR_STRING(return NULL);
	if(_sessionPrx==NULL)
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1191,"NULL session attach to this playlist");
	return _sessionPrx;
}

void PlaylistExI::attachSession(const ::TianShanIce::SRM::SessionPrx& prx, const ::Ice::Current&)
{
	IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return);
	
	_sessionPrx=prx;
}
::Ice::Identity PlaylistExI::getIdent(const ::Ice::Current& ic) const
{
	IceUtil::RWRecMutex::RLock sync(*this);
	return ident;
}

void PlaylistExI::setDestination(const ::std::string& strIP, ::Ice::Int port, const ::Ice::Current& /* = ::Ice::Current( */)
{
	/*IceUtil::RWRecMutex::WLock sync(*this);	*/
	LOCAL_ERROR_STRING(return);
	_pList->setDestination((char*)strIP.c_str(),port);
}
bool PlaylistExI::seekToPosition(::Ice::Int cNum, ::Ice::Int timeOffset,::Ice::Int startPos, const ::Ice::Current& ic/* = ::Ice::Current( */)
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return false);
	IPlaylist::SeekStartPos pos;
	switch(startPos)
	{
	case 0:
		pos=IPlaylist::SEEK_POS_CUR;
		break;
	case 1:
		pos=IPlaylist::SEEK_POS_BEGIN;
		break;
	case 2:
		pos=IPlaylist::SEEK_POS_END;
		break;
	default:
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",1201,"invalid parameter of startPos");
		}
		break;
	}
	bool ret = _pList->SeekTo(cNum,timeOffset,pos);
	if(!ret)
	{
		switch(_pList->lastErrCode())
		{
		case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1202,_pList->lastError());
			break;
		case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_SERVER_ERROR:
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1203,_pList->lastError());
			break;
		default:
			break;
		}
	}
	return ret;
}

void PlaylistExI::setDestMac(const ::std::string& strDestMac, const ::Ice::Current& ic /* = ::Ice::Current( */)
{
	IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return);
	_pList->setDestMac((char*)strDestMac.c_str());
}
void PlaylistExI::setMuxRate(::Ice::Int nowRate, ::Ice::Int maxRate, ::Ice::Int minRate, const ::Ice::Current& ic/* = ::Ice::Current( */)
{
	/*IceUtil::RWRecMutex::WLock sync(*this);*/
	LOCAL_ERROR_STRING(return);
	_pList->setMuxRate(nowRate,maxRate,minRate);
}
void PlaylistExI::setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask& mask, 
										const ::TianShanIce::Streamer::ConditionalControlPrx& prx, 
										const ::Ice::Current& ic/* = ::Ice::Current( */)
{
	IceUtil::RWRecMutex::WLock sync(*this);
	_conditionCtrlPrx=prx;
	_conditionMask=mask;
}
bool PlaylistExI::ActionConfirm(int Action)
{	
#pragma message(__MSGLOC__"When can I make this call in other function")
	IceUtil::RWRecMutex::WLock sync(*this);
	::TianShanIce::Streamer::ConditionalControlMask		requestMask;
	requestMask.changeSpeedBackward=TianShanIce::Streamer::sccDeny;
	requestMask.changeSpeedForward=TianShanIce::Streamer::sccDeny;;
	requestMask.pause=TianShanIce::Streamer::sccDeny;
	requestMask.play=TianShanIce::Streamer::sccDeny;
	requestMask.seek=TianShanIce::Streamer::sccDeny;
	switch(Action)
	{
	case ACTION_PLAY:
		requestMask.play=TianShanIce::Streamer::sccUndefined;
		return InternalActionConfirm(_conditionMask.play,requestMask);
		break;
	case ACTION_PAUSE:
		requestMask.pause=TianShanIce::Streamer::sccUndefined;
		return InternalActionConfirm(_conditionMask.pause,requestMask);
		break;
	case ACTION_CHANGESPEEDFORWARD:
		requestMask.changeSpeedForward=TianShanIce::Streamer::sccUndefined;
		return InternalActionConfirm(_conditionMask.changeSpeedForward,requestMask);
		break;
	case ACTION_CHANGESPEEDBACKWARD:
		requestMask.changeSpeedBackward=TianShanIce::Streamer::sccUndefined;
		return InternalActionConfirm(_conditionMask.changeSpeedBackward,requestMask);
		break;
	case ACTION_SEEK:
		requestMask.seek=TianShanIce::Streamer::sccUndefined;
		return InternalActionConfirm(_conditionMask.seek,requestMask);
		break;
	default:
		//不需要审查
		return true;
		break;
	}
}
bool PlaylistExI::InternalActionConfirm(::TianShanIce::Streamer::ConditionalControlOption option,
										::TianShanIce::Streamer::ConditionalControlMask& requestMask)
{
	if(option==TianShanIce::Streamer::sccDeny)
		return false;
	else if(option==TianShanIce::Streamer::sccApprove)
		return true;
	else if(option==TianShanIce::Streamer::sccAuthorize)
	{
		try
		{
			TianShanIce::Streamer::StreamPrx stream=TianShanIce::Streamer::StreamPrx::uncheckedCast( _objApdater->createProxy(ident) );
			return _conditionCtrlPrx->authorize(stream,requestMask)==TianShanIce::Streamer::sccApprove;
		}
		catch (...) 
		{
			return false;				 
		}

	}
	return false;
}

void PlaylistExI::attachClientSessionId(const ::std::string& clientSessId, const ::Ice::Current& /*= ::Ice::Current()*/)
{
	LOCAL_ERROR_STRING(return);
	_pList->setUserCtxIdx(clientSessId.c_str() );
}
void PlaylistExI::enableEoT(bool bEnable,const ::Ice::Current&)
{
	IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return);
	_pList->enableEoT(bEnable);
}
void PlaylistExI::setPathTicketProxy(const ::TianShanIce::Transport::PathTicketPrx& ticketProxy, 
									 const ::TianShanIce::Streamer::InternalPlaylistExPrx& plExPrx,
									 const ::Ice::Current& ic/*= ::Ice::Current()*/)
{
	/*IceUtil::RWRecMutex::WLock sync(*this);*/
	LOCAL_ERROR_STRING(return);
	_pathTicketPrx=ticketProxy;
	if(_objApdater==NULL)
		_objApdater=ZQ::StreamSmith::StreamSmithSite::m_pPlayListManager->m_Adapter;
	Ice::CommunicatorPtr comm=_objApdater->getCommunicator();
	if(!comm)
	{
		return;
	}
	glog(ZQ::common::Log::L_DEBUG,PLSESSID(setPathTicketProxy,"convert playlistex proxyToString"));
	std::string	strPlEx =  comm->proxyToString(plExPrx);
	glog(ZQ::common::Log::L_DEBUG,PLSESSID(setPathTicketProxy,"convert playlistex proxyToString ok [%s]"),strPlEx.c_str());
	
	std::string strTicket = "";
	if( _pathTicketPrx != NULL )
	{
		strTicket = comm->proxyToString(_pathTicketPrx);
		glog(ZQ::common::Log::L_INFO ,
			PLSESSID(setPathTicketProxy,"convert ticket proxy to string[%s]" ),
			strTicket.c_str() );
	}	
	_pList->setPlaylistExProxy( strPlEx ,strTicket );
}
TianShanIce::IValues PlaylistExI::getSequence(const ::Ice::Current& ) const
{
	TianShanIce::IValues	vecRet;
	IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return vecRet);
	std::vector<ULONG> vRet=_pList->getItemSet();
	char	szBuf[1024];
	ZeroMemory(szBuf,sizeof(szBuf));
	char* pBuf= szBuf;
	int iBufLen =sizeof(szBuf)-1;
	
	int iPos = sprintf(pBuf,"[Playlist] SESSID(%s)Thread[%10u][%16s]\treturn: ",
						attr.Guid.c_str(),GetCurrentThreadId(),"getSequence");
	pBuf += iPos;
	iBufLen -= iPos;
	for(int i=0;i<(int)vRet.size();i++)
	{
		vecRet.push_back(vRet[i]);
		if (iBufLen>0) 
		{
			iPos = _snprintf(pBuf,iBufLen," item[%d]",vRet[i]);
			if (iPos < 0) 
			{
				iBufLen = 0;//no more try
			}
			else
			{				
				iBufLen -= iPos;
				pBuf += iPos;
			}
		}		
	}
	glog(Log::L_DEBUG,"%s",szBuf);
	return vecRet;
}


TianShanIce::Streamer::PlaylistItemSetupInfoS PlaylistExI::getPlaylistItems(const ::Ice::Current& ) const {
	TianShanIce::Streamer::PlaylistItemSetupInfoS items;
	return items;
}

::TianShanIce::SRM::ResourceMap PlaylistExI::getResources(const ::Ice::Current& c) const
{
#pragma message(__MSGLOC__"TODO: impl here")
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented>("Playlist", 1001, "getResources()");
	::TianShanIce::SRM::ResourceMap res;
	return res;
}


::Ice::Long PlaylistExI::seekStream(Ice::Long offset,::Ice::Int startPos,const Ice::Current&)
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	LOCAL_ERROR_STRING(return -1);
	glog(Log::L_DEBUG,PLSESSID(PlaylistExI,"enter seekStream"));
	
	StreamControlResultInfo info;
	info.flag = GET_NPT_CURRENTPOS;
	bool ret = _pList->exPlay( 0.0f , static_cast<ULONG>( offset ) , startPos , info );
	if( !ret )
	{
		switch(_pList->lastErrCode())
		{
		case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALIDSTATE:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Playlist",1212,_pList->lastError());
			break;
		case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_SERVER_ERROR:
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("Playlist",1213,_pList->lastError());
			break;
		case ZQ::StreamSmith::Playlist::ERR_PLAYLIST_INVALID_PARA:
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Playlist",1214,_pList->lastError());
			break;
		default:
			break;
		}
	}
	return info.timeOffset;
}
void PlaylistExI::renewPathTicket(const std::string& ticketProxy ,::Ice::Int iTime,const ::Ice::Current& ic)
{
	
	if(iTime<0)
	{
	}
	
	{
		try
		{
			::TianShanIce::Transport::PathTicketPrx	ticketPrx;
			if ( !ticketProxy.empty() )
			{
				
				try
				{					
					ticketPrx =::TianShanIce::Transport::PathTicketPrx::uncheckedCast(_objApdater->getCommunicator()->stringToProxy(ticketProxy));
				}
				catch(...)
				{
					ticketPrx = NULL;
				}				
			}
			if( ticketPrx != NULL )
			{
				ticketPrx->renew(iTime);
				glog(Log::L_DEBUG,PLSESSID(renewPathTicket,"renew with time =%d with proxy is %s"),
					iTime,_objApdater->getCommunicator()->proxyToString(ticketPrx).c_str());
			}
			renewStatusReport(RENEW_OK);
		}
		catch (const ::TianShanIce::InvalidParameter&) 
		{
			glog(Log::L_DEBUG,PLSESSID(renewPathTicket,"invalid renew time =%d"),iTime);
		}
		catch( const IceUtil::NullHandleException& ex )
		{
			glog(Log::L_DEBUG,PLSESSID(renewPathTicket,"renew fail with renew time =%d and error is %s"),iTime,ex.ice_name().c_str());
			renewStatusReport(RENEW_NOOBJECT);
		}
		catch(const Ice::ObjectNotExistException& ex)
		{
			glog(Log::L_DEBUG,PLSESSID(renewPathTicket,"renew fail with renew time =%d and error is %s"),iTime,ex.ice_name().c_str());
			renewStatusReport(RENEW_NOOBJECT);
		}
		catch (const Ice::ConnectionLostException&) 
		{
			renewStatusReport(RENEW_NOCONNECT);
		}
		catch (const Ice::ConnectionRefusedException& ) 
		{
			renewStatusReport(RENEW_NOCONNECT);
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_DEBUG,PLSESSID(renewPathTicket,"renew failed with ice exception is %s"),ex.ice_name().c_str());
		}
		catch(...)
		{
			glog(Log::L_ERROR,PLSESSID(renewPathTicket,"renew fail with time =%d and unexpect error"),iTime);
			renewStatusReport(RENEW_NOCONNECT);
		}
	}
}
void PlaylistExI::renewStatusReport(int status)
{
	//IceUtil::RWRecMutex::WLock sync(*this);
	switch(status)
	{
	case RENEW_OK:
		{
			_bPlaylistShouldDestroy=false;
		}
		break;
	case RENEW_NOOBJECT:
		{
			glog(Log::L_INFO,PLSESSID(renewStatusReport,"destroy playlist because ticket is not exist"));
			this->destroy();
		}
		break;
	case RENEW_NOCONNECT:
		{
			glog(Log::L_WARNING,PLSESSID(renewStatusReport,"connection lost when renew ticket"));
//			if(_bPlaylistShouldDestroy)
//			{
//				glog(Log::L_DEBUG,PLSESSID(renewStatusReport,"destroy playlist %s because connection is lost"),guid.c_str());
//				this->destroy();
//			}
//			else
//			{
//				glog(Log::L_DEBUG,PLSESSID(renewStatusReport,"connection is lost when renew ticket"));
//				_bPlaylistShouldDestroy=true;
//			}
		}
		break;
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
//RenewTicketRequest

RenewTicketRequest::RenewTicketRequest(ZQ::common::NativeThreadPool& pool,
									   std::string& plePrxstr,
									   std::string& ticketPrx,
										ZQADAPTER_DECLTYPE objAdpter,
									   int time):
ZQ::common::ThreadRequest(pool),_strPlexPrx(plePrxstr),_objAdpter(objAdpter),_strTicketPrx(ticketPrx)
{
	renewTime=time;
}
RenewTicketRequest::~RenewTicketRequest()
{
}

void RenewTicketRequest::final(int retcode , bool bCancelled )
{
	delete this;
}

int RenewTicketRequest::run()
{
	try
	{
		if(!_strPlexPrx.empty())
		{
			Ice::CommunicatorPtr ic=_objAdpter->getCommunicator();
			TianShanIce::Streamer::InternalPlaylistExPrx prx=TianShanIce::Streamer::InternalPlaylistExPrx::checkedCast(ic->stringToProxy(_strPlexPrx));
			prx->renewPathTicket( _strTicketPrx , renewTime );
		}
	}
	catch(::Ice::Exception& ex)
	{
		glog(Log::L_DEBUG,"Ice Error when renew ticket:%s",ex.ice_name().c_str());
	}
	catch(...)
	{
		glog(Log::L_ERROR,"unexpect error when RenewTicketRequest call renewPathTicket ");
	}
	return 1;
}



DestroyPlaylistRequest::DestroyPlaylistRequest(ZQ::common::NativeThreadPool&  pool,
											    std::string& strPrx,
												ZQADAPTER_DECLTYPE objAdapter)
:ZQ::common::ThreadRequest(pool),_objAdapter(objAdapter)
{
	_strPrx=strPrx;
}
DestroyPlaylistRequest::~DestroyPlaylistRequest()
{
}
int DestroyPlaylistRequest::run()
{
	try
	{
		if(!_strPrx.empty())
		{
			Ice::CommunicatorPtr ic=_objAdapter->getCommunicator();
			TianShanIce::Streamer::InternalPlaylistExPrx prx=TianShanIce::Streamer::InternalPlaylistExPrx::checkedCast(ic->stringToProxy(_strPrx));			
			if(prx)
				prx->destroy();
		}		
	}
	catch(...)
	{
	}
	return 1;
}
void DestroyPlaylistRequest::final(int retcode,bool bCancelled)
{
	delete this;
}


#endif //_ICE_INTERFACE_SUPPORT
}}//namespace
