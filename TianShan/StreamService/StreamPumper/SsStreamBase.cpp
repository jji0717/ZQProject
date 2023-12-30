
#include <TianShanDefines.h>
#include <TianShanIceHelper.h>
#include "SsStreamBase.h"
#include "SsServiceImpl.h"

#ifdef ZQ_OS_MSWIN
#include "memoryDebug.h"
#endif

namespace ZQ
{
namespace StreamService
{

#if defined ZQ_OS_MSWIN
	#define	PLFMT(x,y) 	"%s/%08X/SsPlaylist[%16s]\t"##y,ident.name.c_str(),GetCurrentThreadId(),#x	
#else
	#define	PLFMT(x,y) 	"%s/%08X/SsPlaylist[%16s] "y,ident.name.c_str(),pthread_self(),#x
#endif	

SsStreamBase::SsStreamBase( SsServiceImpl& svcImpl , SsEnvironment* environment  )
:serviceImpl(svcImpl),
env(environment)
{
	bCurIterInited		= false;
	bNextIterInited		= false;
}

SsStreamBase::SsStreamBase( SsServiceImpl& svcImpl , SsEnvironment* environment, bool )
:serviceImpl(svcImpl),
env(environment)
{	
	initializeStream();	
	bCurIterInited		= false;
	bNextIterInited		= false;
}

SsStreamBase::~SsStreamBase( )
{
}

bool SsStreamBase::initializeStream( )
{
	
	condCtrlMask.play							= TianShanIce::Streamer::sccUndefined;
	condCtrlMask.changeSpeedForward				= TianShanIce::Streamer::sccUndefined;
	condCtrlMask.changeSpeedBackward			= TianShanIce::Streamer::sccUndefined;
	condCtrlMask.pause							= TianShanIce::Streamer::sccUndefined;
	condCtrlMask.seek							= TianShanIce::Streamer::sccUndefined;
	//add a dummy item
	bFirstPlay						= true;
	speed							= 1.0f;//set speed to 1.0 as default
	targetIpAddress					= "";
	targetIpPort					= 0;
	targetMac						= "";
	sourceIpAddress					= "";
	sourcePort						= 0;
	maxMuxRate						= 0;
	nowMuxRate						= 0;
	minMuxRate						= 0;
	playlistState					= TianShanIce::Streamer::stsSetup;
	lastOperationTime				= 0;
	currentItemCtrlNum				= TianShanIce::Streamer::InvalidCtrlNum;
	nextItemCtrlNum					= TianShanIce::Streamer::InvalidCtrlNum;
	lastErrorString					= "";
	lastErrorCode					= 0;
	condControl						= NULL;	
	srmSession						= NULL;
	pathTicket						= NULL;
	bEnableEOT						= true;
	bAlive							= true;
	property.clear();	
	opProfiles.clear();

	TianShanIce::Streamer::ItemSessionInfo dummyItem;
	dummyItem.itemInTimeOffset		= 0;
	dummyItem.itemOutTimeOffset		= 0 ;
	dummyItem.itemProfiles.clear();
	dummyItem.itemRealDuration		= 0;
	dummyItem.itemTotalDuration		= 0;
	dummyItem.loadedCount			= 0;
	dummyItem.muxBitrate			= 0;
	dummyItem.privateData.clear();
	dummyItem.runningOffset			= 0;
	dummyItem.sessFlag				= 0;
	dummyItem.sessionId				= "";
	dummyItem.sessFlag				= 0;
	dummyItem.sessionState			= TianShanIce::Streamer::stsSetup;
	dummyItem.timeLaunched			= 0;
	dummyItem.timeLoaded			= 0;
	dummyItem.timeUnloaded			= 0;
	dummyItem.userCtrlNum			= TianShanIce::Streamer::InvalidCtrlNum;	
	dummyItem.setupInfo.contentName	= "";
	dummyItem.setupInfo.criticalStart= 0;
	dummyItem.setupInfo.flags		= 0;
	dummyItem.setupInfo.forceNormal	= 0;
	dummyItem.setupInfo.inTimeOffset= 0;
	dummyItem.setupInfo.outTimeOffset= 0;
	dummyItem.setupInfo.privateData.clear();
	dummyItem.setupInfo.spliceIn	= 0;
	dummyItem.setupInfo.spliceOut	= 0;

	items.push_back(dummyItem);
	assert( itemCount() == 0 );
	setCurrentItem( iterBeginItem());
	setNextItem(iterBeginItem());

	return true;
}


SsStreamBase::iter	SsStreamBase::findItemWithUserCtrlNum( Ice::Int userCtrlNum )	
{
	assert( itemCount() >= 0 );
	iter it = iterFirstItem();
	for(  ; it != iterEndItem() ; it ++ )
	{
		if ( it->userCtrlNum == userCtrlNum )
		{
			break;
		}
	}
	return it;
}

SsStreamBase::constIter SsStreamBase::findItemWithUserCtrlNum( Ice::Int userCtrlNum ) const
{
	assert( itemCount() >= 0 );
	constIter it = iterFirstItem();
	for(  ; it != iterEndItem() ; it ++ )
	{
		if ( it->userCtrlNum == userCtrlNum )
		{
			break;
		}
	}
	return it;
}

bool SsStreamBase::isItemStreaming( SsStreamBase::constIter it ) const
{
	if( !iterValid(it) )
		return false;
	return ( it->sessionState != TianShanIce::Streamer::stsSetup &&
						it->sessionState != TianShanIce::Streamer::stsStop);
}

SsStreamBase::constIter	SsStreamBase::iterCurrentItem( ) const
{
	assert( itemCount() >= 0 );
	if( bCurIterInited )
	{
		return currentIter;
	}
	else
	{
		((SsStreamBase*)this)->bCurIterInited = true;
		
		iter it = ((SsStreamBase*)this)->items.begin() + 1;//hack
		for( ; it != ((SsStreamBase*)this)->items.end() ; it ++ )
		{
			if( it->userCtrlNum == currentItemCtrlNum )
				break;
		}
		((SsStreamBase*)this)->currentIter = it;
		return currentIter;
	}
}

SsStreamBase::iter			SsStreamBase::iterCurrentItem( )
{
	assert( itemCount() >= 0 );
	if( bCurIterInited )
	{
		return currentIter;
	}
	else
	{
		bCurIterInited = true;
		currentIter = findItemWithUserCtrlNum(currentItemCtrlNum);
		return currentIter;
	}
}

SsStreamBase::iter			SsStreamBase::iterNextItem( )
{
	assert( itemCount() >= 0 );
	if( bNextIterInited )
	{
		return nextIter;
	}
	else
	{
		bNextIterInited = true;
		nextIter		= findItemWithUserCtrlNum(nextItemCtrlNum);
		return nextIter;
	}
}

SsStreamBase::constIter		SsStreamBase::iterNextItem( ) const
{
	assert( itemCount() >= 0 );
	if( bNextIterInited )
	{
		return nextIter;
	}
	else
	{
		iter it = ((SsStreamBase*)this)->items.begin() + 1;//hack
		for( ; it != ((SsStreamBase*)this)->items.end(); it ++ )
		{
			if( it->userCtrlNum == nextItemCtrlNum )
				break;
		}
		((SsStreamBase*)this)->nextIter			= it;
		((SsStreamBase*)this)->bNextIterInited	= true;
		return nextIter;
	}
}

TianShanIce::SRM::ResourceMap SsStreamBase::getResources( const Ice::Current&  ) const
{
	Lock sync(*this);
	return crResource;
}

TianShanIce::Transport::PathTicketPrx SsStreamBase::getPathTicket(const ::Ice::Current&/* = ::Ice::Current()*/) const
{
	Lock sync(*this);
	return pathTicket;
}

TianShanIce::Properties SsStreamBase::getProperties(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return	property;
}

void SsStreamBase::setProperties( const ::TianShanIce::Properties& p, const ::Ice::Current& )
{
	Lock sync(*this);
	property	=	p;
}

int32 SsStreamBase::getEOTSize( ) const
{
	return env->getConfig().iEOTSize;
}

int32 SsStreamBase::getPreloadTime( ) const
{
	return env->getConfig().iPreloadTimeInMS;
}

bool SsStreamBase::setCurrentItem( Ice::Int userCtrlNum )
{
	currentIter = findItemWithUserCtrlNum(userCtrlNum);
	currentItemCtrlNum = userCtrlNum;
	bCurIterInited = true;
	return true;
}

bool SsStreamBase::setCurrentItem( iter it )
{
	currentIter			= it;
	bCurIterInited		= true;
	if( iterValid(it))
		currentItemCtrlNum	= it->userCtrlNum;
	else
		currentItemCtrlNum	= TianShanIce::Streamer::InvalidCtrlNum;
	return true;
}

bool SsStreamBase::setNextItem(  Ice::Int userCtrlNum )
{
	bNextIterInited		= true;
	nextIter			= findItemWithUserCtrlNum(userCtrlNum);
	nextItemCtrlNum		= userCtrlNum;
	return true;
}

bool SsStreamBase::setNextItem( iter it )
{
	bNextIterInited		= true;
	nextIter			= it;
	if( iterValid(it) )
	{
		nextItemCtrlNum	= it->userCtrlNum;
	}
	else
	{
		nextItemCtrlNum	= TianShanIce::Streamer::InvalidCtrlNum;
	}
	return true;
}

bool SsStreamBase::hasURL( constIter it ) const
{
	assert( iterValid(it));
	return it->setupInfo.privateData.find("storageLibraryUrl") != it->setupInfo.privateData.end();
}

std::string	SsStreamBase::getPID( constIter it ) const
{
	if( iterValid(it) )
	{
		std::string		pid;
		ZQTianShan::Util::getValueMapDataWithDefault(it->setupInfo.privateData,"providerId","",pid);
		return pid;
	}
	else
	{
		return "";
	}
}
std::string	SsStreamBase::getPAID( constIter it ) const
{
	if( iterValid(it))
	{
		std::string		paid;
		ZQTianShan::Util::getValueMapDataWithDefault(it->setupInfo.privateData,"providerAssetId","",paid);
		return paid;
	}
	else
	{
		return "";
	}
}

std::string	SsStreamBase::getNextURL( constIter it ) 
{
	assert( iterValid(it));	
	Ice::Int	idx = -1;
	ZQTianShan::Util::getPropertyDataWithDefault(property,"currentUsingUrlIndex",-1,idx);
	idx++;
	std::vector<std::string> urls;
	bool bRange = false;
	try
	{
		ZQTianShan::Util::getValueMapData(it->setupInfo.privateData,"storageLibraryUrl",urls,bRange);
	}
	catch(const TianShanIce::InvalidParameter&)
	{
		urls.clear();
	}
	if( urls.size() <= 0 )
	{
		return "";
	}
	if( idx >= static_cast<Ice::Int>(urls.size()) )
	{
		idx = static_cast<Ice::Int>(urls.size()) -1; 
	}
	
	ZQTianShan::Util::updatePropertyData( property,"currentUsingUrlIndex",idx);

	return urls[idx];
}

std::string	SsStreamBase::getLastURL( constIter it ) const
{
	if( iterValid(it))
	{
		Ice::Int	idx = 0;
		ZQTianShan::Util::getPropertyDataWithDefault( property,"currentUsingUrlIndex",0,idx );	
		std::vector<std::string> urls;
		bool bRange = false;
		try
		{
			ZQTianShan::Util::getValueMapData(it->setupInfo.privateData,"storageLibraryUrl",urls,bRange);
		}
		catch(const TianShanIce::InvalidParameter&)
		{
			urls.clear();
		}
		if( urls.size() <= 0 )
		{
			return "";
		}
		if( idx >= static_cast<Ice::Int>(urls.size()) )
		{
			idx = static_cast<Ice::Int>(urls.size()) -1; 
		}
		return urls[idx];
	}
	else
	{
		return "";
	}
}


void SsStreamBase::recordInstanceState( PlInstanceState& s )
{
	bool bStreaming =( playlistState == TianShanIce::Streamer::stsStreaming) || 
					  (	playlistState == TianShanIce::Streamer::stsPause );
	s.itItem		=	bStreaming ? iterCurrentItem() : iterEndItem();
	s.state			=	playlistState;	
}

}}//namespace ZQ::StreamService

