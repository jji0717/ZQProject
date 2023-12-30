
#include <TianShanDefines.h>
#include <TianShanIceHelper.h>
#include "SsStreamImpl.h"
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
	#define	PLFMT(x,y) 	"%s/%08X/SsPlaylist[%16s] "y,ident.name.c_str(),(unsigned int)pthread_self(),#x
#endif	

//////////////////////////////////////////////////////////////////////////
const std::string& SsStreamImpl::id( ) const
{
	return ident.name;
}

void SsStreamImpl::updateContextProperty( const std::string& key , const std::string& value )
{
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(updateContextProperty,"update property: [%s] ==>> [%s]"), key.c_str() , value.c_str() );
	Lock sync(*this);
	ZQTianShan::Util::updatePropertyData(property , key,  value );
}
void SsStreamImpl::updateContextProperty( const std::string& key , int32 value ) 
{
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(updateContextProperty,"update property: [%s] ==>> [%d]"), key.c_str() , value );
	Lock sync(*this);
	ZQTianShan::Util::updatePropertyData(property , key,  value );
}
void SsStreamImpl::updateContextProperty( const std::string& key , int64 value ) 
{
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(updateContextProperty,"update property: [%s] ==>> [%lld]"), key.c_str() , value);
	Lock sync(*this);
	ZQTianShan::Util::updatePropertyData(property , key,  value );
}
void SsStreamImpl::updateContextProperty( const std::string& key , float value ) 
{
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(updateContextProperty,"update property: [%s] ==>> [%f]"), key.c_str() , value);
	Lock sync(*this);
	ZQTianShan::Util::updatePropertyData(property , key,  value );
}

void SsStreamImpl::updateContextProperty( const TianShanIce::Properties& props )
{
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(updateContextProperty,"update property: %s"),ZQTianShan::Util::dumpStringMap(props).c_str() );
	Lock sync(*this);
	ZQTianShan::Util::mergeProperty( property , props , true );
}

std::string SsStreamImpl::getContextProperty( const std::string& key ) const
{
	Lock sync(*this);
	TianShanIce::Properties::const_iterator it = property.find(key);
	if( it == property.end() )
		return std::string("");
	else
		return it->second;
}

void SsStreamImpl::removeProperty( const std::string& key ) 
{
	ENVLOG(ZQ::common::Log::L_INFO, PLFMT(removeProperty,"remove property with key[%s]"),key.c_str() );
	Lock sync(*this);
	property.erase( key );
}
bool SsStreamImpl::hasContextProperty( const std::string& key ) const 
{
	Lock sync(*this);
	return property.find(key) != property.end();
}

const TianShanIce::Properties& SsStreamImpl::getContextProperty( ) const
{
	Lock sync(*this);
	return property;
}

std::string SsStreamImpl::getStreamingPort ( ) const
{	
	return getStreamPort();
}

const TianShanIce::SRM::ResourceMap&	SsStreamImpl::getContextResources( ) const
{
	Lock sync(*this);
	return crResource;
}

::TianShanIce::SRM::ResourceMap SsStreamImpl::getResources(const ::Ice::Current& c) const
{
	Lock sync(*this);
	return crResource;
}

void SsStreamImpl::updateContextResources( const TianShanIce::SRM::ResourceMap& res ) 
{
	Lock sync(*this);
	crResource = res;
}

const std::string contentStateKey = "StreamService.ContentInService";
SsStreamImpl::SsStreamImpl(SsServiceImpl& serviceImpl , SsEnvironment* environment )
:SsStreamBase(serviceImpl,environment)
{
}

SsStreamImpl::SsStreamImpl( SsServiceImpl& serviceImpl , 
						   SsEnvironment* environment ,
						   const TianShanIce::Properties& streamingResource,
						   const Ice::Identity& plIdent )
:SsStreamBase(serviceImpl,environment,true)
{//playlist is constructed
	bAlive	=	true;
	bCommited = false;
	playlistState = TianShanIce::Streamer::stsSetup;
	ident = plIdent;
	
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(ConstructSsStreamImpl,"create SsStreamImpl with %s"),ZQTianShan::Util::dumpStringMap(streamingResource).c_str() );

	ZQTianShan::Util::mergeProperty( property , streamingResource , true );	 
	ZQTianShan::Util::getPropertyDataWithDefault( streamingResource , STREAMINGRESOURCE_STREAMERID_KEY ,"" , streamerReplicaId );
}

SsStreamImpl::~SsStreamImpl()
{
}

void SsStreamImpl::allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx&,
								   const ::Ice::Current& )
{
	ZQTianShan::_IceThrow<TianShanIce::NotSupported>(ENVLOG,LOGCATAGORY,0,"Not supported");
}

std::string SsStreamImpl::lastError(const ::Ice::Current& ) const 
{
	Lock sync(*this);
	return lastErrorString;
}

Ice::Identity SsStreamImpl::getIdent(const ::Ice::Current& ) const 
{
	Lock sync(*this);
	return ident;
}

void SsStreamImpl::setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask& mask ,
										 const ::TianShanIce::Streamer::ConditionalControlPrx& proxy,
										 const ::Ice::Current& )
{
	Lock sync(*this);
	condControl = proxy;
	condCtrlMask = mask;
	ENVLOG(ZQ::common::Log::L_INFO, PLFMT(setConditionalControl,"set with mask[%X]") , mask );
}

void SsStreamImpl::setMuxRate(::Ice::Int nowRate, ::Ice::Int maxRate, ::Ice::Int minRate, const ::Ice::Current& /*= ::Ice::Current()*/)
{
	Lock sync(*this);
	ENVLOG(ZQ::common::Log::L_INFO,
		PLFMT(setMuxRate,"set mux rate now[%u] max[%u] min[%u]"),
		nowRate , maxRate,minRate);
	nowMuxRate		=	nowRate;
	maxMuxRate		=	maxRate;
	minMuxRate		=	minRate;
	
	ZQTianShan::Util::updatePropertyData( property , STREAMINGRESOURCE_MUXRATE_MAX , maxRate );
}

bool SsStreamImpl::allocDVBCResource(::Ice::Int, ::Ice::Int, const ::Ice::Current& )
{
	ZQTianShan::_IceThrow<TianShanIce::NotSupported>(ENVLOG,LOGCATAGORY,0,"allocDVBCResource is not supported any more");
	return false;
}

void SsStreamImpl::enableEoT(bool enable, const ::Ice::Current&)
{
	Lock sync(*this);
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(enableEoT," [%s] EOT protection"),
		enable ? "enable" : "disable");
	bEnableEOT = enable;
}

bool SsStreamImpl::distance(::Ice::Int to, ::Ice::Int from, ::Ice::Int& dist, const ::Ice::Current& )
{
	ZQTianShan::_IceThrow<TianShanIce::NotSupported>(ENVLOG,LOGCATAGORY,0,"not supported now");
	return false;
}

Ice::Int SsStreamImpl::findItem(::Ice::Int userCtrlNum, ::Ice::Int from, const ::Ice::Current&)
{
	Lock sync(*this);
	iter it = findItemWithUserCtrlNum( from )	;
	if( !iterValid(it) )
	{
		it = iterFirstItem();
	}
	for ( ; it != iterEndItem() ; it++ )
	{
		if(it->userCtrlNum == userCtrlNum )
		{
			return userCtrlNum;
		}
	}
	return TianShanIce::Streamer::InvalidCtrlNum;
}

TianShanIce::IValues SsStreamImpl::getSequence(const ::Ice::Current& ) const 
{
	TianShanIce::IValues retValue;
	{
		Lock sync(*this);
		constIter it = iterFirstItem();
		for( ; it != iterEndItem() ; it ++ )
		{
			retValue.push_back(it->userCtrlNum );
		}
	}
#pragma message(__MSGLOC__"TODO: log the return value")
	return retValue;
}

bool SsStreamImpl::isCompleted(const ::Ice::Current& )
{
	Lock sync(*this);
	return ( iterCurrentItem()->sessionState  == TianShanIce::Streamer::stsSetup ||
				iterCurrentItem()->sessionState  == TianShanIce::Streamer::stsStop );
}

bool SsStreamImpl::empty(const ::Ice::Current& ) const 
{
	Lock sync(*this);
	return itemCount() <= 0 ;
}

Ice::Int SsStreamImpl::left(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return static_cast<Ice::Int>( iterEndItem() - iterCurrentItem());	
}

Ice::Int SsStreamImpl::size(const ::Ice::Current&  ) const 
{
	Lock sync(*this);
	return static_cast<Ice::Int>( itemCount( ) );
}

Ice::Int SsStreamImpl::pushBack(::Ice::Int userCtrlNum,
								const ::TianShanIce::Streamer::PlaylistItemSetupInfo& info,
								const ::Ice::Current& c)
{
	return insert( userCtrlNum , info , TianShanIce::Streamer::InvalidCtrlNum ,c );
}

std::string SsStreamImpl::getId(const ::Ice::Current&  ) const
{
	Lock sync(*this);
	return ident.name;
}

void SsStreamImpl::attachSession( const ::TianShanIce::SRM::SessionPrx& srmPrx , const ::Ice::Current& ) 
{
	Lock sync(*this);
	srmSession = srmPrx;
}

void SsStreamImpl::setDestination(const ::std::string& targetIp, ::Ice::Int targetPort, const ::Ice::Current&) 
{
	Lock sync(*this);
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(setDestination,"set destination [%s][%d]"),
		targetIp.c_str() , targetPort );
	targetIpAddress	=	targetIp;
	targetIpPort	=	targetPort;

	ZQTianShan::Util::updatePropertyData( property , STREAMINGRESOURCE_DESTINATION_IPADDRESS , targetIp );
	ZQTianShan::Util::updatePropertyData( property , STREAMINGRESOURCE_DESTINATION_UDPPORT , targetPort );	
}

void SsStreamImpl::setDestMac(const ::std::string& mac, const ::Ice::Current& )
{
	Lock sync(*this);
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(setDestMac,"set destination macaddress [%s]"),mac.c_str() );
	targetMac	=	mac;	
	ZQTianShan::Util::updatePropertyData( property , STREAMINGRESOURCE_DESTINATION_MACADDRESS , mac );
}

void SsStreamImpl::setSourceStrmPort(::Ice::Int port, const ::Ice::Current& ) 
{
	Lock sync(*this);
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(setSourceStrmPort,"set source port to [%d]"),port);
	sourcePort	=	port;
}

void SsStreamImpl::setPID(::Ice::Int pid, const ::Ice::Current& ) 
{
	Lock sync(*this);
#pragma message(__MSGLOC__"TODO: not implement yet")
}

bool SsStreamImpl::renewTicket(  Ice::Int interval, const Ice::Current& )
{	
	TianShanIce::Transport::PathTicketPrx ticket = NULL;
	{
		Lock sync(*this);
		ticket = pathTicket;
	}
	if( !ticket )	return false;
	try
	{
		ticket->renew( interval );
	}
	catch( const Ice::ObjectNotExistException& ex)
	{
		ENVLOG(ZQ::common::Log::L_WARNING, PLFMT(renewTicket,"caught %s when renew path ticket"), ex.ice_name().c_str() );
		return false;
	}
	catch( const Ice::Exception& )
	{
	}	
	return true;	
}

SsStreamImpl::iter SsStreamImpl::convertPositionToItemWide( iter it , Ice::Long& offset , Ice::Short& from , bool forward  )
{
	if( !iterValid( it ) )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(convertPositionToItemWide,"invalid item"));
	}
	getItemAttribute(it);
	
	if ( from == 1 )
	{
		offset = it->setupInfo.inTimeOffset + offset;		
	}
	else if( from == 2 )
	{		
		offset = offset + it->itemOutTimeOffset;		
		
	}
	else if( offset == 0 )
	{		
		offset	= it->itemInTimeOffset;
	}
	else
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(convertPositionToItemWide,"seek from current position is not supported"));
	}
	from = 1;
	Ice::Int preloadTime = env->getConfig().iPreloadTimeInMS; 
	if( forward )
	{
		offset = offset < it->setupInfo.inTimeOffset ? it->setupInfo.inTimeOffset : offset;
		Ice::Long outtimeoffset = it->itemOutTimeOffset;
		if( preloadTime > 0 )
		{				
			outtimeoffset -= preloadTime;
			outtimeoffset = outtimeoffset < 0 ? 0 : outtimeoffset;
		}
		offset = offset > outtimeoffset ? outtimeoffset : offset;
	}
	else
	{
		offset = offset > it->itemOutTimeOffset ? it->itemOutTimeOffset : offset;
		Ice::Long intimeoffset = it->itemInTimeOffset;
		if( preloadTime > 0 )
		{
			intimeoffset += preloadTime;
			intimeoffset = intimeoffset < 0 ? 0 : intimeoffset;
		}
		offset = offset < intimeoffset ? intimeoffset : offset;
	}
	return it;
}

SsStreamImpl::iter SsStreamImpl::convertPositionToItemWide( Ice::Int& userCtrlNum , Ice::Long& offset , Ice::Short& from , bool newDirection )
{
	iter it = findItemWithUserCtrlNum(userCtrlNum);
	return convertPositionToItemWide( it, offset ,from , newDirection );
}

SsStreamImpl::iter SsStreamImpl::convertPositionToItemWide( Ice::Long& offset , Ice::Short& from ,bool forward )
{
	assert( itemCount() >= 0 );
	if( itemCount() < 1 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
			PLFMT(convertPositionToItemWide,"there is no item in curent playlist"));
	}
	iter it = iterEndItem();
	switch( from )
	{
	case SEEK_FROM_BEGIN:
		{
			if( offset <0 )//must not be negative
				offset = 0 ;
			it = iterFirstItem();
			for ( ; it != iterEndItem() ; it ++ )
			{
				if( !getItemAttribute(it) )
				{
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
						PLFMT(convertPositionToItemWide,"Can't get attribute of item[%s] userCtrlNum[%d]"),
						it->setupInfo.contentName.c_str() ,
						it->userCtrlNum );
				}
				if( offset >= it->itemRealDuration )
				{
					offset -= it->itemRealDuration;
				}
				else
				{
					break;
				}
			}			
			if ( it == iterEndItem() && offset > 0 )
			{
				it		= iterLastItem();
				offset	= it->itemOutTimeOffset;
			}
			else
			{
				if( it == iterEndItem() )
					it = iterLastItem();
				offset = offset + it->itemInTimeOffset;				
			}
			offset = offset < 0 ? 0 : offset;			
		}
		break;
	case SEEK_FROM_END:
		{
			from = SEEK_FROM_BEGIN;
			if (offset >0 )//must not be positive
				offset = 0;

			offset = - offset;
			it = iterLastItem();
			for ( ; it != iterBeginItem() ; it -- )
			{
				if( !getItemAttribute(it) )
				{
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
						PLFMT(convertPositionToItemWide,"Can't get attribute of item[%s] userCtrlNum[%d]"),
						it->setupInfo.contentName.c_str() ,
						it->userCtrlNum );
				}
				if( offset >= it->itemRealDuration )
				{
					offset -= it->itemRealDuration;
				}
				else
				{
					break;
				}				
			}
			if ( it == iterBeginItem() && offset > 0 )
			{
				it		= iterFirstItem();
				offset	= it->itemInTimeOffset;
			}
			else
			{
				if( it == iterBeginItem() )
					it = iterFirstItem();
				offset = it->itemInTimeOffset + ( it->itemOutTimeOffset - offset );
			}			
		}
		break;
	case SEEK_FROM_CURRENT:
		{
			if ( offset == 0 )
			{
				it = iterCurrentItem();
				offset = it->itemInTimeOffset;				
			}
			else
			{
				ZQTianShan::_IceThrow<TianShanIce::NotSupported>(ENVLOG,LOGCATAGORY,0,
					PLFMT(convertPositionToItemWide,"seek from current position is not supported"));
			}
		}
		break;
	default:
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
				PLFMT(convertPositionToItemWide,"Invalid from [%d]") , from );
		}
		break;
	}
	
	from = 1;

	Ice::Int preloadTime = env->getConfig().iPreloadTimeInMS; 
	if( forward )
	{
		offset = offset < it->setupInfo.inTimeOffset ? it->setupInfo.inTimeOffset : offset;
		Ice::Long outtimeoffset = it->itemOutTimeOffset;
		if( preloadTime > 0 )
		{				
			outtimeoffset -= preloadTime;
			outtimeoffset = outtimeoffset < 0 ? 0 : outtimeoffset;
		}
		offset = offset > outtimeoffset ? outtimeoffset : offset;
	}
	else
	{
		offset = offset > it->itemOutTimeOffset ? it->itemOutTimeOffset : offset;
		Ice::Long intimeoffset = it->itemInTimeOffset;
		if( preloadTime > 0 )
		{
			intimeoffset += preloadTime;
			intimeoffset = intimeoffset < 0 ? 0 : intimeoffset;
		}
		offset = offset < intimeoffset ? intimeoffset : offset;
	}
	return it;
}
bool SsStreamImpl::getItemAttribute( iter it )
{//contentStateKey
	if ( contentIsInService(it) )
	{
		return true;
	}
	else
	{//get content's attribute such as duration and 
		StreamParams paras;
		paras.mask = MASK_CONTENT_DURATION | MASK_CONTENT_STATE;		
		int32 ret = SsServiceImpl::doGetStreamAttr( serviceImpl , *this ,"" , it->setupInfo , paras );
		if( ret != ERR_RETURN_SUCCESS )
		{
			throwException( ret , "SsServiceImpl::doGetStreamAttr" );
		}
		it->itemTotalDuration	=	paras.duration;
		if( it->setupInfo.outTimeOffset != 0 )
		{
			it->itemRealDuration	=	( it->itemTotalDuration > it->setupInfo.outTimeOffset ? it->setupInfo.outTimeOffset : it->itemTotalDuration )
										- it->setupInfo.inTimeOffset ;						
		}
		else
		{
			it->itemRealDuration	=	it->itemTotalDuration - it->setupInfo.inTimeOffset;
		}
		it->itemInTimeOffset		=	it->setupInfo.inTimeOffset;		
		
		it->itemOutTimeOffset		=	it->itemInTimeOffset + it->itemRealDuration;

		if( paras.mask & MASK_CONTENT_STATE)
		{
			ZQTianShan::Util::updateValueMapData( it->setupInfo.privateData , 
				contentStateKey,
				paras.contentState == TianShanIce::Storage::csInService ? 1 : 0 );
		}
		
		return true;

	}
}



bool SsStreamImpl::contentIsInService( constIter it )
{
	if( !iterValid(it) )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(contentIsInService,"invalid item"));
	}
	
	Ice::Int	iContentInService = 0;
	
	ZQTianShan::Util::getValueMapDataWithDefault( it->setupInfo.privateData , contentStateKey , 0 , iContentInService );

	return iContentInService >= 1;
}

bool SsStreamImpl::convertPositionToStreamWide( Ice::Long& offset , Ice::Short& from  )
{
//	int32 ret = ERR_RETURN_SUCCESS;
	
	switch ( from )
	{
	case 1://from begin
		{
			return true;
		}
		break;
	case 2:
		{
			if( offset > 0 )
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
					PLFMT(convertPositionToStreamWide,"offset must not be positive if from == END "));
			}
			iter it = iterLastItem();
			StreamParams paras;
			paras.mask = MASK_CONTENT_DURATION | MASK_CONTENT_STATE;
//			TianShanIce::Streamer::StreamState newState = TianShanIce::Streamer::stsSetup;
			while ( iterValid(it) )
			{				
				getItemAttribute( it );				
				//got item's duration
				offset = -offset;
				if ( offset <= it->itemRealDuration )
				{
					offset = it->itemRealDuration - offset + it->itemInTimeOffset;
					break;
				}
				it--;
			}

			if( !iterValid(it))
			{
				offset = 0;
			}
			return true;
		}
		break;
	case 0:
		{
			if ( offset == 0 )
			{
				//only allow 0 as the offset if from == 0 
			}
			else
			{
				ZQTianShan::_IceThrow<TianShanIce::NotSupported>(ENVLOG,LOGCATAGORY,0,
					PLFMT(convertPositionToItemWide,"seek from current position is not supported"));
			}
// 			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
// 				PLFMT(convertPositionToStreamWide,"seek from current is not supported"));
		}
		break;
	default:
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
				PLFMT(convertPositionToStreamWide,"unknown from [%d]"),from);
		}
		break;
	}

	return false;
}
bool SsStreamImpl::convertPositionToStreamWide( iter itTarget, Ice::Long& offset , Ice::Short& from )
{
	if(! iterValid( itTarget ) )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,
			PLFMT(convertPositionToStreamWide,"Invalid Item") );
		return false;
	}
	//NOTE:
	// The offset we calculated must relate to BEGIN
	iter itItem = iterFirstItem();

	Ice::Long	tmpOffset = 0;

	for( ; itItem <= itTarget ; itItem ++ )
	{
		if( !getItemAttribute(itItem) )
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(convertPositionToStreamWide,"can't get attribute for item[%s] ctrlNum[%d]"),
				itItem->setupInfo.contentName.c_str() ,
				itItem->userCtrlNum );
		}
		if( itItem != itTarget )
			tmpOffset += itItem->itemRealDuration;
	}

	switch ( from  )
	{
	case SEEK_FROM_BEGIN:
		{
			if ( offset <0 )
			{
				offset = 0;
			}
			tmpOffset += itTarget->itemInTimeOffset + offset;
			tmpOffset = tmpOffset < itTarget->itemInTimeOffset ? itTarget->itemInTimeOffset : tmpOffset;
		}
		break;
	case SEEK_FROM_END:
		{
			if( offset > 0 )
			{
				offset = 0;
			}
			tmpOffset += itTarget->itemOutTimeOffset + offset;
			tmpOffset = tmpOffset < itTarget->itemInTimeOffset ? itTarget->itemInTimeOffset : tmpOffset;
			tmpOffset = tmpOffset > itTarget->itemOutTimeOffset ? itTarget->itemOutTimeOffset : tmpOffset;
		}
		break;
	case SEEK_FROM_CURRENT:
		{

			if ( offset == 0 )
			{//only take 0 as offset if from == 0
				from		= 0;
				tmpOffset	= 0;
			}
			else
			{
				ZQTianShan::_IceThrow<TianShanIce::NotSupported>(ENVLOG,LOGCATAGORY,0,
					PLFMT(convertPositionToItemWide,"seek from current position is not supported"));
			}
// 			ZQTianShan::_IceThrow<TianShanIce::NotSupported>(ENVLOG,LOGCATAGORY,0,
// 				PLFMT(convertPositionToStreamWide,"seek from current position is not supported"));
		}
		break;
	default:
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
				PLFMT(convertPositionToStreamWide,"invalid parameter with from [%d]") , from );
		}
		break;
	}

	from	= SEEK_FROM_BEGIN;
	
	offset	= tmpOffset;	

	return true;
}
bool SsStreamImpl::convertPositionToStreamWide( Ice::Int& userCtrlNum , Ice::Long& offset , Ice::Short& from )
{	
	assert( itemCount() >= 0 );
	iter itTarget = findItemWithUserCtrlNum( userCtrlNum );	
	return convertPositionToStreamWide(itTarget , offset , from );
}



//计算结果的时候以porting layer返回的数值为准
bool SsStreamImpl::convertResultToStreamWide( constIter itTarget , Ice::Long& offset )
{	
	if ( !iterValid( itTarget ) )
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
			PLFMT(convertResultToStreamWide," invalid item passed in ") );
	}
	offset -= itTarget->itemInTimeOffset;
	offset = offset < 0 ? 0 : offset;
	iter it = iterFirstItem();
	Ice::Long tmpOffset = 0; 
	for ( ; it != itTarget ; it ++ )
	{
		if ( !getItemAttribute( it ) )
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(convertResultToStreamWide,"can't get attribute of item[%s] userCtrlNum[%d]" ),
				it->setupInfo.contentName.c_str() ,
				it->userCtrlNum );
		}
		tmpOffset += it->itemRealDuration;
	}
	tmpOffset += offset;
	offset = tmpOffset;
	return true;
}

SsStreamImpl::iter SsStreamImpl::convertResultToItemWide( iter it , Ice::Long& offset )
{
	if( !iterValid(it))
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(convertResultToItemWide,"invalid parameter"));
	}
	offset = offset - it->itemInTimeOffset;
	offset = offset < 0 ? 0 : offset;
	return it;
}

Ice::Long SsStreamImpl::calculateDuration( iter itFirst , iter itLast ) 
{
	if ( ! (iterValid(itFirst) && iterValid(itLast) ))
	{
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(calculateDuration,"invalid input parameter"));
		return -1;
	}
	Ice::Long lRet = 0;
	iter it = itFirst;
	for( ; it <= itLast ; it ++ )
	{
		if( !getItemAttribute(it) )
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(convertResultToItemWide,"can't get attribute of item [%s] userctrlNum[%d]"),
				it->setupInfo.contentName.c_str() , it->userCtrlNum );
		}
		lRet += it->itemRealDuration;
	}
	return lRet;
}

SsStreamImpl::iter SsStreamImpl::convertResultToItemWide( Ice::Long& offset )
{
	if ( itemCount() < 1 )
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
			PLFMT(convertResultToItemWide,"no item in playlist , can't convert result"));
	}
	if( offset < 0 )
	{//adjust offset to 0
		offset = 0 ;
	}
	iter it = iterFirstItem();
	for ( ; it != iterEndItem() ; it ++ )
	{
		if( !getItemAttribute(it) )
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(convertResultToItemWide,"can't get attribute of item [%s] userctrlNum[%d]"),
				it->setupInfo.contentName.c_str() , it->userCtrlNum );
		}
		if( offset > it->itemRealDuration )
		{
			offset -= it->itemRealDuration;
		}
		else
		{
			break;
		}
	}
	if ( it == iterEndItem() )
	{
		it = iterLastItem();
		offset = it->itemOutTimeOffset;
	}
	else
	{
		offset -= it->itemInTimeOffset;
		offset = offset < 0 ? 0 : offset;
	}
	return it;
}

SsStreamImpl::iter	SsStreamImpl::findNextCriticalStartItem( time_t& timeout )
{
	timeout = 0x7FFFFFFF;
	if ( isReverseStreaming() )
	{
		return iterEndItem();
	}
	else
	{
		time_t now;
		time(&now);
		iter it = iterCurrentItem();
		if( !iterValid(it))	
		{
			return it;
		}		
		
		it ++;

		for ( ; it < iterEndItem() && 0 == it->setupInfo.criticalStart ; it++ )
			;

		if(iterValid(it))
		{
			time_t now;
			time(&now);
			timeout = static_cast<time_t>( ( it->setupInfo.criticalStart - now) *1000 );
			ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(findNextCriticalStartItem,"find criticalStart Item[%s] [%u] at [%s] ,[%d] from now"),
				it->setupInfo.contentName.c_str() ,
				it->userCtrlNum,
				ZQTianShan::Util::convertCriticalStartToStr( it->setupInfo.criticalStart).c_str(),
				timeout);
		}
		if ( timeout < 0 || timeout>0x7fffffff )
			timeout = 0;

		return it;
	}
}

bool SsStreamImpl::getRunningSession(::std::string& curSess, ::std::string& nextSess, ::Ice::Float& speed, ::TianShanIce::Streamer::StreamState& state, const ::Ice::Current&  ) const 
{
	//weird ? 
	serviceImpl.getRenewTicketCenter().doRegister(ident, pathTicket );

	if( env->getConfig().iSupportPlaylist == LIB_SUPPORT_NORMAL_PLAYLIST )
	{
		if( iterValid( iterCurrentItem() ) )
		{
			curSess = iterCurrentItem()->sessionId;
		}
		if( iterValid( iterNextItem() ) )
		{
			nextSess = iterNextItem()->sessionId;
		}
	}
	else
	{
		curSess = getStreamSessId( iterEndItem() );
	}
	speed = this->speed;
	state = this->playlistState;
	return !curSess.empty();
}


#define STRSWITCH() if(0){
#define STRCASE(x)	} else if(::strncmp( it->c_str() , x ,strlen(x) ) == 0 ){
#define STRENDCASE() }

int32 SsStreamImpl::getExpectParamMask( const TianShanIce::StrValues& expectedProps ) const
{
	int32 mask = 0;
	TianShanIce::StrValues::const_iterator it = expectedProps.begin();
	for( ; it != expectedProps.end() ; it ++ )
	{
		STRSWITCH()
			STRCASE("ITEM_CURRENTPOS")		mask |= VALUE_WANTED_ITEM_NPT;
			STRCASE("ITEM_TOTALPOS")			mask |= VALUE_WANTED_ITEM_DURATION;
			STRCASE("CURRENTPOS")				mask |= VALUE_WANTED_STREAM_NPT;
			STRCASE("TOTALPOS")				mask |= VALUE_WANTED_STREAM_DURATION;
			STRCASE("SPEED")					mask |= VALUE_WANTED_STREAM_SPEED;
			STRCASE("STATE")					mask |= VALUE_WANTED_STREAM_STATE;
			STRCASE("USERCTRLNUM")				mask |= VALUE_WANTED_USER_CTRLNUM;
		STRENDCASE()
	}
	return mask;
}
int32 SsStreamImpl::getExpectParamMask( const int32& mask ) const
{
	int32 retMask = 0 ;
	if( mask & VALUE_WANTED_ITEM_NPT || mask & VALUE_WANTED_STREAM_NPT )
	{
		retMask |= MASK_TIMEOFFSET;
	}
	if( mask & VALUE_WANTED_ITEM_DURATION || mask & VALUE_WANTED_STREAM_DURATION )
	{
		retMask |= MASK_CONTENT_DURATION;
	}
	if( mask & VALUE_WANTED_STREAM_SPEED )
	{
		retMask |= MASK_SCALE;
	}
	if( mask & VALUE_WANTED_STREAM_STATE )
	{
		retMask |= MASK_STATE;
	}
	return retMask;
}

std::string SsStreamImpl::getAttribute(const ::std::string& key, const ::Ice::Current& ) const 
{
	return getContextProperty(key);
}
std::string	 SsStreamImpl::getStreamPort( ) const
{
	std::string streamPort;
	ZQTianShan::Util::getPropertyDataWithDefault( property ,STREAMINGRESOURCE_STREAMPORT_KEY , "", streamPort );
	return streamPort;
}

std::string	SsStreamImpl::getStreamSessId( constIter it ) const
{
	if( iterValid(it))
	{
		return it->sessionId;
	}
	else
	{
		std::string sessId;
		ZQTianShan::Util::getPropertyDataWithDefault( property , STREAMING_ATTRIBUTE_STREAM_SESSION_ID , "", sessId );
		return sessId;
	}
}

void SsStreamImpl::updateStreamSessId( iter it , const std::string& sessId )
{
	if( iterValid(it))
	{
		it->sessionId	=	sessId;
	}
	else
	{
		ZQTianShan::Util::updatePropertyData( property , STREAMING_ATTRIBUTE_STREAM_SESSION_ID , sessId );
	}
}

std::string	 SsStreamImpl::dumpStreamParas( const StreamParams& paras )
{
	std::ostringstream oss;
	
	if( paras.mask & MASK_SCALE )
		oss <<"Scale["<<paras.scale<<"] ";

	if( paras.mask & MASK_TIMEOFFSET )
		oss <<"TimeOffset["<<paras.timeoffset<<"] ";

	if( paras.mask & MASK_CONTENT_DURATION )
		oss <<"Duration["<<paras.duration<<"] ";

	return oss.str();
	
}

void SsStreamImpl::analyzeInstanceState( )
{
	if( env->getConfig().iSupportPlaylist == LIB_SUPPORT_NORMAL_PLAYLIST )
	{
		if( mInstanceStateBeforeAction.itItem != mInstanceStateAfterAction.itItem )
		{
			fireItemSteppedMsg( mInstanceStateBeforeAction.itItem , 
								mInstanceStateAfterAction.itItem ,
								(Ice::Int)mInstanceStateAfterAction.timeOffset );
		}
	}
}



void SsStreamImpl::play_async(const ::TianShanIce::Streamer::AMD_Stream_playPtr& callback, const ::Ice::Current& c)
{
	try
	{
		(new SsStreamPlayRequest(  this , env , c ,callback ))->execute();
	}
	catch( const Ice::Exception& ex)
	{
		callback->ice_exception(ex);
	}
}
void SsStreamImpl::seekStream_async(const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr& callback, ::Ice::Long offset, ::Ice::Int startPos, const ::Ice::Current& c)
{
	try
	{
		(new SsStreamSeekStreamRequest( this , env , c, callback , offset , startPos ))->execute();
	}
	catch( const Ice::Exception& ex)
	{
		callback->ice_exception(ex);
	}
}
void SsStreamImpl::playEx_async(const ::TianShanIce::Streamer::AMD_Stream_playExPtr& callback, ::Ice::Float newSpeed, ::Ice::Long offset, ::Ice::Short from, const ::TianShanIce::StrValues& expectedProps, const ::Ice::Current& c)
{
	try
	{
		(new SsStreamPlayExRequest( this , env , c , callback , newSpeed , offset , from ,expectedProps))->execute();
	}
	catch( const Ice::Exception& ex)
	{
		callback->ice_exception(ex);
	}
}
void SsStreamImpl::skipToItem_async(const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr& callback, ::Ice::Int where, bool bPlay, const ::Ice::Current& c )
{
	try
	{
		(new SsStreamSkipToItemRequest( this , env , c , callback , where ,bPlay))->execute();
	}
	catch( const Ice::Exception& ex)
	{
		callback->ice_exception(ex);
	}
}
void SsStreamImpl::seekToPosition_async(const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr& callback, ::Ice::Int userCtrlNum, ::Ice::Int timeOffset, ::Ice::Int startPos, const ::Ice::Current& c ) 
{
	try
	{
		(new SsStreamSeekToPositionRequest( this , env, c , callback , userCtrlNum , timeOffset , startPos ))->execute();
	}
	catch( const Ice::Exception& ex)
	{
		callback->ice_exception(ex);
	}
}
void SsStreamImpl::playItem_async(const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr& callback, ::Ice::Int userCtrlNum, ::Ice::Int timeOffset, ::Ice::Short from , ::Ice::Float newSpeed, const ::TianShanIce::StrValues& expectedProps, const ::Ice::Current& c)
{
	try
	{
		(new SsStreamPlayItemRequest( this , env , c , callback , userCtrlNum , timeOffset , from , newSpeed , expectedProps ))->execute();
	}
	catch( const Ice::Exception& ex)
	{
		callback->ice_exception(ex);
	}
}
void SsStreamImpl::commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr& callback,const ::Ice::Current& c)
{
	try
	{
		(new SsStreamCommitRequest(this,env, c ,callback))->execute();
	}
	catch( const Ice::Exception& ex)
	{
		callback->ice_exception(ex);
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#if defined ZQ_OS_MSWIN
#define	REQFMT(x,y) 	"[SsStreamRequest]%s TID[%08X][%16s]\t"##y,streamImpl->ident.name.c_str(),GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
#define	REQFMT(x,y) 	"[SsStreamRequest]%s TID[%08X][%16s] "y,streamImpl->ident.name.c_str(),(unsigned int)pthread_self(),#x
#endif

int SsStreamCommitRequest::run( )
{
	ZQTianShan::Util::TimeSpan sw;sw.start();
	ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(run,"enter commit request"));
	//generate a IRP
	bool bRet = false;
	
	try
	{
		std::string	streamPort;
		ZQTianShan::Util::getPropertyDataWithDefault( streamImpl->property , STREAMINGRESOURCE_STREAMPORT_KEY ,"", streamPort );
		if( streamPort.empty() )
		{
			ENVLOG(ZQ::common::Log::L_INFO,REQFMT(SsStreamCommitRequest,"no stream port is found in streaming resource"));
		}

		{
			{
				IceUtil::AbstractMutex::Lock sync(*streamImpl.get());
				if( streamImpl->bCommited  )
				{
					ENVLOG(ZQ::common::Log::L_INFO,REQFMT(SsStreamCommitRequest,"playlist has been comited, return directly"));
					TianShanIce::InvalidStateOfArt err;
					err.message = "already commited";
					mCallback->ice_exception(err);
					return -1;
				}
				streamImpl->bCommited = true;
			}

			ENVLOG(ZQ::common::Log::L_DEBUG, REQFMT(SsStreamCommitRequest,"trying to prepare commit resources"));

			PlaylistItemSetupInfos itemInfos;
			TianShanIce::Streamer::ItemSessionInfoS::const_iterator it = streamImpl->items.begin();	
			it++;
			for( ; it != streamImpl->items.end() ; it ++ )
			{
				itemInfos.push_back(it->setupInfo);
			}

			ENVLOG(ZQ::common::Log::L_DEBUG, REQFMT(SsStreamCommitRequest,"trying to commit a stream"));
			int32 ret = SsServiceImpl::doCommit( streamImpl->serviceImpl , *streamImpl.get(), itemInfos , streamImpl->crResource );
			if( ret != ERR_RETURN_SUCCESS )
			{
				ENVLOG(ZQ::common::Log::L_ERROR,REQFMT(SsStreamCommitRequest,"failed to commit stream"));
				TianShanIce::ServerError err;
				err.message = "failed to commit stream";
				mCallback->ice_exception( err );
				return -1;
			}		
			bRet = ( ret == ERR_RETURN_SUCCESS );
			if( bRet )
			{
				IceUtil::AbstractMutex::Lock sync(*streamImpl.get());
				PlaylistItemSetupInfos::const_iterator it = itemInfos.begin();
				TianShanIce::Streamer::ItemSessionInfoS::iterator itSessItem = streamImpl->iterFirstItem();
				for( ; it != itemInfos.end() ; it ++ )
				{
					itSessItem->setupInfo = *it;
					itSessItem++;
				}
				streamImpl->bCommited = true;
			}
		}
	}	
	catch(const TianShanIce::BaseException& ex)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,REQFMT(SsStreamCommitRequest,"failed to commit stream due to [%s]"),ex.message.c_str());
		mCallback->ice_exception( ex );
		return -1;
	}
	catch(const Ice::Exception& ex)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,REQFMT(SsStreamCommitRequest,"failed to commit stream due to [%s]"),ex.ice_name().c_str());
		mCallback->ice_exception( ex );
		return -1;
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,REQFMT(SsStreamCommitRequest,"caught unkown exception when porting layer execute SsServiceImpl::doCommit") );
		TianShanIce::ServerError err;
		err.message = "unknown exception when commit stream";
		mCallback->ice_exception( err );
		return -1;
	}
	ZQTianShan::Util::TimeSpan icerep;icerep.start();
	try
	{
		mCallback->ice_response();
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamCommitRequest,"[%s] caught when sending back the response to client"),
			ex.ice_name().c_str() );
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamCommitRequest,"unknown exception caught when sending back the response to client"));
	}
	ENVLOG(ZQ::common::Log::L_INFO, REQFMT(SsStreamCommitRequest,"leave commit request [%s], cost[%lld/%lld]"),
		bRet ?"Success":"Fail",sw.stop(),icerep.stop());
	return 1;
}

int SsStreamPlayRequest::run()
{
	ZQTianShan::Util::TimeSpan sw;sw.start();
	bool bRet = true;
	try
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamPlayRequest,"enter play"));
		bRet = streamImpl->play(mIceCurrent);
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamPlayRequest,"leave play"));
	}
	catch( const Ice::Exception& ex)
	{
		mCallback->ice_exception(ex);
		return -1;
	}
	catch(...)
	{
		TianShanIce::ServerError err;
		mCallback->ice_exception(err);
		return -1;
	}
	ZQTianShan::Util::TimeSpan icerp;icerp.start();
	try
	{
		mCallback->ice_response( bRet );
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamPlayRequest,"[%s] caught when sending back the response to client"),
			ex.ice_name().c_str() );
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamPlayRequest,"unknown exception caught when sending back the response to client"));
	}
	ENVLOG(ZQ::common::Log::L_INFO,REQFMT(SsStreamPlayRequest,"play , cost[%lld/%lld]"), sw.stop(), icerp.stop() );	
	return 0;
}

int SsStreamSeekStreamRequest::run()
{
	ZQTianShan::Util::TimeSpan sw; sw.start();
	Ice::Long lRet = 0;
	try
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamSeekStreamRequest,"enter seekStream with offset[%lld] startpos[%d]"),mOffset,mStartPos);
		lRet = streamImpl->seekStream(mOffset,mStartPos,mIceCurrent);
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamSeekStreamRequest,"leave seekStream"));
	}
	catch( const Ice::Exception& ex)
	{
		mCallback->ice_exception(ex);
		return -1;
	}
	catch(...)
	{
		TianShanIce::ServerError err;
		mCallback->ice_exception(err);
		return -1;
	}
	ZQTianShan::Util::TimeSpan icerp; icerp.start();
	try
	{
		mCallback->ice_response( lRet );
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamSeekStreamRequest,"[%s] caught when sending back the response to client"),
			ex.ice_name().c_str() );
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamSeekStreamRequest,"unknown exception caught when sending back the response to client"));
	}

	ENVLOG(ZQ::common::Log::L_INFO, REQFMT(SsStreamSeekStreamRequest,"seek, cost[%lld/%lld]"),sw.stop() ,icerp.stop() );

	return 0;
}

int SsStreamPlayExRequest::run()
{
	ZQTianShan::Util::TimeSpan sw; sw.start();
	TianShanIce::Streamer::StreamInfo info;
	try
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamPlayExRequest,"enter playEx with speed[%f] offset[%lld] startPos[%d] expectMask[%s]"),
			mNewSpeed , mOffset , mFrom , ZQTianShan::Util::dumpTianShanIceStrValues(mExpectedProps).c_str() );
		info = streamImpl->playEx( mNewSpeed , mOffset , mFrom , mExpectedProps , mIceCurrent );
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamPlayExRequest,"leave playEx"));
	}
	catch( const Ice::Exception& ex)
	{
		mCallback->ice_exception(ex);
		return -1;
	}
	catch(...)
	{
		TianShanIce::ServerError err;
		mCallback->ice_exception(err);
		return -1;
	}

	ZQTianShan::Util::TimeSpan icerp;icerp.start();
	try
	{
		mCallback->ice_response(info);
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamPlayExRequest,"[%s] caught when sending back the response to client"),
			ex.ice_name().c_str() );
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamPlayExRequest,"unknown exception caught when sending back the response to client"));
	}
	ENVLOG(ZQ::common::Log::L_INFO, REQFMT(SsStreamPlayExRequest,"playex, cost[%lld/%lld]"),sw.stop() ,icerp.stop() );
	return 0;
}

int SsStreamSkipToItemRequest::run()
{
	ZQTianShan::Util::TimeSpan sw; sw.start();
	bool bRet = true;
	try
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamSkipToItemRequest,"enter skipToItem with where[%d] play[%s]"),
			mWhere,mbPlay?"true":"false");
		bRet =  streamImpl->skipToItem( mWhere , mbPlay , mIceCurrent );
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamSkipToItemRequest,"leave skipToItem"));
	}
	catch( const Ice::Exception& ex)
	{
		mCallback->ice_exception(ex);
		return -1;
	}
	catch( ... )
	{
		TianShanIce::ServerError err;
		mCallback->ice_exception(err);
		return -1;
	}
	ZQTianShan::Util::TimeSpan icerp;icerp.start();
	try
	{
		mCallback->ice_response(bRet);
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamSkipToItemRequest,"[%s] caught when sending back the response to client"),
			ex.ice_name().c_str() );
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamSkipToItemRequest,"unknown exception caught when sending back the response to client"));
	}
	ENVLOG(ZQ::common::Log::L_INFO, REQFMT(SsStreamSkipToItemRequest,"playex, cost[%lld/%lld]"),sw.stop() ,icerp.stop() );
	return 0;
}

int SsStreamSeekToPositionRequest::run()
{
	ZQTianShan::Util::TimeSpan sw;sw.start();
	bool bRet  = true;
	try
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamSeekToPositionRequest,"enter seekToPosition with ctrlNum[%d] ofset[%d] startPos[%d]"),
			mUserCtrlNum,mTimeOffset,mStartPos);
		bRet = streamImpl->seekToPosition( mUserCtrlNum , mTimeOffset , mStartPos , mIceCurrent);
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamSeekToPositionRequest,"leave seekToPosition"));
	}
	catch( const Ice::Exception& ex)
	{
		mCallback->ice_exception(ex);
		return -1;
	}
	catch(...)
	{
		TianShanIce::ServerError err;
		mCallback->ice_exception(err);
		return -1;
	}

	ZQTianShan::Util::TimeSpan icerp;icerp.start();
	try
	{
		mCallback->ice_response(bRet);
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamSeekToPositionRequest,"[%s] caught when sending back the response to client"),
			ex.ice_name().c_str() );
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamSeekToPositionRequest,"unknown exception caught when sending back the response to client"));
	}
	ENVLOG(ZQ::common::Log::L_INFO, REQFMT(SsStreamSeekToPositionRequest,"playex, cost[%lld/%lld]"),sw.stop() ,icerp.stop() );
	return 0;
}

int SsStreamPlayItemRequest::run( )
{
	ZQTianShan::Util::TimeSpan sw;sw.start();
	TianShanIce::Streamer::StreamInfo		 info;
	try
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamPlayItemRequest,"enter playItem with ctrlNum[%d] offset[%d] startPos[%d] speed[%f] exepectMask[%s]"),
			mUserCtrlNum, mTimeOffset,mFrom,mNewSpeed,ZQTianShan::Util::dumpTianShanIceStrValues(mExpectedProps).c_str() );
		info = streamImpl->playItem( mUserCtrlNum , mTimeOffset, mFrom , mNewSpeed , mExpectedProps , mIceCurrent );
		ENVLOG(ZQ::common::Log::L_DEBUG,REQFMT(SsStreamPlayItemRequest,"leave playItem"));
	}
	catch( const Ice::Exception& ex )
	{
		mCallback->ice_exception( ex );
		return -1;
	}
	catch( ... )
	{

		TianShanIce::ServerError err;
		mCallback->ice_exception(err);
		return -1;
	}
	ZQTianShan::Util::TimeSpan icerp;icerp.start();
	try
	{
		mCallback->ice_response(info);
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamPlayItemRequest,"[%s] caught when sending back the response to client"),
			ex.ice_name().c_str() );
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SsStreamPlayItemRequest,"unknown exception caught when sending back the response to client"));
	}
	ENVLOG(ZQ::common::Log::L_INFO, REQFMT(SsStreamPlayItemRequest,"playItem, cost[%lld/%lld]"),sw.stop() ,icerp.stop() );
	return 0;
}

SsStreamRequest::SsStreamRequest( SsStreamImpl::Ptr stream , SsEnvironment* environment , const Ice::Current& c)
:ZQ::common::ThreadRequest(environment->getMainThreadPool()),
env(environment),
mIceCurrent(c),
streamImpl(stream)
{
	
}

void SsStreamRequest::execute( )
{
	ZQ::common::NativeThreadPool& pool = env->getMainThreadPool();
	int pendingSize = pool.pendingRequestSize();
	int activeCount = pool.activeCount();
	int poolSize = pool.size();
	static int maxPendingSize = env->getConfig().iMaxPendingRequestSize;
	if( pendingSize >= 1 )
	{
		ENVLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SsStreamRequest,"pendingRequest[%d], poolSize[%d], active threads[%d]"),
			pendingSize , poolSize , activeCount);
		if( pendingSize > maxPendingSize )
		{
			ZQ::common::Log* logger = &(env->getMainLogger());
			delete this;			
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SsStreamRequest,"too many pendingRequest[%d], maxPendingSize[%d] poolSize[%d], active threads[%d]"),
				pendingSize , maxPendingSize,
				poolSize , activeCount);
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(*logger,"StreamService",503, CLOGFMT(SsStreamRequest,"too many pendingRequest") );
		}
	}
	
	start();
}

}}//namespace ZQ::StreamService

