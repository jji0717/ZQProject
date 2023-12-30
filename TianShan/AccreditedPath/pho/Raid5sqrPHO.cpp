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
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/pho/Raid5sqrPHO.cpp $
// 
// 6     1/11/16 4:57p Dejian.fei
// 
// 5     12/12/13 1:32p Hui.shao
// %lld
// 
// 4     9/05/13 3:26p Zonghuan.xiao
// modify log from glog to mlog because glog cannot confirm which pclog
// used
// 
// 3     3/10/11 2:25p Fei.huang
// 
// 2     3/08/11 12:28p Hongquan.zhang
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 33    08-12-02 18:20 Hongquan.zhang
// 
// 32    08-12-02 18:15 Hongquan.zhang
// 
// 31    08-07-08 16:00 Hongquan.zhang
// 
// 29    08-03-18 14:49 Hongquan.zhang
// check in for StreamLink Penalty and DVBC share link
// 
// 28    08-01-03 18:22 Hongquan.zhang
// 
// 27    08-01-03 15:55 Hui.shao
// move PDSchema namespace
// 
// 26    07-12-14 11:38 Hongquan.zhang
// update Error Code
// 
// 25    07-11-19 11:49 Hongquan.zhang
// 
// 24    07-10-25 14:09 Hongquan.zhang
// 
// 23    07-09-18 12:55 Hongquan.zhang
// 
// 22    07-08-30 15:47 Hongquan.zhang
// 
// 21    07-06-18 10:24 Hongquan.zhang
// 
// 20    07-06-06 16:08 Hongquan.zhang
// 
// 19    07-05-24 11:19 Hongquan.zhang
// 
// 18    07-03-22 17:24 Hongquan.zhang
// 
// 17    07-03-21 13:57 Hongquan.zhang
// 
// 16    07-03-14 12:33 Hongquan.zhang
// 
// 15    07-03-07 14:48 Hongquan.zhang
// 
// 14    07-01-11 16:06 Hongquan.zhang
// 
// 13    07-01-09 15:10 Hongquan.zhang
// 
// 12    07-01-05 10:59 Hongquan.zhang
// 
// 11    06-12-28 16:43 Hongquan.zhang
// 
// 10    06-12-25 12:22 Hongquan.zhang
// 
// 9     06-10-09 13:54 Hongquan.zhang
// 
// 8     06-09-18 19:26 Hui.shao
// use plugin for PHO
// 
// 7     06-09-05 12:44 Hui.shao
// 
// 6     06-08-28 18:14 Hui.shao
// 
// 5     06-08-25 14:28 Hui.shao
// 
// 4     06-08-10 12:46 Hui.shao
// moved bandwidth into the privateData
// 
// 1     06-08-08 16:55 Hui.shao
// ===========================================================================

#include "Raid5sqrPHO.h"
#include "Log.h"
#include <public.h>
#include <TianShanIceHelper.h>


extern ZQ::common::Log* pho_Seachangelog;
#define MLOG (*pho_Seachangelog)

namespace ZQTianShan {
namespace AccreditedPath {

///schema for STORLINK_TYPE_RAID5SQR
static ConfItem Raid5_conf[] = {
	{ "TotalBandwidth",	::TianShanIce::vtLongs,			false, "",false},
	{ NULL, ::TianShanIce::vtInts, true, "" ,false},
};
#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
	if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
	else MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(Raid5sqrPHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
	}

Raid5sqrPHO::Raid5sqrPHO(IPHOManager& mgr)
	: IStorageLinkPHO(mgr)
{
	_phoManager=&mgr;
#ifndef PHO_SEACHANGE_EXPORTS // if is a plugin, let the InitPHO do this
	_helperMgr.registerStorageLinkHelper(STORLINK_TYPE_RAID5SQR, *this, NULL);
#endif // PHO_SEACHANGE_EXPORTS
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

	else 
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Raid5sPHO",1001,"unsupported type [%s] in IpEdgePHO", type);
	
	configPD.erase( _SERVICE_STATE_KEY_ );		
}

Ice::Int Raid5sqrPHO::doEvaluation(LinkInfo& linkInfo,
								   const SessCtx& sessCtx, 
								   TianShanIce::ValueMap& hintPD,
								   const ::Ice::Int oldCost)
{
	std::string& linktype=linkInfo.linkType;
	if (0 == linktype.compare(STORLINK_TYPE_RAID5SQR))
		return eval_Raid5sLink(linkInfo, sessCtx, hintPD,linkInfo.rcMap, oldCost);

	return ::TianShanIce::Transport::OutOfServiceCost;
}

void Raid5sqrPHO::validate_Raid5sLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	MLOG(ZQ::common::Log::L_DEBUG, 
		CLOGFMT(IpEdgePHO, "validate a Raid5^2 StorageLink's Configuration: link[%s]"), identStr);

	Raid5sAttr ra;
	ra._storageLinkID=identStr;
	::TianShanIce::Variant val;
	try
	{
		
		val=PDField(configPD,"TotalBandwidth");
		if(val.type!=::TianShanIce::vtLongs)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Raid5ssqrPHO",1011,"Invalid totalBandwidth type,should be vtInts");
		}
		ra._totalBandwidth=val.lints[0]*1000;
		ra._availBandwidth=ra._totalBandwidth;
		ra._usedBandwidth = 0;
		
		Ice::Int iServiceState = TianShanIce::stNotProvisioned;
		ZQTianShan::Util::getValueMapDataWithDefault( configPD, _SERVICE_STATE_KEY_, TianShanIce::stNotProvisioned, iServiceState );

		if( iServiceState != TianShanIce::stInService )
		{
			ZQ::common::MutexGuard gd(_raid5sattrmapLocker);
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
						TianShanIce::ValueMap PDData=ticketprx->getPrivateData();
						TianShanIce::SRM::ResourceMap dataMap=ticketprx->getResources();

						::TianShanIce::Variant bandwidth;
						try
						{
							TianShanIce::SRM::ResourceMap::const_iterator it=dataMap.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
							if(it==dataMap.end())
							{
								MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig ticket %s but no bandwidth resource"),itID->name.c_str());
							}
							else
							{		
								//TianShanIce::ValueMap val=it->second.resourceData;
								bandwidth = PDField(PDData,PathTicketPD_Field(bandwidth));
								MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig() ticket %s used bandwidth %lld"),itID->name.c_str(),bandwidth.lints[0]);
								ra._availBandwidth -= bandwidth.lints[0];
								ra._usedBandwidth += bandwidth.lints[0];
								ResourceRaidData rid;
								rid._usedBandwidth = bandwidth.lints[0];
								{
									_raidResourceMap[itID->name] = rid;
								}
							}
						}
						catch(::TianShanIce::InvalidParameter& e)
						{
							MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig() invalidParameter exception is caught:%s"),e.message.c_str());
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
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig() insert a new raid5s attr with storageLink id=%s"),identStr);

			}
			else
			{
				itAttr->second=ra;
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig() update raid5s attr with storageLink id=%s"),identStr);
			}
		}
		else
		{
			ZQ::common::MutexGuard gd(_raid5sattrmapLocker);
			Raid5sAttrMap::iterator itAttr=_raid5sAttrmap.find(ra._storageLinkID);
			if(itAttr==_raid5sAttrmap.end())
			{
				_raid5sAttrmap.insert(std::make_pair<std::string,Raid5sAttr>(ra._storageLinkID,ra));
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig() insert a new raid5s attr with storageLink id=%s"),identStr);

			}
			else
			{
				ra._usedBandwidth	= itAttr->second._usedBandwidth;
				itAttr->second		= ra;
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(Raid5sqrPHO,"validate_Raid5sLinkConfig() update raid5s attr with storageLink id=%s"),identStr);
			}
		}
	}
	catch(...)
	{
	}
}

Ice::Int Raid5sqrPHO::eval_Raid5sLink(LinkInfo& linkInfo,
									  const SessCtx& sessCtx, 
									  TianShanIce::ValueMap& hintedPD, 
									  TianShanIce::SRM::ResourceMap& rcMap , 
									  const ::Ice::Int oldCost)
{
	::Ice::Long usedBW =0;
	::Ice::Int newCost = oldCost;
	std::string sessId=sessCtx.sessId;
//	if(!sess)
//	{
//		sessId="";
//	}
//	else	
//	{
//		sessId=sess->getId();
//	}
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(Raid5sqrPHO, "eval_Raid5sLink() Session[%s] enter with oldCost=%d"),sessId.c_str(),oldCost);



	if (newCost > ::TianShanIce::Transport::MaxCost)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s]oldCost is bigger than outOfService cost"),sessId.c_str());
		return newCost;
	}
	
	::Ice::Long totalBW =0;
	
	::TianShanIce::SRM::ResourceMap resourceMap = sessCtx.resources;
	
	::Ice::Long bw2Alloc = 0;
	if (1)
	{
		try
		{

			// try to get the bandwidth requirement from the session context
			if (resourceMap.end() != resourceMap.find(::TianShanIce::SRM::rtTsDownstreamBandwidth))
			{
				::TianShanIce::SRM::Resource& tsDsBw = resourceMap[::TianShanIce::SRM::rtTsDownstreamBandwidth];
				if (tsDsBw.resourceData.end() != tsDsBw.resourceData.find("bandwidth") && !tsDsBw.resourceData["bandwidth"].bRange && tsDsBw.resourceData["bandwidth"].lints.size() !=0)
					bw2Alloc = tsDsBw.resourceData["bandwidth"].lints[0];
				else
					MLOG(ZQ::common::Log::L_WARNING, 
							CLOGFMT(Raid5sqrPHO, "eval_IPStreamLink() Session[%s] unacceptable rtTsDownstreamBandwidth in session: bandwidth(range=%d, size=%d)"),
							sessId.c_str(),tsDsBw.resourceData["bandwidth"].bRange, tsDsBw.resourceData["bandwidth"].lints.size());
			}
		}
		catch (...)
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Raid5sqrPHO, "eval_Raid5sLink() Session[%s] can not query the given session for resource info, stop evaluation"),sessId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	else
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(Raid5sqrPHO, "eval_Raid5sLink() no session specified, use hinted private data only"));
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
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Raid5sqrPHO, "eval_Raid5sLink() Session[%s] 0 bandwidth has been specified, quit evaluation"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	// the following evaluation is only based on bandwidth. we should extend for future purposes
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(Raid5sqrPHO, "eval_Raid5sLink() Session[%s] required bandwidth:%lldbps"),sessId.c_str(),bw2Alloc);	


	{
		ZQ::common::MutexGuard gd(_raid5sattrmapLocker);
		Raid5sAttrMap::iterator it=_raid5sAttrmap.find(linkInfo.linkIden.name);
		if(it==_raid5sAttrmap.end())
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] can't find raid5sattr through storagelink id is %s"),
				sessId.c_str(),linkInfo.linkIden.name.c_str());
			return ::TianShanIce::Transport::MaxCost;
		}

		totalBW = it->second._totalBandwidth;
		usedBW	= it->second._usedBandwidth;
		MLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] now totalBandWidth is %lld and used bandwidth is %lld"),
								sessId.c_str(),totalBW,usedBW);

		if (::TianShanIce::Transport::MaxCost >= newCost && totalBW >0)
		{
			if(totalBW - usedBW < bw2Alloc)
			{
				MLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] not enough bandwith"
					" RequiredBW=[%lld] totalBW=[%lld] usedBW=[%lld]"),
						sessId.c_str(),bw2Alloc,totalBW,it->second._usedBandwidth);
				newCost = ::TianShanIce::Transport::OutOfServiceCost;
				return newCost;
				
			}			
			
			newCost = (::Ice::Int) (usedBW * ::TianShanIce::Transport::MaxCost / totalBW);
			
			//it->second._availBandwidth-=bw2Alloc;

			TianShanIce::Variant val;
			val.lints.clear();
			val.lints.push_back(bw2Alloc);
			val.bRange=false;
			val.type=TianShanIce::vtLongs;
			hintedPD[PathTicketPD_Field(bandwidth)]=val;
			//do not need to put bandwidth resource into resourceMap
			MLOG(ZQ::common::Log::L_DEBUG,
				CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] after alloc bandwidth [%lld] "
				" and now totalBandWidth is [%lld] , used bandwidth is [%lld]"),
				sessId.c_str(),bw2Alloc,totalBW,it->second._usedBandwidth);			
		}
		else
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] oldCost [%d] is bigger than MaxCost or TotalBW[%lld] <= 0"),
				sessId.c_str(),newCost,totalBW);
			return TianShanIce::Transport::OutOfServiceCost;
		}
	}
	// step 4. return the higher cost
	MLOG(ZQ::common::Log::L_DEBUG,
		CLOGFMT(Raid5sqrPHO,"eval_Raid5sLink() Session[%s] return with cost=[%d], oldCost=[%d] newCost=[%d]"),
		sessId.c_str(), max(oldCost, newCost),oldCost,newCost);
	return max(oldCost, newCost);
}

IPathHelperObject::NarrowResult Raid5sqrPHO::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket,
													  const SessCtx& sessCtx)
{

	::TianShanIce::Transport::StorageLinkPrx StrLinkPrx=_helperMgr.openStorageLink(ticket->storageLinkIden);
	if (!StrLinkPrx) 
	{
		MLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(Raid5sqrPHO,"doNarrow() can't get storageLink using storageLinkID[%s]"),
			ticket->storageLinkIden.name.c_str());
		return NR_Error;
	}
	TianShanIce::Variant val;
	try
	{
		val=GetResourceMapData(ticket->resources,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth");
	}
	catch (TianShanIce::InvalidParameter&)
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(Raid5sqrPHO,"doNarrow() no bandwidth is found in resource,maybe invalid resources"));
		return NR_Error;
	}	
	{
		Ice::Identity& ticketID=ticket->ident;
		ResourceRaidData rrd;
		TianShanIce::SRM::ResourceMap& resMap=ticket->resources;
		Ice::Long bw2Alloc=0;
		
	
		READ_RES_FIELD(bw2Alloc, resMap, rtTsDownstreamBandwidth, bandwidth, lints);
		Raid5sAttrMap::iterator itAttr =_raid5sAttrmap.end();
		{
			ZQ::common::MutexGuard	gd(_raid5sattrmapLocker);
			itAttr =_raid5sAttrmap.find(StrLinkPrx->getIdent().name);
			if (itAttr==_raid5sAttrmap.end()) 
			{
				MLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(Raid5sqrPHO,"doNarrow() can't Find storageLink [%s] in attr map"),
					ticket->storageLinkIden.name.c_str());
				return NR_Error;
			}
			if ( (itAttr->second._totalBandwidth - itAttr->second._usedBandwidth) <bw2Alloc ) 
			{
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(Raid5sqrPHO,"doNarrow() no available bandwidth total[%lld] usedBW[%lld] needBW[%lld]"),
					itAttr->second._totalBandwidth,itAttr->second._usedBandwidth,bw2Alloc);
				return NR_Error;
			}
			itAttr->second._usedBandwidth += bw2Alloc;		
			rrd._usedBandwidth=bw2Alloc;
			_raidResourceMap[ticketID.name]=rrd;
			
			TianShanIce::Variant varLinkType;
			TianShanIce::Variant varLinkId;
			
			varLinkType.bRange = false;
			varLinkType.type = TianShanIce::vtStrings;
			varLinkType.strs.clear();
			varLinkType.strs.push_back( STORLINK_TYPE_RAID5SQR );
			ticket->privateData[PathTicketPD_Field(ownerStorageLinkType)] = varLinkType;
			
			varLinkId.bRange = false;
			varLinkId.type = TianShanIce::vtStrings;
			varLinkId.strs.clear();
			varLinkId.strs.push_back( StrLinkPrx->getIdent().name );
			ticket->privateData[PathTicketPD_Field(ownerStorageLinkId)] = varLinkId;
		
			MLOG(ZQ::common::Log::L_DEBUG,
				CLOGFMT(Raid5sqrPHO,"doNarrow() ticket [%s] is narrowed with bandwidth [%lld] and totalBW[%lld] usedBW[%lld]"),
				ticketID.name.c_str(),val.lints[0],
				itAttr->second._totalBandwidth,itAttr->second._usedBandwidth);
		}
	}
	
	return NR_Narrowed;
}
void Raid5sqrPHO::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
{
	try
	{
		Ice::Identity& ticketID = ticket->ident;

		TianShanIce::ValueMap& ticketPD = ticket->privateData;
		TianShanIce::ValueMap::const_iterator itLinkType = ticketPD.find( PathTicketPD_Field(ownerStorageLinkType) ) ;
		if ( itLinkType == ticketPD.end()  || itLinkType->second.type != TianShanIce::vtStrings ||itLinkType->second.strs.size() == 0  ) 
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"no ticket owner link type is found for ticket [%s] , this ticket may not be narrowed "),ticketID.name.c_str());
			return;
		}
		std::string strLinkType = itLinkType->second.strs[0];
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStorageLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkType->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"no ticket owner link id is found for ticket [%s] , this ticket may not be narrowed"),ticketID.name.c_str());
			return;				 
		}
		std::string strStramlinkID = itLinkId->second.strs[0];

		if(strLinkType!=STORLINK_TYPE_RAID5SQR)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"doFreeResource() invalid storage link type"));
			return;
		}
		TianShanIce::Variant val;
		try
		{
			val=GetResourceMapData(ticket->resources,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth");
		}
		catch (TianShanIce::InvalidParameter&)
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(Raid5sqrPHO,"doNarrow() no bandwidth is found in resource,maybe invalid resources"));
			//return;
		}
		std::string&	strLinkID = strStramlinkID;
		{
			ZQ::common::MutexGuard	gd(_raid5sattrmapLocker);
			ResourceRaidMap::iterator itAlloc=_raidResourceMap.find(ticketID.name);
			if(itAlloc==_raidResourceMap.end())
			{
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(Raid5sqrPHO,"doFreeResource() no allocated raid5s resource with ticketID=%s"),ticketID.name.c_str());
				return;
			}
			Raid5sAttrMap::iterator it=_raid5sAttrmap.find(strLinkID);
			if(it==_raid5sAttrmap.end())
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"doFreeResource() can't find raid5sattr through linkid=%s"),strLinkID.c_str());
				return;
			}
			//it->second._availBandwidth+=val.lints[0];
			it->second._usedBandwidth -= itAlloc->second._usedBandwidth;

			MLOG(ZQ::common::Log::L_DEBUG,
				CLOGFMT(Raid5sqrPHO,"doFreeResource() free raid5s resource from ticket [%s] "
				" with bandwidth=[%lld] and now totalBW=[%lld] usedBW=[%lld]"),
				ticketID.name.c_str(),itAlloc->second._usedBandwidth,
				it->second._totalBandwidth,it->second._usedBandwidth);
		
			_raidResourceMap.erase(itAlloc);
			
		}
	}
	catch (TianShanIce::BaseException& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"doFreeResource() catch a tianshanice exception:%s"),ex.message.c_str());
		return;
	}

	catch(Ice::Exception& e)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"doFreeResource() catch a ice exception:%s"),e.ice_name().c_str());
		return;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(Raid5sqrPHO,"doFreeResource() catch a unexpect exception"));
	}

}
void Raid5sqrPHO::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{
	
}

}} // namespace

