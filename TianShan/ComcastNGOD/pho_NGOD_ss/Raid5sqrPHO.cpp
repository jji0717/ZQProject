// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: Raid5sqrPHO.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 


#include "Raid5sqrPHO.h"
#include "Log.h"
#include "public.h"
namespace ZQTianShan {
namespace AccreditedPath {

///schema for STORLINK_TYPE_RAID5SQR
static ConfItem Raid5_conf[] = {
	{ "TotalBandwidth",	::TianShanIce::vtLongs,			false, NULL},
	{ NULL, ::TianShanIce::vtInts, true, NULL },
};
#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
	if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
	else glog(ZQ::common::Log::L_WARNING, CLOGFMT(Raid5sqrPHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
	}

Raid5sqrPHO::Raid5sqrPHO(IPHOManager& mgr)
	: IStorageLinkPHO(mgr)
{
	_phoManager=&mgr;
	//_helperMgr.registerStorageLinkHelper(STORLINK_TYPE_RAID5SQR, *this, NULL);
}

Raid5sqrPHO::~Raid5sqrPHO()
{
	_helperMgr.unregisterStorageLinkHelper(STORLINK_TYPE_RAID5SQR);
}

bool Raid5sqrPHO::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;
	
	// address the schema definition
	if (0 == strcmp(type, STORLINK_TYPE_RAID5SQR))
		config = Raid5_conf;

	// no matches
	if (NULL == config)
		return false;

	// copy the schema to the output
	for (::ZQTianShan::ConfItem *item = config; item && item->keyname; item ++)
	{
		::TianShanIce::PDElement elem;
		elem.keyname = item->keyname;
		elem.optional = item->optional;
		elem.defaultvalue.type= item->type;

#pragma message ( __MSGLOC__ "TODO: prepare the default values here")

		schema.push_back(elem);
	}

	return true;
}

void Raid5sqrPHO::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
		type = "NULL";

	if (0 == strcmp(STORLINK_TYPE_RAID5SQR, type))
		validate_Raid5sLinkConfig(identStr, configPD);

	else ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>("Raid5sqrPHO",0,"unsupported type [%s] in IpEdgePHO", type);
}

Ice::Int Raid5sqrPHO::doEvaluation(::TianShanIce::Transport::StorageLinkPrx& link, 
								   const TianShanIce::SRM::SessionPrx& sess, 
								   TianShanIce::ValueMap& hintedPD,  
								   TianShanIce::SRM::ResourceMap& rcMap ,
								   const ::Ice::Int oldCost)
{
	std::string linktype;
	if (!link)
	{		
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	
	linktype = link->getType();

	if (0 == linktype.compare(STORLINK_TYPE_RAID5SQR))
		return eval_Raid5sLink(link, sess, hintedPD,rcMap, oldCost);

	return ::TianShanIce::Transport::OutOfServiceCost;
}

void Raid5sqrPHO::validate_Raid5sLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	glog(ZQ::common::Log::L_DEBUG, 
		CLOGFMT(IpEdgePHO, "validate a Raid5^2 StorageLink's Configuration: link[%s]"), identStr);

#ifdef _DEBUG
	::ZQTianShan::dumpValueMap(configPD, "  " STORLINK_TYPE_RAID5SQR ": ");
#endif _DEBUG
	

	Raid5sAttr ra;
	ra._storageLinkID=identStr;
	::TianShanIce::Variant val;
	try
	{
		ZQ::common::MutexGuard gd(_raid5sattrmapLocker);
		val=PDField(configPD,"TotalBandwidth");
		//ASSETVARIANT(val,::TianShanIce::vtLongs);
		if(val.type != TianShanIce::vtLongs)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Raid5sqrPHO",0,"TotalBandwidth should be vtLongs");
		}
		ra._totalBandwidth=val.lints[0]*1000;
		ra._availBandwidth=ra._totalBandwidth;
		Raid5sAttrMap::iterator itAttr=_raid5sAttrmap.find(ra._storageLinkID);
		//find out all associated storage link and remove used bandwidth
			::TianShanIce::Transport::StorageLinkPrx storLinkPrx=_phoManager->openStorageLink(identStr);
			if(storLinkPrx)
			{
				IdentCollection idc;
				try
				{
					idc=_phoManager->listPathTicketsByLink(storLinkPrx);
				}
				catch (Ice::ObjectNotExistException&)
				{
					idc.clear();
				}
				IdentCollection::const_iterator itID=idc.begin();
				for(;itID!=idc.end();itID++)
				{
					::TianShanIce::Transport::PathTicketPrx ticketprx=_phoManager->openPathTicket(*itID);
					if(ticketprx)
					{
						//TianShanIce::ValueMap data=ticketprx->getPrivateData();
						TianShanIce::SRM::ResourceMap dataMap=ticketprx->getResources();

						::TianShanIce::Variant bandwidth;
						try
						{
							TianShanIce::SRM::ResourceMap::const_iterator it=dataMap.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
							if(it==dataMap.end())
							{
								glog(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig ticket %s but no bandwidth resource"),itID->name.c_str());
							}
							else
							{		
								TianShanIce::ValueMap val=it->second.resourceData;
								bandwidth=PDField(val,PathTicketPD_Field(bandwidth));
								glog(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig() ticket %s used bandwidth %lld"),itID->name.c_str(),bandwidth.lints[0]);
								ra._availBandwidth -= bandwidth.lints[0];
							}
						}
						catch(::TianShanIce::InvalidParameter& e)
						{
							glog(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig() invalidParameter exception is caught:%s"),e.message.c_str());
						}
						catch (...)
						{

						}
					}
				}
			}
			
			
		if(itAttr==_raid5sAttrmap.end())
		{
			_raid5sAttrmap.insert(std::make_pair<std::string,Raid5sAttr>(ra._storageLinkID,ra));
			glog(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig() insert a new raid5s attr with storageLink id=%s and availableBW=%lld(BPS) TotalBW=%lld(BPS)"),
				identStr,ra._availBandwidth,ra._totalBandwidth);
						
		}
		else
		{
			itAttr->second=ra;
			glog(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig() update raid5s attr with storageLink id=%s and availableBW=%lld(BPS) TotalBW=%lld(BPS)"),
				identStr,ra._availBandwidth,ra._totalBandwidth);
		}
	}
	catch(...)
	{
	}
}

Ice::Int Raid5sqrPHO::eval_Raid5sLink(::TianShanIce::Transport::StorageLinkPrx& link, 
									  const TianShanIce::SRM::SessionPrx& sess, 
									  TianShanIce::ValueMap& hintedPD,
									  TianShanIce::SRM::ResourceMap& rcMap ,
									  const ::Ice::Int oldCost)
{
	::Ice::Long usedBW =0;
	::Ice::Int newCost = oldCost;
	std::string sessId;
	if(!sess)
	{
		sessId="";
	}
	else	
	{
		sessId=sess->getId();
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(Raid5sqrPHO, "eval_Raid5sLink() Session[%s] enter with oldCost=%d"),sessId.c_str(),oldCost);

	if (!link)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] no StorageLink passed in"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	if (newCost >= ::TianShanIce::Transport::MaxCost)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s]oldCost is bigger than outOfService cost"),sessId.c_str());
		return newCost;
	}
	
	::Ice::Long totalBW =0;
	
	::TianShanIce::SRM::ResourceMap resourceMap;
	
	::Ice::Long bw2Alloc = 0;
	if (sess)
	{
		try
		{
			resourceMap = sess->getReources();

			// try to get the bandwidth requirement from the session context
			if (resourceMap.end() != resourceMap.find(::TianShanIce::SRM::rtTsDownstreamBandwidth))
			{
				::TianShanIce::SRM::Resource& tsDsBw = resourceMap[::TianShanIce::SRM::rtTsDownstreamBandwidth];
				if (tsDsBw.resourceData.end() != tsDsBw.resourceData.find("bandwidth") && !tsDsBw.resourceData["bandwidth"].bRange && tsDsBw.resourceData["bandwidth"].lints.size() !=0)
					bw2Alloc = tsDsBw.resourceData["bandwidth"].lints[0];
				else
					glog(ZQ::common::Log::L_WARNING, 
							CLOGFMT(Raid5sqrPHO, "eval_IPStreamLink() Session[%s] unacceptable rtTsDownstreamBandwidth in session: bandwidth(range=%d, size=%d)"),
							sessId.c_str(),tsDsBw.resourceData["bandwidth"].bRange, tsDsBw.resourceData["bandwidth"].lints.size());
			}
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(Raid5sqrPHO, "eval_Raid5sLink() Session[%s] can not query the given session for resource info, stop evaluation"),sessId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(Raid5sqrPHO, "eval_Raid5sLink() no session specified, use hinted private data only"));
	}

	// step 2, adjust if the hintedPD also specify the bandwidth to the max of them
	if (hintedPD.end() != hintedPD.find(PD_FIELD(PathTicket, bandwidth)))
	{
		::TianShanIce::Variant& var = hintedPD[PD_FIELD(PathTicket, bandwidth)];
		if (var.type == TianShanIce::vtLongs && var.lints.size() >0)
			bw2Alloc = (bw2Alloc >0) ? max(bw2Alloc, var.lints[0]) : var.lints[0];
	}

	// step 2.1, double check if the bandwidth is valid
	if (bw2Alloc <= 0)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Raid5sqrPHO, "eval_Raid5sLink() Session[%s] 0 bandwidth has been specified, quit evaluation"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	// the following evaluation is only based on bandwidth. we should extend for future purposes
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(Raid5sqrPHO, "eval_Raid5sLink() Session[%s] required bandwidth:%lldbps"),sessId.c_str(),bw2Alloc);	


	{
		ZQ::common::MutexGuard gd(_raid5sattrmapLocker);
		Raid5sAttrMap::iterator it=_raid5sAttrmap.find(link->getIdent().name);
		if(it==_raid5sAttrmap.end())
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] can't find raid5sattr through storagelink id is %s"),
				sessId.c_str(),link->getIdent().name.c_str());
			return ::TianShanIce::Transport::MaxCost;
		}
		totalBW=it->second._totalBandwidth;
		usedBW=totalBW - it->second._availBandwidth;
		glog(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] now totalBandWidth is %lld and available bandwidth is %lld"),
								sessId.c_str(),totalBW,it->second._availBandwidth);

		if (::TianShanIce::Transport::MaxCost > newCost && totalBW >0)
		{
			if(totalBW - usedBW < bw2Alloc)
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] not enough bandwithmRequiredBW=%lld availableBW=%lld totalBW=%lld"),
						sessId.c_str(),bw2Alloc,it->second._availBandwidth,totalBW);
				newCost = ::TianShanIce::Transport::OutOfServiceCost;
				return newCost;
			}
			
			
			newCost = (::Ice::Int) (usedBW * ::TianShanIce::Transport::MaxCost / totalBW);
			it->second._availBandwidth-=bw2Alloc;
			TianShanIce::Variant val;
			val.lints.clear();
			val.lints.push_back(bw2Alloc);
			val.bRange=false;
			val.type=TianShanIce::vtLongs;
			hintedPD[PathTicketPD_Field(bandwidth)]=val;
			//do not need to put bandwidth resource into resourceMap
			glog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] after alloc bandwidth %lld  and now totalBandWidth is %lld , available bandwidth is %lld"),
				sessId.c_str(),bw2Alloc,totalBW,it->second._availBandwidth);
			
		}
		else
		{
			return TianShanIce::Transport::OutOfServiceCost;
		}
	}
	// step 4. return the higher cost
	glog(ZQ::common::Log::L_DEBUG,
		CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] return with cost=%d, oldCost=%d newCost=%d"),sessId.c_str(), max(oldCost, newCost),oldCost,newCost);
	return max(oldCost, newCost);
}

IPathHelperObject::NarrowResult Raid5sqrPHO::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const TianShanIce::SRM::SessionPrx& sess)
{
	TianShanIce::Variant val;
	try
	{
		val=GetResourceMapData(ticket->resources,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth");
	}
	catch (TianShanIce::InvalidParameter&)
	{
		glog(ZQ::common::Log::L_INFO,CLOGFMT(Raid5sqrPHO,"doNarrow() no bandwidth is found in resource,maybe invalid resources"));
		return NR_Error;
	}	
	glog(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"doNarrow() ticket with id=%s is narrowed band bandwidth is %lld"),
					ticket->getIdent().name.c_str(),val.lints[0]);
	{
		Ice::Identity ticketID=ticket->getIdent();
		ZQ::common::MutexGuard	gd(_raid5sattrmapLocker);
		ResourceRaidData rrd;
		TianShanIce::SRM::ResourceMap& resMap=ticket->resources;
		Ice::Long bw2Alloc=0;
		READ_RES_FIELD(bw2Alloc, resMap, rtTsDownstreamBandwidth, bandwidth, lints);
		rrd._usedBandwidth=bw2Alloc;
		_raidResourceMap[ticketID.name]=rrd;
	}
	return NR_Narrowed;
}
void Raid5sqrPHO::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
{
	try
	{
		ZQ::common::MutexGuard	gd(_raid5sattrmapLocker);
		TianShanIce::Transport::StorageLinkPrx storageLinkPrx=ticket->getStorageLink();
		if(!storageLinkPrx)
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"doFreeResource() no storagelink was found with ticket"));
			return;
		}
		std::string	strLinkType=storageLinkPrx->getType();		
		Ice::Identity ticketID=ticket->getIdent();
		if(strLinkType!=STORLINK_TYPE_RAID5SQR)
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"doFreeResource() invalid storage link type"));
			return;
		}
		TianShanIce::Variant val;
		try
		{
			val=GetResourceMapData(ticket->resources,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth");
		}
		catch (TianShanIce::InvalidParameter&)
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(Raid5sqrPHO,"doNarrow() no bandwidth is found in resource,maybe invalid resources"));
			return;
		}
		std::string	strLinkID=storageLinkPrx->getIdent().name;
		{
			
			ResourceRaidMap::iterator itAlloc=_raidResourceMap.find(ticketID.name);
			if(itAlloc==_raidResourceMap.end())
			{
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(Raid5sqrPHO,"doFreeResource() no allocated raid5s resource with ticketID=%s"),
					ticketID.name.c_str());
				return;
			}
			Raid5sAttrMap::iterator it=_raid5sAttrmap.find(strLinkID);
			if(it==_raid5sAttrmap.end())
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(Raid5sqrPHO,"doFreeResource() can't find raid5sattr through linkid=%s"),
					strLinkID.c_str());
				return;
			}
			it->second._availBandwidth+=val.lints[0];			
			glog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(Raid5sqrPHO,"doFreeResource() free raid5s resource from ticket %s "
				"with bandwidth=%lld and now totalbandwidth=%lld availBandwidth=%lld"),
				ticketID.name.c_str(),val.lints[0],it->second._totalBandwidth,it->second._availBandwidth);
			_raidResourceMap.erase(itAlloc);
		}
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"doFreeResource() catch a tianshanice exception:%s"),ex.message.c_str());
		return;
	}

	catch(Ice::Exception& e)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"doFreeResource() catch a ice exception:%s"),e.ice_name().c_str());
		return;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"doFreeResource() catch a unexpect exception"));
	}

}
void Raid5sqrPHO::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const TianShanIce::SRM::SessionPrx& sess)
{
	
}

}} // namespace

