
#include "Log.h"
#include <TianShanIceHelper.h>
#include "phoStorageLink.h"
namespace ZQTianShan {
namespace AccreditedPath {

static ConfItem c2trsanfer_stor_conf[] = {
	{ "TotalBandwidth",	::TianShanIce::vtLongs, false, "",false},
	{ NULL, ::TianShanIce::vtInts, true, "" ,false},
};

static ConfItem standby_storelink_conf[] = {
	{ "TotalBandwidth",	::TianShanIce::vtLongs, false, "",false },
	{ "SessionThreshold", ::TianShanIce::vtInts, false, "", true },
	{ NULL, ::TianShanIce::vtInts, true, "" , false},
};
static ConfItem c2overaqua_stor_conf[] = {
	{ "TotalBandwidth",	::TianShanIce::vtLongs, false, "",false},
	{ NULL, ::TianShanIce::vtInts, true, "" ,false},
};
NssC2Transfer::NssC2Transfer( IPHOManager& mgr )
:IStorageLinkPHO(mgr),
mPhoManager(mgr)
{
}
NssC2Transfer::~NssC2Transfer()
{
  mPhoManager.unregisterStorageLinkHelper(STORLINK_TYPE_C2TRANSFER);
  mPhoManager.unregisterStorageLinkHelper(STORLINK_TYPE_C2TRANSFER_STANDBY);
  mPhoManager.unregisterStorageLinkHelper(STORLINK_TYPE_C2OVERAQUA);
}

bool NssC2Transfer::getSchema( const char* type, ::TianShanIce::PDSchema& schema )
{
	::ZQTianShan::ConfItem *config = NULL;

	// address the schema definition
	if (0 == strcmp( type, STORLINK_TYPE_C2TRANSFER) )
		config = c2trsanfer_stor_conf;
	else if (0 == strcmp(type, STORLINK_TYPE_C2TRANSFER_STANDBY) )
		config = standby_storelink_conf;
	else if (0 == strcmp(type, STORLINK_TYPE_C2OVERAQUA) )
		config = c2overaqua_stor_conf;

	// no matches
	if (NULL == config)
		return false;

	// copy the schema to the output
	for (::ZQTianShan::ConfItem *item = config; item && item->keyname; item ++)
	{
		::TianShanIce::PDElement elem;
		elem.keyname = item->keyname;
		//elem.optional = item->optional;
		elem.optional2 = item->optional2;
		elem.defaultvalue.type= item->type;
		elem.defaultvalue.bRange = item->bRange;
		switch(item->type) 
		{
		case TianShanIce::vtInts:
			{
				elem.defaultvalue.ints.clear();
				elem.defaultvalue.ints.push_back( atoi( item->hintvalue ) );
			}
			break;
		case TianShanIce::vtLongs:
			{
				elem.defaultvalue.lints.clear();
				elem.defaultvalue.lints.push_back( atoi( item->hintvalue ) );

			}
			break;
		case TianShanIce::vtStrings:
			{
				elem.defaultvalue.strs.clear();
				elem.defaultvalue.strs.push_back(item->hintvalue);
			}
			break;
		default:
			break;
		}
		schema.push_back(elem);
	}
	return true;
}

void NssC2Transfer::filteroutUsedBWForC2TransferConf( const char* identStr , StorageLinkAttr& attr )
{
	//TODO: not implement
	//scan all ticket all get the used bandwidth
	::TianShanIce::Transport::StorageLinkPrx storLinkPrx = mPhoManager.openStorageLink( identStr );
	if( !storLinkPrx )
	{
		return;
	}

	IdentCollection idc;
	try
	{
		idc = mPhoManager.listPathTicketsByLink(storLinkPrx);
	}
	catch (Ice::ObjectNotExistException&)
	{
		idc.clear();
		return;
	}

	IdentCollection::const_iterator it = idc.begin();
	for( ; it != idc.end() ; it++ )
	{
		::TianShanIce::Transport::PathTicketPrx ticketPrx = NULL;
		TianShanIce::ValueMap			prvData;
		TianShanIce::SRM::ResourceMap	resMap;
		std::string						ticketId;
		try
		{
			ticketPrx = mPhoManager.openPathTicket( *it );
			if(!ticketPrx)
				continue;

			prvData		=	ticketPrx->getPrivateData();
			//resMap		=	ticketPrx->getResources();
			ticketId	=	ticketPrx->getIdent().name;
		}
		catch(const Ice::Exception& ex )
		{
			glog(ZQ::common::Log::L_WARNING,CLOGFMT(NssC2Transfer,"getAllTicket() caught [%s] when get ticket for[%s]"),ex.ice_name().c_str() , it->name.c_str() );
			continue;
		}	

		std::string ownerLinkType;
		std::string ownerLinkId;
		Ice::Long	requestBW = 0;		
		try
		{
			ZQTianShan::Util::getValueMapData( prvData, PathTicketPD_Field(bandwidth), requestBW );
		}
		catch(const Ice::Exception&)
		{
			continue;
		}

		ZQTianShan::Util::getValueMapDataWithDefault( prvData, PathTicketPD_Field(ownerStorageLinkType), "", ownerLinkType );
		if( ownerLinkType != std::string(STORLINK_TYPE_C2TRANSFER)) // && ownerLinkType != std::string(STORLINK_TYPE_C2TRANSFER_STANDBY))
			continue;
		
		attr.usedBW += requestBW;

		ZQ::common::MutexGuard gd(mMutex);

		TicketAttr ta;
		ta.ticketId	= ticketId;
		ta.usedBW	= requestBW;		
		mTickets.insert( TicketS::value_type( ticketId , ta ) );
		glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NssC2Transfer,"getAllTicket() get ticket[%s] linkId[%s] requestBW[%lld]"),
			ticketId.c_str(), ownerLinkId.c_str() , requestBW);		
	}
}

void NssC2Transfer::validateC2TransStorConf( const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NssC2Transfer, "validating C2Transfer StorageLink[%s]'s attributes"), identStr );

	StorageLinkAttr attr;
	attr.linkId	= identStr;
	
	//get total config bandwidth
	Ice::Long totalBW =0;
	ZQTianShan::Util::getValueMapDataWithDefault( configPD, "TotalBandwidth", 0, totalBW );
	attr.maxBW	= totalBW * 1000;
	attr.usedBW = 0 ;

	//get current service state;
	Ice::Int iServiceState = TianShanIce::stNotProvisioned;
	ZQTianShan::Util::getValueMapDataWithDefault( configPD, _SERVICE_STATE_KEY_, TianShanIce::stNotProvisioned, iServiceState );

	if( iServiceState != TianShanIce::stInService )
	{		
		filteroutUsedBWForC2TransferConf( identStr , attr );
		/// can only lock mMutex after filteroutUsedBWForC2TransferConf, 
		/// or else dead locked

		// service is NOT in stInService state
		ZQ::common::MutexGuard gd(mMutex);		
		StorageLinkS::iterator itStorLink  = mStorLinks.find( identStr );
		if( itStorLink != mStorLinks.end() )
		{	//find it, update the attr
			itStorLink->second = attr;
			glog(ZQ::common::Log::L_INFO,CLOGFMT(NssC2Transfer, "update StorageLink[%s]: maxBW[%lld] useBW[%lld]"), identStr , attr.maxBW , attr.usedBW );
		}
		else
		{	//insert a new record
			mStorLinks.insert( StorageLinkS::value_type( identStr, attr) );
			glog(ZQ::common::Log::L_INFO,CLOGFMT(NssC2Transfer,"add StorageLink[%s]: maxBW[%lld] useBW[%lld]"), identStr , attr.maxBW , attr.usedBW );
		}
	}
	else
	{
		ZQ::common::MutexGuard gd( mMutex );
		StorageLinkS::iterator itStorLink  = mStorLinks.find( identStr );
		if( itStorLink != mStorLinks.end() )
		{//find it, update the attr
			attr.usedBW = itStorLink->second.usedBW;
			itStorLink->second = attr;
			glog(ZQ::common::Log::L_INFO,CLOGFMT(NssC2Transfer,"update StorageLink[%s]: maxBW[%lld] useBW[%lld]"), identStr , attr.maxBW , attr.usedBW );
		}
		else
		{	//insert a new record
			mStorLinks.insert( StorageLinkS::value_type( identStr ,attr ) );
			glog(ZQ::common::Log::L_INFO,CLOGFMT(NssC2Transfer,"add StorageLink[%s]: maxBW[%lld] useBW[%lld]"), identStr , attr.maxBW , attr.usedBW );
		}
	}

}

// void NssC2Transfer::validateC2TransStandbyConf( const char* identStr, ::TianShanIce::ValueMap& configPD)
// {
// }

void NssC2Transfer::validateConfiguration( const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD )
{
	if (NULL == type)
		type = "NULL";

	if (0 == strcmp(STORLINK_TYPE_C2TRANSFER, type) || 0 == strcmp(STORLINK_TYPE_C2TRANSFER_STANDBY, type) || 0 == strcmp(STORLINK_TYPE_C2OVERAQUA, type))
		validateC2TransStorConf(identStr, configPD);
	else
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NssC2Transfer",1001,"unsupported type [%s] in NssC2Transfer", type);

	configPD.erase( _SERVICE_STATE_KEY_ );

}

Ice::Int NssC2Transfer::evaluateC2Trans( LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost  )
{
	Ice::Int newCost = oldCost;

	const std::string& sessId = sessCtx.sessId;
	const std::string& linkId = linkInfo.linkIden.name;

	glog(ZQ::common::Log::L_DEBUG,CLOGFMT( NssC2Transfer,"evaluateC2Trans() enter evaluation for sess[%s] link[%s] with oldCost[%d]"),
			sessId.c_str() , linkId.c_str(), oldCost );
	if ( oldCost > ::TianShanIce::Transport::MaxCost )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(NssC2Transfer,"evaluateC2Trans() sess[%s] oldCost is bigger than outOfService cost"), sessId.c_str());
		return oldCost;
	}

	//get request bandwidth
	Ice::Long requestBW = 0;
	ZQTianShan::Util::getResourceDataWithDefault( linkInfo.rcMap ,TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth" , 0, requestBW );
	requestBW = max( requestBW , 0 );

	Ice::Long hintBw = -1;
	ZQTianShan::Util::getValueMapDataWithDefault( hintPD , PD_FIELD(PathTicket, bandwidth) , -1, hintBw );
	if( hintBw >= 0 )
	{
		requestBW = max(hintBw,requestBW);		
	}	
	glog(ZQ::common::Log::L_INFO,CLOGFMT(NssC2Transfer,"evaluateC2Trans() sess[%s] link[%s] ,requestBW[%lld] "), sessId.c_str() , linkId.c_str() , requestBW );

	Ice::Long maxBW		= 0;
	Ice::Long usedBW	= 0;

	{
		ZQ::common::MutexGuard gd( mMutex );
		StorageLinkS::const_iterator itLinkAttr = mStorLinks.find( linkId );
		if( itLinkAttr == mStorLinks.end() )
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(NssC2Transfer,"evaluateC2Trans() sess[%s] link[%s], can't find storage link with id[%s]"),
				sessId.c_str() , linkId.c_str() ,linkId.c_str() );
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
		maxBW		= itLinkAttr->second.maxBW;
		usedBW		= itLinkAttr->second.usedBW;

		if( maxBW <= 0 )
		{
			newCost = 0;//means unlimited
			glog(ZQ::common::Log::L_INFO,CLOGFMT(NssC2Transfer,"evaluateC2Trans() sess[%s] link[%s], this link's maxBW is set to unlimited, return cost[0]"),
				sessId.c_str() , linkId.c_str() );
		}
		else
		{
			if( ( usedBW + requestBW ) > maxBW )
			{
				newCost = ::TianShanIce::Transport::OutOfServiceCost;
			}
			else
			{
				newCost = (Ice::Int)((usedBW * ::TianShanIce::Transport::MaxCost) / maxBW);
			}
		}		
	}
	//update request bandwidth into hintPD
	ZQTianShan::Util::updateValueMapData( hintPD, PathTicketPD_Field(bandwidth), requestBW);
	glog(ZQ::common::Log::L_INFO,CLOGFMT(NssC2Transfer,"evaluateC2Trans() sess[%s] link[%s], got cost[%d] with requestBW[%lld] maxBW[%lld] usedBW[%lld]"),
		sessId.c_str() , linkId.c_str() , newCost , requestBW, maxBW, usedBW );
	return newCost;

}

Ice::Int NssC2Transfer::doEvaluation( LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost )
{
	const std::string& linktype = linkInfo.linkType;
	if ( 0 == linktype.compare(STORLINK_TYPE_C2TRANSFER) || 0 == linktype.compare(STORLINK_TYPE_C2TRANSFER_STANDBY) || 0 == linktype.compare(STORLINK_TYPE_C2OVERAQUA))
		return evaluateC2Trans( linkInfo, sessCtx , hintPD , oldCost );

	return ::TianShanIce::Transport::OutOfServiceCost;
}

IStorageLinkPHO::NarrowResult NssC2Transfer::doNarrow( const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx )
{
	const std::string& ticketId		= ticket->ident.name;
	const std::string& sessId		= sessCtx.sessId;
	const std::string& linkId		= ticket->storageLinkIden.name;

	glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NssC2Transfer,"doNarrow() sess[%s] link[%s] ticket[%s]"), sessId.c_str() , linkId.c_str() , ticketId.c_str() );
	Ice::Long requestBW = 0;
	ZQTianShan::Util::getResourceDataWithDefault( ticket->resources , TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth" , 0, requestBW );

	ZQTianShan::Util::updateValueMapData( ticket->privateData , PathTicketPD_Field(ownerStorageLinkId) , linkId );
	ZQTianShan::Util::updateValueMapData( ticket->privateData , PathTicketPD_Field(ownerSessionId) , sessId );
	
	{
		ZQ::common::MutexGuard gd(mMutex);
		//get storage link attribute
		StorageLinkS::iterator it = mStorLinks.find( linkId );
		if( it == mStorLinks.end() )
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(NssC2Transfer,"doNarrow() sess[%s] link[%s] ticket[%s] can't find storage link attribute"),
				sessId.c_str() , linkId.c_str() , ticketId.c_str());
			return NR_Error;
		}
		const Ice::Long& maxBW		= it->second.maxBW;
		Ice::Long& usedBW			= it->second.usedBW;
		if( maxBW <= 0 )
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(NssC2Transfer,"doNarrow() sess[%s] link[%s] ticket[%s] link has unlimited bandwidth"),
				sessId.c_str() , linkId.c_str() , ticketId.c_str());
			return NR_Narrowed;
		}
		else
		{
			if( ( usedBW + requestBW ) > maxBW )
			{
				glog(ZQ::common::Log::L_ERROR,CLOGFMT(NssC2Transfer,"doNarrow() sess[%s] link[%s] ticket[%s] not enough bandwidth for requestBW[%lld] , maxBW[%lld] usedBW[%lld]"),
					sessId.c_str() , linkId.c_str() , ticketId.c_str(), requestBW , maxBW , usedBW );
				return NR_Error;
			}
			else
			{
				TicketAttr ta;
				ta.ticketId		= ticketId;
				ta.usedBW		= requestBW;

				//insert new ticket record 
				mTickets[ticketId] = ta;
				//update used bandwidth for storage link attribute
				usedBW			+= requestBW;
			}
		}
	}

	//update linkId and linkType to this ticket	
	ZQTianShan::Util::updateValueMapData( ticket->privateData , PathTicketPD_Field(ownerStorageLinkType) , std::string(STORLINK_TYPE_C2TRANSFER) );
	//update request bandwidth to this ticket
	ZQTianShan::Util::updateValueMapData( ticket->privateData , PathTicketPD_Field(bandwidth) , requestBW );
	

	return NR_Narrowed;


}
void NssC2Transfer::doFreeResources( const ::TianShanIce::Transport::PathTicketPtr& ticket )
{
	std::string		ticketId = ticket->ident.name;
	std::string		sessId;
	std::string		linkId;

	std::string		ownerLinkType;
	
	ZQTianShan::Util::getValueMapDataWithDefault( ticket->privateData, PathTicketPD_Field(ownerStorageLinkType) , "" , ownerLinkType );
	ZQTianShan::Util::getValueMapDataWithDefault( ticket->privateData, PathTicketPD_Field(ownerStorageLinkId) , "" , linkId );
	ZQTianShan::Util::getValueMapDataWithDefault( ticket->privateData, PathTicketPD_Field(ownerSessionId) , "" , sessId );

	//ownerSessionId
	if( ownerLinkType != std::string(STORLINK_TYPE_C2TRANSFER)) //  // && ownerLinkType != std::string(STORLINK_TYPE_C2TRANSFER_STANDBY))
	{
		glog(ZQ::common::Log::L_DEBUG,CLOGFMT(NssC2Transfer,"doFreeResources() sess[%s] link[%s] ticket[%s] has no owner link type ,maybe this ticket was not narrowed"),
			sessId.c_str() , linkId.c_str() , ticketId.c_str() );

		return;
	}

	{
		ZQ::common::MutexGuard gd( mMutex );
		
		TicketS::iterator itTicket = mTickets.find( ticketId );
		if( itTicket == mTickets.end() )
		{
			return;
		}
		Ice::Long requestBW = itTicket->second.usedBW;
		//erase the ticket record
		mTickets.erase( itTicket );	

		

		StorageLinkS::iterator itLink = mStorLinks.find( linkId );
		if( itLink == mStorLinks.end())
		{
			return;			
		}
		const Ice::Long& maxBW	= itLink->second.maxBW;
		Ice::Long& usedBW	= itLink->second.usedBW;

		usedBW -= requestBW;

		glog(ZQ::common::Log::L_INFO,CLOGFMT(NssC2Transfer,"doFreeResources() sess[%s] link[%s] ticket[%s] free bandwidth [%lld], maxBW[%lld] usedBW[%lld]"),
			sessId.c_str() , linkId.c_str() , ticketId.c_str() , requestBW, maxBW , usedBW );				
	}
}

void NssC2Transfer::doCommit( const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx )
{
	//do nothing
}

}}

