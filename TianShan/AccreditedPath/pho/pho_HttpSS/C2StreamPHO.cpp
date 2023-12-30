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
// Ident : $Id: C2StreamPHO.cpp $
// Branch: $Name:  $
// Author: li.huang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/pho/pho_HttpSS/C2StreamPHO.cpp $
// 
// 7     1/11/16 4:58p Dejian.fei
// 
// 6     4/23/15 12:59p Zhiqiang.niu
// only need one locate url
// 
// 5     12/18/14 2:31p Hui.shao
// 
// 5     12/18/14 2:20p Hui.shao
// %i64d to %lld
// 
// 4     5/14/14 4:07p Li.huang
// 
// 3     5/12/14 3:42p Li.huang
// 
// 2     5/08/14 4:12p Li.huang
// 
// 1     5/08/14 3:32p Li.huang

// ===========================================================================

#include "C2StreamPHO.h"
#include "Log.h"
#include <public.h>
#include <time.h>
#include <algorithm>
#include <TianShanIceHelper.h>

extern ZQ::common::Log* gHttpSSLog;
#define MLOG (*gHttpSSLog)

namespace ZQTianShan {
namespace AccreditedPath {
	
///schema for STRMLINK_TYPE_IPEDGE_IP
static ConfItem HTTPSS_C2Stream[] = {
	{ "C2LocateURL",	    ::TianShanIce::vtStrings,	false,	"http://192.168.81.21:10080/vodadi.cgi;http://192.168.81.22:10080/vodadi.cgi",	false }, //use ; as a splitter
	{ "MaxSessionCount",	::TianShanIce::vtInts,		false,	"0",		false },
	{ "BandwidthKbps",		::TianShanIce::vtLongs,		false,	"0",	    false }, // in Kbps
	{ NULL,					::TianShanIce::vtInts,		true,	"" ,		false },
};

C2StreamPHO::C2StreamPHO(IPHOManager& mgr)
	: IStreamLinkPHO(mgr)
{
	_phoManager=&mgr;
	_helperMgr.registerStreamLinkHelper(STRMLINK_TYPE_HTTPSS_C2STREAM, *this, NULL);
}

C2StreamPHO::~C2StreamPHO()
{
	_helperMgr.unregisterStreamLinkHelper(STRMLINK_TYPE_HTTPSS_C2STREAM);
}

bool C2StreamPHO::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;
	
	// address the schema definition
	if (0 == strcmp(type, STRMLINK_TYPE_HTTPSS_C2STREAM))
		config = HTTPSS_C2Stream;

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
#pragma message ( __MSGLOC__ "TODO: prepare the default values here")		
		elem.defaultvalue.bRange = item->bRange;
		switch(item->type) 
		{
		case TianShanIce::vtInts:
			{				
				elem.defaultvalue.ints.clear();
				if(!elem.defaultvalue.bRange)
				{
					elem.defaultvalue.ints.push_back(atoi(item->hintvalue));
				}				
				else
				{
					int a,b;
					sscanf(item->hintvalue,"%d ~ %d",&a,&b);
					elem.defaultvalue.ints.push_back(a);
					elem.defaultvalue.ints.push_back(b);
				}
			}
			break;
		case TianShanIce::vtLongs:
			{
				elem.defaultvalue.lints.clear();
				if (!elem.defaultvalue.bRange)
				{					
					elem.defaultvalue.lints.push_back( _atoi64(item->hintvalue));
				}
				else
				{
					Ice::Long a,b;
					sscanf(item->hintvalue,FMT64" ~ "FMT64,&a,&b);
					elem.defaultvalue.lints.push_back(a);
					elem.defaultvalue.lints.push_back(b);
				}
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

void C2StreamPHO::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
		type = "NULL";

	if (0 == strcmp(STRMLINK_TYPE_HTTPSS_C2STREAM, type))
	{
		validate_C2StreamLinkConfig(identStr, configPD);
	}
	else
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"C2StreamPHO",1001,"unsupported type [%s] in C2StreamPHO for [%s] ", type , identStr );
	}
}

Ice::Int C2StreamPHO::doEvaluation(LinkInfo& linkInfo, 
								 const SessCtx& sessCtx,
								 TianShanIce::ValueMap& hintPD,
								 const ::Ice::Int oldCost)
{
	std::string &linktype =linkInfo.linkType;

	if (0 == linktype.compare(STRMLINK_TYPE_HTTPSS_C2STREAM))
		return eval_C2StreamLink(linkInfo, sessCtx, hintPD, linkInfo.rcMap,oldCost);

	return ::TianShanIce::Transport::OutOfServiceCost;
}


void C2StreamPHO::validate_C2StreamLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(C2StreamPHO, " enter validate a C2 StreamLink's Configuration: link[%s]"), identStr);

	C2StreamLinkAttr la;
	la._streamLinkID=identStr;
	::TianShanIce::Variant val;
	val.bRange=false;
	try
	{
		// get C2LocateURL
		bool bRange = false;
		ZQTianShan::Util::getValueMapData(configPD, "C2LocateURL", la._C2LocateURLs, bRange);

		if (la._C2LocateURLs.empty()) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("C2StreamPHO",1012,"Invalid C2LocateURL for streamLink[%s]",identStr);
		}
        // only need one locate url
        if (la._C2LocateURLs.size() > 1)
        {
            MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(C2StreamPHO,"only need one locate url, tailor it"));
            
            std::string locateUrl;
            locateUrl = la._C2LocateURLs[0];

            la._C2LocateURLs.clear();
            la._C2LocateURLs.push_back(locateUrl);

            ZQTianShan::Util::updateValueMapData(configPD, "C2LocateURL", la._C2LocateURLs, bRange);
        }

		::std::string C2LocateURL = "";

		int iSize = la._C2LocateURLs.size();
		for (int i = 0; i < iSize; i++)
		{
			C2LocateURL += la._C2LocateURLs[i];
			if (i < iSize - 1)
				C2LocateURL += ::std::string(", ");
		}

		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2StreamPHO,"C2Stream link[%s] get C2LocateURL[%s]"),identStr, C2LocateURL.c_str());

		// get TotalBandwidth
		val=PDField(configPD,"BandwidthKbps");
		if(val.type!=::TianShanIce::vtLongs)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("C2StreamPHO",1011,"Invalid TotalBandwidth type,should be vtLongs for StreamLink[%s]",identStr);
		}
		if (val.lints.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("C2StreamPHO",1012,"Invalid TotalBandwidth ,lints size() == 0  for streamLink[%s]",identStr);
		}
		la._totalBandwidth = val.lints[0]*1000;
		la._availableBandwidth = la._totalBandwidth;
		la._usedBandwidth = 0;
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2StreamPHO,"C2Stream link[%s] get totalBW[%lld]"),identStr,la._totalBandwidth);
		
		//get MaxSessionCount
		try
		{
			val=PDField(configPD,"MaxSessionCount");
			if (val.type == TianShanIce::vtInts && val.ints.size() > 0) 
			{
				la._maxSessionsCount = val.ints[0];
				la._usedSessionCount = 0;
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2StreamPHO,"C2Stream IP link[%s] get totalStreamCount[%d]"),identStr,la._maxSessionsCount);
			}
			else
			{
				la._maxSessionsCount = 0;
				la._usedSessionCount = 0;
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2StreamPHO,"C2Stream IP link[%s] ,invalid MaxSessionCount type or data,set it to default [0]"),identStr);
			}
		}
		catch (const ::TianShanIce::InvalidParameter& ) 
		{//maxStreamCount isn't found

			la._maxSessionsCount =0;//hard code the default value
			la._usedSessionCount = 0;
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2StreamPHO,"stream IP link[%s] ,MaxStreamCount is not found,set it to default [80]"),identStr);
		}


		ZQ::common::MutexGuard gd(_lockC2StreamLinkMap);
		C2StreamLinkAttrMap::iterator itAttr = _c2StreamLinkMap.find(la._streamLinkID);
		::TianShanIce::Transport::StreamLinkPrx strmLinkPrx = _phoManager->openStreamLink(identStr);
		if(strmLinkPrx)
		{
			IdentCollection idc;
			try
			{
				idc=_phoManager->listPathTicketsByLink(strmLinkPrx);
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
					if(la._maxSessionsCount != 0)
						la._usedSessionCount++;
					if(la._totalBandwidth == 0)
						continue;

					//TianShanIce::ValueMap data=ticketprx->getPrivateData();
					TianShanIce::SRM::ResourceMap dataMap=ticketprx->getResources();

					TianShanIce::Variant	bandwidth;
					try
					{
						TianShanIce::SRM::ResourceMap::const_iterator it=dataMap.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
						if(it==dataMap.end())
						{
							MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2StreamPHO,"validate_C2StreamLinkConfig ticket %s but no bandwidth resource"),itID->name.c_str());
						}
						else
						{	
							TianShanIce::ValueMap	PDData	=	ticketprx->getPrivateData();


							std::string ownerStreamlinkId;
							ZQTianShan::Util::getValueMapDataWithDefault(PDData,PathTicketPD_Field(ownerStreamLinkId),"",ownerStreamlinkId );
							if( ownerStreamlinkId.empty() )
							{
								MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(C2StreamPHO,"validate_C2StreamLinkConfig() ticket %s found, but no owner stream link id, skip"),
									itID->name.c_str());
								continue;
							}

							bandwidth=PDField(PDData,PathTicketPD_Field(bandwidth));
							if (bandwidth.lints.size()==0) 
							{
								MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2StreamPHO,"validate_C2StreamLinkConfig() ticket %s found a invalid bandwidth,maybe db error"),itID->name.c_str());
								continue;
							}
							MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2StreamPHO,"validate_C2StreamLinkConfig() ticket %s used bandwidth %lld"),itID->name.c_str(),bandwidth.lints[0]);

							la._availableBandwidth	-= bandwidth.lints[0];
							la._usedBandwidth		+= bandwidth.lints[0];

						}
					}
					catch(::TianShanIce::InvalidParameter& e)
					{
						MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"validate_C2StreamLinkConfig() invalidParameter exception is caught:%s"),e.message.c_str());
					}
					catch (...)
					{
					}
				}
			}
		}

		if(itAttr==_c2StreamLinkMap.end())
		{//list all ticket by link and get it		
			_c2StreamLinkMap.insert(std::make_pair<std::string, C2StreamLinkAttr>(la._streamLinkID,la));
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2StreamPHO,"validate_C2StreamLinkConfig() insert a C2Stream link with streamlink id =%s totalBW=%lld usedBW=%lld totalSessionCount=%d usedSessionCount=%d"),
				identStr,la._totalBandwidth,la._usedBandwidth,la._maxSessionsCount,la._usedSessionCount);
		}
		else
		{
			la._usedBandwidth	= itAttr->second._usedBandwidth;
			la._usedSessionCount	= itAttr->second._usedSessionCount;

			itAttr->second		= la;
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2StreamPHO,"validate_C2StreamLinkConfig() update a C2Stream link with streamlink id =%s totalBW=%lld usedBW=%lld totalSessionCount=%d usedSessionCount=%d"),
					identStr,la._totalBandwidth,la._usedBandwidth,la._maxSessionsCount,la._usedSessionCount);
			}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	catch (const ::Ice::Exception& e)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"validate_C2StreamLinkConfig() catch a ice exception :%s"),e.ice_name().c_str());
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"validate_C2StreamLinkConfig() unexpect error"));
	}
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(C2StreamPHO, "leave validate a C2Stream link Configuration: link[%s]"), identStr);
}



#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
  if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
 else MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(C2StreamPHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}
void C2StreamPHO::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{
	try
	{
		TianShanIce::ValueMap& ticketPD = ticket->privateData;  

		///get streamlinkID
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkId->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2StreamPHO,"doCommit() failed to get streamlink from ticket"));
			return ;
		}
		std::string strStreamlinkID = itLinkId->second.strs[0];
		std::string sessionId = sessCtx.sessId;

		MLOG(ZQ::common::Log::L_INFO, CLOGFMT (C2StreamPHO , "doCommit() [%s#%s]" ), strStreamlinkID.c_str(), sessionId.c_str());
	}
	catch (...){}
}
IPathHelperObject::NarrowResult C2StreamPHO::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{
	if (!ticket)
		return NR_Unrecognized;

	std::string		type;
	::TianShanIce::Transport::StreamLinkPrx strmLink;
	try 
	{
		strmLink = _helperMgr.openStreamLink(ticket->streamLinkIden);
		type = strmLink->getType();
	}
	catch(...) 
	{
		strmLink = NULL;
		type = "";
	}
	if ( 0 == type.compare( STRMLINK_TYPE_HTTPSS_C2STREAM ) )
	{
		return narrow_C2StreamLink( strmLink , sessCtx , ticket );
	}

	MLOG( ZQ::common::Log::L_ERROR, CLOGFMT(C2StreamPHO, "doNarrow() unrecognized stream link type [%s]"), type.c_str() );
	return IPathHelperObject::NR_Unrecognized;
}

Ice::Int C2StreamPHO::eval_C2StreamLink(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap ,const ::Ice::Int oldCost)
{
	Ice::Long lstart  = ZQTianShan::now();
	Ice::Int	newCost = oldCost;
	const std::string sessId = sessCtx.sessId;

	::TianShanIce::Transport::StreamLinkPrx strmLinkPrx = NULL;
	try
	{
		strmLinkPrx = ::TianShanIce::Transport::StreamLinkPrx::checkedCast(linkInfo.linkPrx);
	}
	catch (::Ice::Exception &ex)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(C2StreamPHO,"eval_C2StreamLink()SessID[%s] failed to get Streamlink proxy caught exception(%s)"), sessId.c_str(), ex.ice_name().c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(C2StreamPHO,"eval_C2StreamLink()SessID[%s] failed to get Streamlink proxy caught unknown exception"), sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(C2StreamPHO , "eval_C2StreamLink()SessID[%s] enter evaluation with oldCost[%d]" ), sessId.c_str(),  oldCost);
	if ( newCost > ::TianShanIce::Transport::MaxCost )
	{	
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(C2StreamPHO, "eval_C2StreamLink()SessID[%s] return oldCost[%d] because oldCost >= MaxCost"),sessId.c_str(), newCost);
		return newCost;
	}
    
	// step 1. read the link private data that need to be evaluated, please refer to 
	Ice::Long totalBW =0;
	Ice::Long usedBW =0;
	int	totalSessionCount = 0;
	int usedSessionCount = 0;

	ZQ::common::MutexGuard gd(_lockC2StreamLinkMap);
	C2StreamLinkAttrMap::iterator it=_c2StreamLinkMap.find(linkInfo.linkIden.name);
	if(it==_c2StreamLinkMap.end())
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"Session[%s] Can't find the streamlink attr through the id %s"),sessId.c_str(),linkInfo.linkIden.name.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	totalBW = it->second._totalBandwidth;
	usedBW  = it->second._usedBandwidth;
	usedSessionCount = it->second._usedSessionCount;
	totalSessionCount = it->second._maxSessionsCount;

	// step 1.2 get resource information from session
	TianShanIce::SRM::ResourceMap resourceMap = sessCtx.resources;
	Ice::Long bw2Alloc = 0;

	// try to get the bandwidth requirement from the session context
	READ_RES_FIELD(bw2Alloc, resourceMap, rtTsDownstreamBandwidth, bandwidth, lints);

	// step 2 adjust if the hintPD also specify parameters

	// step 2.1, adjust if the hintPD also specify the bandwidth to the max of them
	if (hintPD.end() != hintPD.find(PathTicketPD_Field(bandwidth)))
	{
		::TianShanIce::Variant& var = hintPD[PathTicketPD_Field(bandwidth)];
		if (var.type == TianShanIce::vtLongs && var.lints.size() >0)
			bw2Alloc = (bw2Alloc >0) ? max(bw2Alloc, var.lints[0]) : var.lints[0];
	}

	// step 2.5, double check if the bandwidth is valid
	if (totalBW > 0 && bw2Alloc <= 0)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(C2StreamPHO, "eval_C2StreamLink() Session[%s] 0 bandwidth has been specified, quit evaluation"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	// step 3. start evaluation.
	//         the following evaluation is only based on bandwidth for pure IP streaming
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2StreamPHO, "eval_C2StreamLink() Session[%s] requested allocation: bandwidth:%lldbps"),sessId.c_str(), bw2Alloc);

	{
		if(totalBW > 0 &&  bw2Alloc > (totalBW - usedBW) )
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(C2StreamPHO,"Session[%s] no enought bandwidth Required bw=%lld and used BW=%lld totalBW=%lld"),
				sessId.c_str(),bw2Alloc,usedBW,totalBW);
			return TianShanIce::Transport::OutOfServiceCost;
		}
		if (totalSessionCount > 0 && totalSessionCount <= usedSessionCount ) 
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(C2StreamPHO,"Session[%s] no enough bandwidth Required TotalSessionCount[%d] usedSessionCount[%d],no available SessionCount "),
				sessId.c_str(), totalSessionCount ,usedSessionCount);
			return TianShanIce::Transport::OutOfServiceCost;
		}

		// step 3.1. find all the allocation by StreamLinks
		MLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT(C2StreamPHO,"Session[%s] current available bandwidth is %lld and needed bandwidth is %lld"),
			sessId.c_str(),totalBW-usedBW,bw2Alloc);
	}

	// step 3.3. calculate the new cost based on the bandwidth
	if(totalBW > 0)
	{
		if (::TianShanIce::Transport::MaxCost > newCost)
		{
			if(totalBW - usedBW < bw2Alloc)
				newCost = ::TianShanIce::Transport::OutOfServiceCost;
			else if (usedBW > 0)
				newCost = (::Ice::Int) (usedBW * ::TianShanIce::Transport::MaxCost / totalBW);
			else 
				newCost=0;
		}
	}
	else
		newCost = 0;

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2StreamPHO,"Session[%s] bandwidth cost[%d] with usedBandwidth[%lld] totalBandwidth[%lld]"),
		sessId.c_str(),newCost,usedBW,totalBW);

	// step 3.4. calculate the new cost based on the maxSessionCount
	int sessionCountCost = TianShanIce::Transport::MaxCost;
	if(totalSessionCount > 0)
		sessionCountCost = usedSessionCount* TianShanIce::Transport::MaxCost / totalSessionCount;
	else
		sessionCountCost = 0;

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2StreamPHO,"Session[%s] streamcount cost[%d] with usedStreamCount[%d] totalStreamCount[%d]"),
		sessId.c_str(),sessionCountCost,usedSessionCount,totalSessionCount);

	newCost = newCost > sessionCountCost ? sessionCountCost : newCost;
	// end of the evaluation
	newCost = max(oldCost, newCost);

	if (newCost <= ::TianShanIce::Transport::MaxCost)
	{
		// sounds like a potential allocation, fill in the hintedPD with link information
		try
		{
			//fill useful resource data into rcMap 
			::TianShanIce::SRM::Resource res;
			res.attr	=	TianShanIce::SRM::raMandatoryNegotiable;

			::TianShanIce::Variant value;
			value.bRange = false;

			res.resourceData.clear();
			//a. c2locateURL
			value.type = TianShanIce::vtStrings;
			value.strs.clear();
			value.strs = it->second._C2LocateURLs;

			std::string C2LocateURL = "";
			int iSize = it->second._C2LocateURLs.size();
			for (int i = 0; i < iSize; i++)
			{
				C2LocateURL += it->second._C2LocateURLs[i];
				if (i < iSize - 1)
					C2LocateURL += ::std::string(", ");
			}
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(C2StreamPHO,"eval_C2StreamLink() Session[%s] set C2LocateURL to %s"),
				sessId.c_str(), C2LocateURL.c_str());

			res.resourceData["sessionInterface"]=value;
			res.status=TianShanIce::SRM::rsAssigned;
			if(rcMap.find(TianShanIce::SRM::rtStreamer) != rcMap.end())
			{
				rcMap[TianShanIce::SRM::rtStreamer].resourceData["sessionInterface"] = value;
			}
			else
				rcMap[TianShanIce::SRM::rtStreamer]=res;	
			hintPD[PathTicketPD_Field(sessionInterface)] = value;
		}
		catch(...)
		{
			MLOG(ZQ::common::Log::L_ERROR, 
				CLOGFMT(C2StreamPHO, "eval_C2StreamLink() Session[%s] exception caught while fillin the ticket private data"),
				sessId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2StreamPHO, "eval_C2StreamLink()[%s] do Evaluation with newCost=%d took %dms"), sessId.c_str(), max(oldCost,newCost), ZQTianShan::now() - lstart);

	// step 5. return the higher as the cost
	return max(oldCost, newCost);
}
IPathHelperObject::NarrowResult C2StreamPHO::narrow_C2StreamLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
								 const SessCtx& sessCtx,
								 const TianShanIce::Transport::PathTicketPtr& ticket)
{

	std::string sessId = sessCtx.sessId;

	std::string	ticketID = ticket->ident.name;

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2StreamPHO,"Session[%s] narrowed ticket with ticketid=%s"),sessId.c_str(),ticketID.c_str());

	std::string	strmLinkId = "";
	IPathHelperObject::NarrowResult narrowresult = NR_Error;
	try
	{
		strmLinkId = strmLink->getIdent().name;
	}
	catch (const Ice::Exception& ex) 
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(C2StreamPHO, "session[%s]ticketId[%s] failed to get streamID caught expception(%s)"), sessId.c_str(), ticketID.c_str(),ex.ice_name().c_str());
		return narrowresult;
	}
	catch (...) 
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(C2StreamPHO, "session[%s]ticketId[%s] failed to get streamID caught expception"), sessId.c_str(), ticketID.c_str());
		return narrowresult;
	}

	{
		C2TicketAttr rid;
		const TianShanIce::ValueMap& priData = ticket->privateData;
		TianShanIce::SRM::ResourceMap& resMap=ticket->resources;
		Ice::Long	bw2Alloc = 0;
		Ice::Long	bwTotal = 0;
		Ice::Long	bwUsed = 0;

		READ_RES_FIELD(bw2Alloc, resMap, rtTsDownstreamBandwidth, bandwidth, lints);
		std::string	strLinkIdent = strmLinkId ; //strmLink->getIdent().name;
		{
			ZQ::common::MutexGuard gd(_lockC2StreamLinkMap);
			C2StreamLinkAttrMap::iterator itC2Streammap = _c2StreamLinkMap.find(strLinkIdent);
			if ( itC2Streammap == _c2StreamLinkMap.end())
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"narrow_C2StreamLink() Session[%s] No streamlink with ID[%s] is found"),
					sessId.c_str(),strLinkIdent.c_str());
				return NR_Error;
			}
			C2StreamLinkAttr& c2StreamLinkAttr = itC2Streammap->second;

			if (c2StreamLinkAttr._maxSessionsCount > 0  && c2StreamLinkAttr._usedSessionCount >= c2StreamLinkAttr._maxSessionsCount) 
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"narrow_C2StreamLink() Session[%s] not enough sessioncount,totalSessionCount[%d] usedSessionCount[%d] for strmLink[%s]"),
					sessId.c_str() , c2StreamLinkAttr._maxSessionsCount, c2StreamLinkAttr._usedSessionCount, strLinkIdent.c_str()	);
				return NR_Error;
			}				

			if (c2StreamLinkAttr._totalBandwidth > 0 && c2StreamLinkAttr._usedBandwidth + bw2Alloc > c2StreamLinkAttr._totalBandwidth) 
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"narrow_C2StreamLink() Session[%s] not enough bandwidth,totalBandwidth[%d] usedBandwidth[%d] for strmLink[%s]"),
					sessId.c_str() , c2StreamLinkAttr._totalBandwidth ,c2StreamLinkAttr._usedBandwidth , strLinkIdent.c_str()	);
				return NR_Error;
			}

			bwTotal = c2StreamLinkAttr._totalBandwidth;
			c2StreamLinkAttr._usedSessionCount ++ ;		
			c2StreamLinkAttr._usedBandwidth += bw2Alloc;
            rid._usedBandwidth = bw2Alloc;

			bwUsed = c2StreamLinkAttr._usedBandwidth;
			rid._C2LocateURLs = c2StreamLinkAttr._C2LocateURLs;
		}

		TianShanIce::Variant varLinkType;
		TianShanIce::Variant varLinkId;

		varLinkType.bRange = false;
		varLinkType.type = TianShanIce::vtStrings;
		varLinkType.strs.clear();
		varLinkType.strs.push_back( STRMLINK_TYPE_HTTPSS_C2STREAM );
		ticket->privateData[PathTicketPD_Field(ownerStreamLinkType)] = varLinkType;

		varLinkId.bRange = false;
		varLinkId.type = TianShanIce::vtStrings;
		varLinkId.strs.clear();
		varLinkId.strs.push_back( strLinkIdent );
		ticket->privateData[PathTicketPD_Field(ownerStreamLinkId)] = varLinkId;

		{
			ZQ::common::MutexGuard gd(_lockC2StreamLinkMap);
			_c2ticketAttrMap[ticketID] = rid;			
		}
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2StreamPHO,"narrow_C2StreamLink() Session[%s] narrowed with NeedBW[%lld]"
			"  and UsedBW[%lld] TotalBW[%lld] "),sessId.c_str() , bw2Alloc , bwUsed , bwTotal);

	}
	return NR_Narrowed;		
}
void C2StreamPHO::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
{
	Ice::Long lstart  = ZQTianShan::now();
	std::string strStreamlinkID="";
	Ice::Identity ticketID;
	try
	{
		TianShanIce::ValueMap& ticketPD = ticket->privateData;
		///get streamlinkType
		TianShanIce::ValueMap::const_iterator itLinkType = ticketPD.find( PathTicketPD_Field(ownerStreamLinkType) ) ;
		if ( itLinkType == ticketPD.end()  || itLinkType->second.type != TianShanIce::vtStrings ||itLinkType->second.strs.size() == 0  ) 
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"doFreeResource() no ticket owner link type is found"));
			return;
		}
		std::string strLinkType = itLinkType->second.strs[0];

		///get streamlinkID
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkId->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"doFreeResource() can' get streamlink from ticket"));
			return ;
		}
		strStreamlinkID = itLinkId->second.strs[0];

		///get ticket Identity;
		ticketID = ticket->ident;

		if ( strcmp( STRMLINK_TYPE_HTTPSS_C2STREAM , strLinkType.c_str() ) == 0 ) 
		{
			MLOG(ZQ::common::Log::L_INFO, CLOGFMT (C2StreamPHO , "doFreeResource() streamLink[%s] ticketId[%s] free resource" ),
				strStreamlinkID.c_str(), ticketID.name.c_str());

			ZQ::common::MutexGuard gd(_lockC2StreamLinkMap);
			C2StreamTicketAttrMap::iterator itAlloc=_c2ticketAttrMap.find(ticketID.name);
			if(itAlloc==_c2ticketAttrMap.end())
			{
				MLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2StreamPHO,"doFreeResource() streamlink [%s] ticket[%s] no allocated C2Stream resource"),
					strStreamlinkID.c_str() , ticketID.name.c_str() );
				return;
			}			
			//TianShanIce::ValueMap value=ticket->getPrivateData();
			C2StreamLinkAttrMap::iterator it = _c2StreamLinkMap.find(strStreamlinkID);
			if( it == _c2StreamLinkMap.end() )
			{
				MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(C2StreamPHO,"doFreeResource() streamlink [%s] ticket[%s] can't find strmlink attr "),
					strStreamlinkID.c_str(), ticketID.name.c_str());
				return;
			}
			C2StreamLinkAttr& c2StreamLinkAttr = it->second;
			C2TicketAttr& rid = itAlloc->second;

			c2StreamLinkAttr._usedBandwidth -= rid._usedBandwidth;
			c2StreamLinkAttr._usedSessionCount -- ;
            //adjust usedBandwith and usedSessionCount;
			c2StreamLinkAttr._usedBandwidth		= c2StreamLinkAttr._usedBandwidth > 0 ? c2StreamLinkAttr._usedBandwidth : 0 ;
			c2StreamLinkAttr._usedSessionCount	= c2StreamLinkAttr._usedSessionCount > 0 ? c2StreamLinkAttr._usedSessionCount : 0 ;

			MLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(C2StreamPHO,"doFreeResource() streamlink [%s] ticket[%s]  "
				"free C2 Stream resource with bandwidth=[%lld] ,"
				"now totalbandwidth=[%lld] usedBandwidth=[%lld] totalSessionCount=[%d] usedSessionCount=[%d]"),
				strStreamlinkID.c_str (),
				ticketID.name.c_str(),
				rid._usedBandwidth,					
				c2StreamLinkAttr._totalBandwidth,
				c2StreamLinkAttr._usedBandwidth,
				c2StreamLinkAttr._maxSessionsCount,
				c2StreamLinkAttr._usedSessionCount);

			_c2ticketAttrMap.erase(itAlloc);
			////////////////////////////////////////////////////////////
		}
		else
		{
			MLOG(ZQ::common::Log::L_WARNING , CLOGFMT(C2StreamPHO,"unrecognized stream link type [%s]"), strLinkType.c_str() );
		}
	}
	catch( const Ice::Exception& e)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"doFreeResource() caught exception(%s)"),e.ice_name().c_str());
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2StreamPHO,"doFreeResource() caught unknown exception(%d)"), SYS::getLastErr());
	}	

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT (C2StreamPHO , "doFreeResource() Leave free resource streamlink [%s] ticket[%s]took %d ms" ), 
		strStreamlinkID.c_str(),ticketID.name.c_str(), ZQTianShan::now() - lstart);

}

}} // namespace




