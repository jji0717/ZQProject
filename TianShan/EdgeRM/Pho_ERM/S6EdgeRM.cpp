// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: S6EdgeRM.cpp $
// Branch: $Name:  $
// Author: Huang Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/Pho_ERM/S6EdgeRM.cpp $
// 
// 24    4/22/16 5:33p Dejian.fei
// 
// 23    1/11/16 5:53p Dejian.fei
// 
// 22    12/30/14 11:45a Li.huang
// 
// 21    10/16/14 4:56p Li.huang
// fix 454 no delete session
// 
// 20    8/22/14 10:07a Li.huang
// remove ICE DB
// 
// 19    5/27/14 9:34a Li.huang
// fix up s6 connection with base url
// 
// 18    4/04/14 11:09a Li.huang
// 
// 17    4/03/14 4:17p Li.huang
// 
// 16    4/03/14 2:07p Li.huang
// add default symbolrate
// 
// 15    4/01/14 3:27p Li.huang
// 
// 14    3/13/14 3:26p Li.huang
// modify glog
// 
// 13    9/23/13 11:43a Li.huang
// modify "%I64d" to "%lld" for printf Ice::long
// 
// 12    9/03/13 1:30p Bin.ren
// 
// 11    8/30/13 6:21p Bin.ren
// 
// 10    8/29/13 5:43p Bin.ren
// 
// 9     4/12/13 11:19a Li.huang
// 
// 8     11/01/12 3:31p Li.huang
// 
// 7     10/26/12 9:08a Li.huang
// 
// 6     10/25/12 5:24p Li.huang
// 
// 5     10/25/12 3:06p Li.huang
// 
// 4     10/25/12 2:56p Li.huang
// add sessionGroup
// 
// 3     9/28/12 2:34p Li.huang
// using new rtsplcient interface
// 
// 2     3/15/11 5:23p Fei.huang
// migrate to linux
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 3     10-06-01 16:06 Li.huang
// add minKey 
// 
// 2     10-02-09 15:28 Li.huang
// 
// 1     10-02-01 17:20 Li.huang
// add SeaChange.S6.EdgeRM StreamLink

// ===========================================================================
#include "S6EdgeRM.h"
#include "Log.h"
#include "strHelper.h"
#include "TianShanIceHelper.h"
#include "public.h"
#include <time.h>
#include <algorithm>
#include "S6Client.h"

extern ZQ::common::Config::Loader<PHOConfig>  _cfg;

namespace ZQTianShan {
namespace AccreditedPath {

	using namespace ZQTianShan::EdgeRM;

static ConfItem Cfg_S6EdgeRM[] = {
	{ "IP",				       ::TianShanIce::vtStrings,	false,	"",			false }, //QAM name for this streamlink
	{ "Port",			       ::TianShanIce::vtInts,	    false,	"",	        false},
	{ "Qam.ModulationFormat",  ::TianShanIce::vtInts,	    false,	"8",	    false},
	{ "Route-IDs",			   ::TianShanIce::vtStrings,	false,	"",	        false},
	{ "TotalBandWidth",		   ::TianShanIce::vtLongs,	    false,	"",	        false},//if TotalBandWidth <=0, 表示这个Link上没有带宽限制
	{ "SopName",			   ::TianShanIce::vtStrings,	false,	"",			false }, 
	{ "DefaultSymbolRate",			   ::TianShanIce::vtInts,	    false,	"6875000",	false }, 
	{ "QamType",			   ::TianShanIce::vtInts,	    false,	"0",	false }, 
	{ NULL,					   ::TianShanIce::vtInts,		true,	"",			false },
	};

class MyRandom 
{
public:
	ptrdiff_t operator() (ptrdiff_t max) 
	{
		static bool bInit = false;
		if( !bInit )
		{
			srand(time(NULL));
			bInit = true;
		}
		if( max  > 0 )
			return rand()%max;
		else 
			return 0;
	}
};

S6EdgeRMPHO::S6EdgeRMPHO(IPHOManager& mgr, ZQTianShan::EdgeRM::PhoEdgeRMEnv& env)
: IStreamLinkPHO(mgr), _env(env)
{
	_phoManager=&mgr;
}

S6EdgeRMPHO::~S6EdgeRMPHO()
{
	_helperMgr.unregisterStreamLinkHelper( STRMLINK_TYPE_TianShanS6ERM );
}
/// no problem
bool S6EdgeRMPHO::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;

	// address the schema definition
	if ( 0 == strcmp(type, STRMLINK_TYPE_TianShanS6ERM) )
		config = Cfg_S6EdgeRM;

	// no matches
	if (NULL == config)
		return false;

	// copy the schema to the output
	for (::ZQTianShan::ConfItem *item = config; item && item->keyname; item ++)
	{
		::TianShanIce::PDElement elem;
		elem.keyname			= item->keyname;
		//elem.optional			= item->optional;
		elem.optional2			= item->optional2;
		elem.defaultvalue.type	= item->type;
		elem.defaultvalue.bRange= item->bRange;
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
/// no problem
void S6EdgeRMPHO::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
	{
		type = "NULL";
	}
	if (0 == strcmp( STRMLINK_TYPE_TianShanS6ERM, type))
	{
		validate_S6EdgeRMStrmLinkConfig( identStr, configPD );
	}
	else
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1001,"unsupported type [%s] in S6EdgeRMPHO", type);
	}
}

void S6EdgeRMPHO::validate_S6EdgeRMStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "enter validate type[%s] Configuration: link[%s]"), STRMLINK_TYPE_TianShanS6ERM, identStr);
	S6EdgeRMLinkAttr lia;
	lia._streamLinkID	= identStr;
	lia._usedBandwidth = 0;
	lia._svcGrpId = -1;

	TianShanIce::Variant var;
	var.bRange			= false;
	try
	{
		//get rtsp IP address
		::std::string edgeRMIPAddress;
		try
		{
			::ZQTianShan::Util::getValueMapData(configPD, "IP", edgeRMIPAddress);
			lia._rtspIp = edgeRMIPAddress;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"StreamLink[%s] get IP[%s]"), identStr, edgeRMIPAddress.c_str());
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get edgeRMIPAddress caught exception: %s:%d:%s", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get edgeRMIPAddress", identStr);
		}
		//get rtsp port 
		::Ice::Int port = 0;
		try
		{
			::ZQTianShan::Util::getValueMapData(configPD, "Port", port);
			lia._rtspPort = port;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"StreamLink[%s] get Port[%d]"), identStr, port);
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get port caught exception: %s:%d:%s", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get port", identStr);
		}

		//get QAM modulationFormat
		::Ice::Int modulationFormat = 0;
		try
		{
			::ZQTianShan::Util::getValueMapData(configPD, "Qam.ModulationFormat", modulationFormat);
			lia._modulationFormat = modulationFormat;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"StreamLink[%s] get ModulationFormat[%d]"), identStr, modulationFormat);
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get ModulationFormat caught exception: %s:%d:%s", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get  ModulationFormat", identStr);
		}

	    //get Route-IDs
		//::std::string strQAMIDs;
		try
		{
			bool bRange = false;
			::ZQTianShan::Util::getValueMapData(configPD, "Route-IDs", lia._vecQAMID, bRange);

			::std::string strQAMIDList = "";
			int iSize = lia._vecQAMID.size();
			for (int i = 0; i < iSize; i++)
			{
				strQAMIDList += lia._vecQAMID[i];
				if (i < iSize - 1)
					strQAMIDList += ::std::string(", ");
			}
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"StreamLink[%s] get Route-IDs, vector is [%s]"), identStr, strQAMIDList.c_str());
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get Route-IDs caught exception: %s:%d:%s", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get Route-IDs", identStr);
		}

		//get TotalBandWidth 
		::Ice::Long totalBandwidth = 0;
		try
		{
			::ZQTianShan::Util::getValueMapData(configPD, "TotalBandWidth", totalBandwidth);
			lia._totalBandWidth = totalBandwidth;
			lia._availableBandwidth = totalBandwidth;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"StreamLink[%s] get totalBandwidth[%d]"), identStr, totalBandwidth);
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get totalBandwidth caught exception: %s:%d:%s", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get totalBandwidth", identStr);
		}

		//get SopName
		::std::string sopname;
		try
		{
			::ZQTianShan::Util::getValueMapDataWithDefault(configPD, "SopName", "" , sopname);
			lia._strSopName = sopname;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"StreamLink[%s] get SopName[%s]"), identStr, sopname.c_str());
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get SopName caught exception: %s:%d:%s", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get SopName", identStr);
		}

		//get SymbolRate
		Ice::Int symbolRate;
		try
		{
			::ZQTianShan::Util::getValueMapData(configPD, "DefaultSymbolRate", symbolRate);
			lia._symbolRate = symbolRate;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"StreamLink[%s] get symbolRate[%d]"), identStr, symbolRate);
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get symbolRate caught exception: %s:%d:%s", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get symbolRate", identStr);
		}

		//get QamType
		Ice::Int qamType;
		try
		{
			::ZQTianShan::Util::getValueMapDataWithDefault(configPD, "QamType",0 ,qamType);
			lia._qamType = qamType;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"StreamLink[%s] get qamType[%d]"), identStr, qamType);
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get qamType caught exception: %s:%d:%s", identStr, ex.category.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"StreamLink[%s] failed to get qamType", identStr);
		}


		TianShanIce::Transport::StreamLinkPrx strmLinkPrx = NULL;
		try
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"open StreamLink[%s]"), identStr);
			strmLinkPrx =_phoManager->openStreamLink(identStr);  
		}
		catch (Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("S6EdgeRMPHO",1011,"failed to open StreamLink[%s]", identStr);
		}

		if(strmLinkPrx)
		{
			fixupRouteIds(strmLinkPrx, identStr, lia);

			IdentCollection idc;
			try
			{
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"list PathTickets by StreamLink[%s]"), identStr);
				idc=_phoManager->listPathTicketsByLink(strmLinkPrx);
			}
			catch (Ice::ObjectNotExistException&)
			{
				idc.clear();
			}
			IdentCollection::const_iterator itID=idc.begin();
			for(;itID!=idc.end();itID++)
			{
//				envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"StreamLink[%s] open PathTickets[%s]"),identStr, itID->name.c_str());
				::TianShanIce::Transport::PathTicketPrx ticketprx=_phoManager->openPathTicket(*itID);
				if(ticketprx)
				{
					//TianShanIce::ValueMap data=ticketprx->getPrivateData();
					TianShanIce::SRM::ResourceMap dataMap;
					TianShanIce::Variant	bandwidth;
					TianShanIce::Variant    onDemandSession;

					try
					{
						TianShanIce::ValueMap	PDData	=	ticketprx->getPrivateData();
						dataMap = ticketprx->getResources();
						TianShanIce::SRM::ResourceMap::const_iterator it=dataMap.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
						if(it==dataMap.end())
						{
							envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(S6EdgeRMPHO,"validate ticket [%s] but no bandwidth resource"),itID->name.c_str());
						}
						else
						{	
							bandwidth=PDField(PDData,PathTicketPD_Field(bandwidth));
							if (bandwidth.lints.size()==0) 
							{
								envlog(ZQ::common::Log::L_WARNING,CLOGFMT(S6EdgeRMPHO,"validate ticket [%s] found invalid bandwidth,maybe db error"),itID->name.c_str());
								continue;
							}
							envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(S6EdgeRMPHO,"validate ticket [%s] used bandwidth %lld"),itID->name.c_str(),bandwidth.lints[0]);

							if(lia._totalBandWidth <= 0)
							{
								lia._availableBandwidth = lia._totalBandWidth;
							}
							else
							{
								lia._availableBandwidth	-= bandwidth.lints[0];
							}
							lia._usedBandwidth		+= bandwidth.lints[0];

							TicketAttr rid;
							rid._usedBanwidth	= bandwidth.lints[0];
							{
								_ticketAttrMap[itID->name] = rid;
							}
						}

						onDemandSession = PDField(PDData, PathTicketPD_Field(OnDemandSessionId));
						if (onDemandSession.strs.size()!=0 ) 
						{
							_env.addOnDemandSession(onDemandSession.strs[0]);
						}
					}
					catch(::TianShanIce::InvalidParameter& e)
					{
						envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"validate ticket [%s] caught exception [%s]"),itID->name.c_str(), e.message.c_str());
					}
					catch (...)
					{
					}
				}
			}
		}

		//update stream link attribute map
		{
			::ZQ::common::MutexGuard guard(_linkAttrMapMutex);
			S6EdgeRMLinkAttrMap::iterator iter = _linkAttrMap.find(identStr);
			if ( iter != _linkAttrMap.end() )
			{
				iter->second = lia;
			}
			else
			{
				_linkAttrMap[identStr] = lia;
			}
			sendSetParameter(lia);
		}
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"[%s]update S6EdgeRM streamlink"), identStr);
	}	
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	catch (const ::Ice::Exception& e)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"validat ticket  caught exception [%s]"), e.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"validate configuration caught unknown exception(%d)"), SYS::getLastErr());
	}
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "Leave validate [%s] Configuration: link[%s]"), STRMLINK_TYPE_TianShanS6ERM, identStr);
}

#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
	if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
 else envlog(ZQ::common::Log::L_WARNING, CLOGFMT(S6EdgeRMPHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}

Ice::Int S6EdgeRMPHO::doEvaluation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& ticketPrivateData, const ::Ice::Int oldCost)
{
   return eval_S6EdgeRMStrmLink(linkInfo, sessCtx, ticketPrivateData, linkInfo.rcMap,oldCost);
}
///
Ice::Int S6EdgeRMPHO::eval_S6EdgeRMStrmLink(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap, const ::Ice::Int oldCost)
{
	::Ice::Int newCost = oldCost;	
	std::string sessId = sessCtx.sessId;
	std::string streamLinkId = linkInfo.linkIden.name;
 	if (!linkInfo.linkPrx)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"streamLinkId[%s] session[%s] no Streamlink is attached,return with OutOfServiceCost"),streamLinkId.c_str(),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	ZQ::common::MutexGuard gd(_linkAttrMapMutex);
	S6EdgeRMLinkAttrMap::iterator it= _linkAttrMap.find(linkInfo.linkIden.name);
	if(it==_linkAttrMap.end())
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"streamLinkId[%s] session[%s] Can't find the streamlink attr"),streamLinkId.c_str(), sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	S6EdgeRMLinkAttr linktattr = it->second;

	Ice::Long lStart = ZQTianShan::now();
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO,"streamLinkId[%s] session[%s] enter evaluation with oldCost[%d]"),streamLinkId.c_str(), sessId.c_str(),oldCost);

	if(newCost > ::TianShanIce::Transport::MaxCost)
	{	
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"streamLinkId[%s] session[%s] return oldCost=%d because oldCost>=MaxCost"),streamLinkId.c_str(), sessId.c_str(), newCost);
		return newCost;
	}

	// step 1. read the link private data that need to be evaluated, please refer to 
	::Ice::Long totalBW =0;
	::Ice::Long usedBW =0;
	// step 1.2 get resource information from session
	::TianShanIce::SRM::ResourceMap resourceMap = sessCtx.resources;
	::Ice::Long bw2Alloc = 0;
	
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
	if (bw2Alloc <= 0)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "streamLinkId[%s] session[%s] 0 bandwidth has been specified, quit evaluation"),streamLinkId.c_str(), sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	// step 3. start evaluation.
	//         the following evaluation is only based on bandwidth for pure IP streaming
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO, "streamLinkId[%s] session[%s] requested allocation bandwidth:%lldbps"),streamLinkId.c_str(), sessId.c_str(), bw2Alloc);

	std::string	strConfigedDestMac = "";
	{
		totalBW = linktattr._totalBandWidth;
		usedBW  = linktattr._usedBandwidth;

		if(totalBW <=0)
		{
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(S6EdgeRMPHO,"streamLinkId[%s] session[%s] no bandwidth limited, needed bandwidth is %lld"),
				streamLinkId.c_str(), sessId.c_str(), bw2Alloc);
			newCost = 0;
		}
		else
		{
			if( bw2Alloc > (totalBW - usedBW))
			{
				envlog(ZQ::common::Log::L_ERROR,
					CLOGFMT(S6EdgeRMPHO,"streamLinkId[%s] session[%s] no enough bandwidth Required bw=%lld and used BW=%lld totalBW=%lld"),
					streamLinkId.c_str(), sessId.c_str(),bw2Alloc,usedBW,totalBW);
				return TianShanIce::Transport::OutOfServiceCost;
			}
			envlog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(S6EdgeRMPHO,"streamLinkId[%s] session[%s] current available bandwidth is %lld and needed bandwidth is %lld"),
				streamLinkId.c_str(), sessId.c_str(),totalBW-usedBW,bw2Alloc);
		}
	}

	// step 3.3. calculate the new cost based on the bandwidth
	if ( ::TianShanIce::Transport::MaxCost > newCost && totalBW >0 )
	{
		if(totalBW - usedBW < bw2Alloc)
			newCost = ::TianShanIce::Transport::OutOfServiceCost;
		else if (usedBW >0)
			newCost = (::Ice::Int) (usedBW * ::TianShanIce::Transport::MaxCost / totalBW);
		else 
			newCost=0;
	}

	envlog(ZQ::common::Log::L_INFO,CLOGFMT(S6EdgeRMPHO,"streamLinkId[%s] session[%s] bandwidth cost[%d] with usedBandwidth[%lld] totalBandwidth[%lld]"),
  streamLinkId.c_str(), sessId.c_str(),newCost,usedBW,totalBW);

	// end of the evaluation
	newCost = max(oldCost, newCost);

	if (newCost <= ::TianShanIce::Transport::MaxCost)
	{
		try
		{
			//fill useful resource data into rcMap 
			::TianShanIce::SRM::Resource res;
			res.attr	=	TianShanIce::SRM::raMandatoryNonNegotiable;
			res.status	=	TianShanIce::SRM::rsRequested;

			::TianShanIce::Variant value;
			value.bRange = false;

			//////////////////////////////////////////////////////////////////////////			
			// a. bandwidth
			res.resourceData.clear();
			value.type = TianShanIce::vtLongs;
			value.lints.clear();
			value.lints.push_back(bw2Alloc);
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO,"streamLinkId[%s] session[%s] set bandwidth to %lld"), streamLinkId.c_str(), sessId.c_str(),bw2Alloc);
			hintPD[PathTicketPD_Field(bandwidth)] = value;

			res.resourceData["bandwidth"]=value;
			res.status=TianShanIce::SRM::rsAssigned;
			rcMap[TianShanIce::SRM::rtTsDownstreamBandwidth]=res;	


			//set ownerStreamLinkType to ticket privatedata
			TianShanIce::Variant varStreamLinkType;
			varStreamLinkType.bRange = false;
			varStreamLinkType.type = TianShanIce::vtStrings;
			varStreamLinkType.strs.clear();
			varStreamLinkType.strs.push_back(STRMLINK_TYPE_TianShanS6ERM);
			hintPD[PathTicketPD_Field(ownerStreamLinkType)] = varStreamLinkType;

			//set ownerStreamLinkId to ticket privatedata	
			TianShanIce::Variant varStreamLinkId;
			varStreamLinkId.bRange = false;
			varStreamLinkId.type = TianShanIce::vtStrings;
			varStreamLinkId.strs.clear();
			varStreamLinkId.strs.push_back(linkInfo.linkIden.name);
			hintPD[PathTicketPD_Field(ownerStreamLinkId)] = varStreamLinkId;

			::TianShanIce::Variant var_SopName;
			///set sop_name mac
			var_SopName.type = ::TianShanIce::vtStrings;
			var_SopName.bRange = false;
			var_SopName.strs.clear();
			var_SopName.strs.push_back(linktattr._strSopName);
			hintPD[PathTicketPD_Field(sop_name)] = var_SopName;
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "streamLinkId[%s] session[%s] failded to set ticket privatedata caught unkonwn exception"), 
				streamLinkId.c_str(), sessId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"streamLinkId[%s] session[%s] return with newCost=%d took %d ms"),
		      streamLinkId.c_str(), sessId.c_str(),max(oldCost,newCost), ZQTianShan::now() - lStart);

	// step 4. return the higher as the cost
	return max(oldCost, newCost);
}

IPathHelperObject::NarrowResult S6EdgeRMPHO::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
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
	if (0 == type.compare( STRMLINK_TYPE_TianShanS6ERM ) )
	{
		return narrow_S6EdgeRMStrmLink( strmLink , sessCtx , ticket);
	}

	envlog( ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "doNarrow() unrecognized stream link type [%s]"), type.c_str() );
	return IPathHelperObject::NR_Unrecognized;
}

IPathHelperObject::NarrowResult S6EdgeRMPHO::narrow_S6EdgeRMStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink,
															 const SessCtx& sessCtx,
															 const TianShanIce::Transport::PathTicketPtr&  ticket)
{
	///setp1. get StreamLink Id
	IPathHelperObject::NarrowResult narrowresult = NR_Error;
	::std::string	strmLinkId = "", sessId = "", ticketID = "", s6SessId;
  
	Ice::Long bw2Alloc = 0;
	Ice::Long lStart = ZQTianShan::now();
    S6Session::Ptr s6Session = NULL;
	Ice::Long  totalBandWidth = 0;
	std::string sopname="";
	try
	{	
		sessId = sessCtx.sessId;
		ticketID = ticket->ident.name;
		strmLinkId = ticket->streamLinkIden.name;

		///get Bandwidth
		TianShanIce::SRM::ResourceMap& ticetRes= ticket->resources;
		//try to get the bandwidth requirement from the session context	
		READ_RES_FIELD(bw2Alloc, ticetRes, rtTsDownstreamBandwidth, bandwidth, lints);

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO,"stearmLink[%s] ticketId[%s] session[%s] enter narrow"),
			                   strmLinkId.c_str(), ticketID.c_str(), sessId.c_str());
		Ice::Byte modulation = 0;
		std::string strTransport;
		std::string s6Ip;
		Ice::Int s6Port;
		Ice::Int symbolRate;
		{
			ZQ::common::MutexGuard gd(_linkAttrMapMutex);
			S6EdgeRMLinkAttrMap::iterator it= _linkAttrMap.find(ticket->streamLinkIden.name);
			if(it==_linkAttrMap.end())
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"stearmLink[%s] ticketId[%s] session[%s] can't find the streamlink attr"),
					strmLinkId.c_str(), ticketID.c_str(), sessId.c_str());
				return IPathHelperObject::NR_Unrecognized;
			}
			S6EdgeRMLinkAttr& linktattr = it->second;
			sopname = linktattr._strSopName;

			if(linktattr._totalBandWidth > 0)
			{
				totalBandWidth = linktattr._totalBandWidth;
				if(linktattr._availableBandwidth - bw2Alloc >=0)
				{
					linktattr._usedBandwidth += bw2Alloc;
					if(linktattr._totalBandWidth > 0)
						linktattr._availableBandwidth -= bw2Alloc;
				}
				else
				{
					envlog(ZQ::common::Log::L_ERROR,
						CLOGFMT(S6EdgeRMPHO, "stearmLink[%s] ticketId[%s] session[%s] no available Badnwidth,totalBW[%lld] usedBW[%lld] availableBandwidth[%lld]"),
						strmLinkId.c_str(), ticketID.c_str(), sessId.c_str(), it->second._totalBandWidth,it->second._usedBandwidth, it->second._availableBandwidth);
					return narrowresult;
				}
			}
			fixupRouteIds(strmLink, strmLinkId, linktattr);
			///QAM modulation
			modulation = linktattr._modulationFormat;
			s6Ip = linktattr._rtspIp;
			s6Port = linktattr._rtspPort;
			symbolRate = linktattr._symbolRate;
			/// rtsp transport
			char strQAMList[512] = "";
			for (size_t i = 0; i < linktattr._vecQAMID.size(); i++)
			{
				if(linktattr._vecQAMID[i].size() > 0)
				{
					snprintf(strQAMList, 512, 
						"MP2T/DVBC/QAM;unicast;client=00AF123456DE;bandwidth="FMT64";qam_name=%s;modulation=%s;qam_type=%d,",
						bw2Alloc, linktattr._vecQAMID[i].c_str(), modulationInt2Str(modulation).c_str(),linktattr._qamType);
					strTransport+= strQAMList ;
					memset(strQAMList, 0, 512);
				}
			}
		}
		/// step 1.3  send request
		char baseURL[256];
		snprintf(baseURL, sizeof(baseURL)-2, "rtsp://%s:%d/",  s6Ip.c_str(), s6Port);

		S6Session::Ptr s6Session = S6SessionGroup::createSession(sessId.c_str(), baseURL, strmLinkId);
		if(NULL == s6Session)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "stearmLink[%s] ticketId[%s] session[%s] failed to create s6session"),
					strmLinkId.c_str(), ticketID.c_str(), sessId.c_str());
			if(totalBandWidth > 0)
			{
				ZQ::common::MutexGuard gd(_linkAttrMapMutex);
				S6EdgeRMLinkAttrMap::iterator it= _linkAttrMap.find(ticket->streamLinkIden.name);
				if(it !=_linkAttrMap.end())
				{
					S6EdgeRMLinkAttr& linktattr = it->second;
					linktattr._usedBandwidth -= bw2Alloc;
					linktattr._availableBandwidth += bw2Alloc;
				}
			}
			return narrowresult;
		}

		ZQTianShan::EdgeRM::S6Client* s6client = s6Session->getS6Client();
		if (!s6client)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "stearmLink[%s] ticketId[%s] session[%s] failed to get s6client connection "),
			                                                      strmLinkId.c_str(), ticketID.c_str(), sessId.c_str());
			if(totalBandWidth > 0)
			{
				ZQ::common::MutexGuard gd(_linkAttrMapMutex);
				S6EdgeRMLinkAttrMap::iterator it= _linkAttrMap.find(ticket->streamLinkIden.name);
				if(it !=_linkAttrMap.end())
				{
					S6EdgeRMLinkAttr& linktattr = it->second;
					linktattr._usedBandwidth -= bw2Alloc;
					linktattr._availableBandwidth += bw2Alloc;
				}
			}
			s6Session->destroy();
			return narrowresult;
		}

		std::string sessGroup = s6Session->getSessionGroupName();
		RTSPMessage::AttrMap headers;
		MAPSET(RTSPMessage::AttrMap, headers, "Content-Type",      "text/parameters");
		MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_REQUIRE, "com.comcast.ngod.s6");
		MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_SESSIONGROUP, sessGroup);
		MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_ENCRYPTIONTYPE, "N/A");
		MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_CASID, "N/A");
		MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_ENCRYPTCONTROL, "N/A");
		MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_TRANSPORT, strTransport.c_str());
		MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_ONDEMANDSESSIONID, sessId.c_str());
		MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_POLICY, "priority=1");
		MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_INBANDMARKER, "N/A");

		int currentCSeq = s6client->sendSETUP(*s6Session, NULL, NULL, headers);
		if (currentCSeq <= 0 || !s6client->waitForResponse(currentCSeq))
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "stearmLink[%s] ticketId[%s] session[%s] sessionGroup[%s] failed to send setup message with server timeout"),
				                                    strmLinkId.c_str(), ticketID.c_str(), sessId.c_str(), sessGroup.c_str());
			if(totalBandWidth > 0)
			{
				ZQ::common::MutexGuard gd(_linkAttrMapMutex);
				S6EdgeRMLinkAttrMap::iterator it= _linkAttrMap.find(ticket->streamLinkIden.name);
				if(it !=_linkAttrMap.end())
				{
					S6EdgeRMLinkAttr& linktattr = it->second;
					linktattr._usedBandwidth -= bw2Alloc;
					linktattr._availableBandwidth += bw2Alloc;
				}
			}
			s6Session->destroy();
			return narrowresult;
		}

		if(s6Session->_resultCode != 200)
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"fail to narrow stearmLink[%s] session[%s] sessionGroup[%s] with error[%s] took %d ms"), strmLinkId.c_str(), sessId.c_str(), sessGroup.c_str(), s6Session->_errorMsg.c_str(), ZQTianShan::now() - lStart);
			if(totalBandWidth > 0)
			{
				ZQ::common::MutexGuard gd(_linkAttrMapMutex);
				S6EdgeRMLinkAttrMap::iterator it= _linkAttrMap.find(ticket->streamLinkIden.name);
				if(it !=_linkAttrMap.end())
				{
					S6EdgeRMLinkAttr& linktattr = it->second;
					linktattr._usedBandwidth -= bw2Alloc;
					linktattr._availableBandwidth += bw2Alloc;
				}
			}
			s6Session->destroy();
			return narrowresult;
		}	
		s6SessId = s6Session->getSessionId();
		///setp2. insert resource data to ticket resource
		S6Session::QamInfo&  qamInfo = s6Session->getQamInfo();
		::TianShanIce::Variant var_QamIp;
		//set QAM ip
		var_QamIp.type = ::TianShanIce::vtStrings;
		var_QamIp.bRange = false;
		var_QamIp.strs.clear();
		var_QamIp.strs.push_back(qamInfo.edgeDeviceIP);
		ticket->privateData[PathTicketPD_Field(Qam.IP)] = var_QamIp;
		ticket->privateData[PathTicketPD_Field(destAddr)] = var_QamIp;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtEthernetInterface, "destIP", var_QamIp, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "session[%s]sessionGroup[%s]set destAddr to [%s]"), sessId.c_str(), sessGroup.c_str(), var_QamIp.strs[0].c_str());

		::TianShanIce::Variant var_QamMac;
		///set QAM mac
		var_QamMac.type = ::TianShanIce::vtStrings;
		var_QamMac.bRange = false;
		var_QamMac.strs.clear();
		var_QamMac.strs.push_back(qamInfo.edgeDeviceMac);
		ticket->privateData[PathTicketPD_Field(Qam.Mac)] = var_QamMac;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtEthernetInterface, "destMac", var_QamMac, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "session[%s]sessionGroup[%s]set destMac to [%s]"), sessId.c_str(), sessGroup.c_str(), var_QamMac.strs[0].c_str());

		::TianShanIce::Variant var_QambasePort;
		//set QAM.basePort
		var_QambasePort.type = ::TianShanIce::vtInts;
		var_QambasePort.bRange = false;
		var_QambasePort.ints.clear();
		var_QambasePort.ints.push_back(qamInfo.destPort);
		ticket->privateData[PathTicketPD_Field(Qam.basePort)] = var_QambasePort;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtEthernetInterface, "destPort", var_QambasePort, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "session[%s]sessionGroup[%s]set destPort to [%d]"), sessId.c_str(), sessGroup.c_str(), qamInfo.destPort);

		::TianShanIce::Variant var_QamSymbolRate;
		//set QAM symbolRate
		var_QamSymbolRate.type = ::TianShanIce::vtInts;
		var_QamSymbolRate.bRange = false;
		var_QamSymbolRate.ints.clear();
		if(qamInfo.symbolRate <= 0)
			qamInfo.symbolRate = symbolRate; 
		var_QamSymbolRate.ints.push_back(qamInfo.symbolRate);
		ticket->privateData[PathTicketPD_Field(Qam.symbolRate)] = var_QamSymbolRate;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtAtscModulationMode, "symbolRate", var_QamSymbolRate, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "session[%s]sessionGroup[%s]set symbolRate to [%d]"), sessId.c_str(), sessGroup.c_str(), qamInfo.symbolRate);

		::TianShanIce::Variant var_QamFrequency;
		//set QAM frequency
		var_QamFrequency.type = ::TianShanIce::vtInts;
		var_QamFrequency.bRange = false;
		var_QamFrequency.lints.clear();
		var_QamFrequency.lints.push_back(qamInfo.channelId);
		var_QamFrequency.ints.clear();
		var_QamFrequency.ints.push_back((Ice::Int)qamInfo.channelId);
		ticket->privateData[PathTicketPD_Field(Qam.frequency)] = var_QamFrequency;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtPhysicalChannel , "channelId", var_QamFrequency, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "session[%s]sessionGroup[%s]set channelId to [%d]"), sessId.c_str(), sessGroup.c_str(), (Ice::Int)qamInfo.channelId);

		::TianShanIce::Variant var_QamName;
		//set QAM name
		var_QamName.type = ::TianShanIce::vtStrings;
		var_QamName.bRange = false;
		var_QamName.strs.clear();
		var_QamName.strs.push_back(qamInfo.edgeDeviceName);
		ticket->privateData[PathTicketPD_Field(Qam.name)] = var_QamName;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceName", var_QamName,  sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "session[%s]sessionGroup[%s]set edgeDeviceName to [%s]"), sessId.c_str(), sessGroup.c_str(), var_QamName.strs[0].c_str());

		::TianShanIce::Variant var_QamZone;
		//set QAM deviceZone
		var_QamZone.type = ::TianShanIce::vtStrings;
		var_QamZone.bRange = false;
		var_QamZone.strs.clear();
		var_QamZone.strs.push_back(qamInfo.edgeDeviceZone);
		ticket->privateData[PathTicketPD_Field(Qam.Group)] = var_QamZone;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceZone", var_QamZone, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "session[%s]sessionGroup[%s]set edgeDeviceZone to [%s]"), sessId.c_str(), sessGroup.c_str(), var_QamZone.strs[0].c_str());

		::TianShanIce::Variant var_QamModulationFormat;
		//set QAM modulationformat
		var_QamModulationFormat.type = ::TianShanIce::vtInts;
		var_QamModulationFormat.bRange = false;
		var_QamModulationFormat.bin.clear();
		var_QamModulationFormat.bin.push_back(qamInfo.modulationFormat);
		var_QamModulationFormat.ints.clear();
		var_QamModulationFormat.ints.push_back((int)qamInfo.modulationFormat);
		ticket->privateData[PathTicketPD_Field(Qam.mode)] = var_QamModulationFormat;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtAtscModulationMode, "modulationFormat", var_QamModulationFormat, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "session[%s]sessionGroup[%s]set modulationFormat to [%d]"), sessId.c_str(), sessGroup.c_str(), qamInfo.modulationFormat);

		::TianShanIce::Variant var_QamPN;
		//set PN
		var_QamPN.type = ::TianShanIce::vtInts;
		var_QamPN.bRange = false;
		var_QamPN.lints.clear();
		var_QamPN.lints.push_back(qamInfo.PnId);
		var_QamPN.ints.clear();
		var_QamPN.ints.push_back((Ice::Int)qamInfo.PnId);
		ticket->privateData[PathTicketPD_Field(PN)] = var_QamPN;
		PutResourceMapData(ticket->resources, TianShanIce::SRM::rtMpegProgram, "Id", var_QamPN, sessId);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "session[%s]sessionGroup[%s]set pn to [%d]"), sessId.c_str(), sessGroup.c_str(), (Ice::Int)qamInfo.PnId);

		{
			::TianShanIce::Variant var;
			var.type = ::TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.push_back(sessGroup);
			ticket->privateData[PathTicketPD_Field(s6SessionGroup)] = var;
		}

		{
			::TianShanIce::Variant var;
			var.type = ::TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.push_back(sessId);
			ticket->privateData[PathTicketPD_Field(OnDemandSessionId)] = var;
		}

		{
			::TianShanIce::Variant var;
			var.type = ::TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.push_back(s6SessId);
			ticket->privateData[PathTicketPD_Field(S6SessionId)] = var;	
		}

		{
			//set ownerStreamLinkType to ticket privatedata
			TianShanIce::Variant varStreamLinkType;
			varStreamLinkType.bRange = false;
			varStreamLinkType.type = TianShanIce::vtStrings;
			varStreamLinkType.strs.clear();
			varStreamLinkType.strs.push_back(STRMLINK_TYPE_TianShanS6ERM);
			ticket->privateData[PathTicketPD_Field(ownerStreamLinkType)] = varStreamLinkType;
		}

		{
			//set ownerStreamLinkId to ticket privatedata	
			TianShanIce::Variant varStreamLinkId;
			varStreamLinkId.bRange = false;
			varStreamLinkId.type = TianShanIce::vtStrings;
			varStreamLinkId.strs.clear();
			varStreamLinkId.strs.push_back(strmLinkId);
			ticket->privateData[PathTicketPD_Field(ownerStreamLinkId)] = varStreamLinkId;
		}

		{
			::TianShanIce::Variant var_SopName;
			///set sop_name mac
			var_SopName.type = ::TianShanIce::vtStrings;
			var_SopName.bRange = false;
			var_SopName.strs.clear();
			var_SopName.strs.push_back(sopname);
			ticket->privateData[PathTicketPD_Field(sop_name)] = var_SopName;
		}

		//step2. 
		TicketAttr rid;		
		rid._usedBanwidth			= bw2Alloc;	

		{
			ZQ::common::MutexGuard gd(_ticketAttrMapMutex);
			MAPSET(TicketAttrMap, _ticketAttrMap, ticketID, rid);
		}

		_env.addOnDemandSession(sessId);
		narrowresult = NR_Narrowed;	
		envlog(ZQ::common::Log::L_INFO,CLOGFMT(S6EdgeRMPHO,"stearmLink[%s] session[%s] in sessionGroup[%s] narrowed with NeedBW[%lld] took %d ms"), strmLinkId.c_str(), sessId.c_str(), sessGroup.c_str(), bw2Alloc, (int)(ZQTianShan::now() - lStart));
	}
	catch (...) 
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"narrow_S6EdgeRMStrmLink() session[%s] narrowed caught unknown exception(%d)"), sessId.c_str() , SYS::getLastErr());  
		if(totalBandWidth > 0)
		{
			ZQ::common::MutexGuard gd(_linkAttrMapMutex);
			S6EdgeRMLinkAttrMap::iterator it= _linkAttrMap.find(ticket->streamLinkIden.name);
			if(it !=_linkAttrMap.end())
			{
				S6EdgeRMLinkAttr& linktattr = it->second;
				linktattr._usedBandwidth -= bw2Alloc;
				linktattr._availableBandwidth += bw2Alloc;
			}
		}
		if(s6Session)
			s6Session->destroy();
	}
	return narrowresult;
}

void S6EdgeRMPHO::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
{
	Ice::Long lStart = ZQTianShan::now();

	std::string strStreamlinkID="";
	::std::string s6SessionId, s6SessionGroup, OnDemondSession;
	Ice::Long bandwidth =  0;

	///get ticket Identity;
	Ice::Identity& ticketID = ticket->ident;
	envlog(ZQ::common::Log::L_INFO,CLOGFMT(S6EdgeRMPHO,"doFreeResource() ticketId[%s]"),  ticketID.name.c_str());

	try
	{
		TianShanIce::ValueMap& ticketPD = ticket->privateData;
        ///get streamlinkType
		TianShanIce::ValueMap::const_iterator itLinkType = ticketPD.find( PathTicketPD_Field(ownerStreamLinkType) ) ;
		if ( itLinkType == ticketPD.end()  || itLinkType->second.type != TianShanIce::vtStrings ||itLinkType->second.strs.size() == 0  ) 
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"doFreeResource() ticketId[%s] no ticket owner link type is found"), ticketID.name.c_str());
			return;
		}
		std::string strLinkType = itLinkType->second.strs[0];

        ///get streamlinkID
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkId->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"doFreeResource() ticketId[%s] can't get streamlink from ticket"), ticketID.name.c_str());
			return ;
		}
		strStreamlinkID = itLinkId->second.strs[0];

		ZQ::common::MutexGuard gd(_ticketAttrMapMutex);
		TicketAttrMap::iterator itAlloc =_ticketAttrMap.find(ticketID.name);
		if(itAlloc != _ticketAttrMap.end())
		{	
			bandwidth = itAlloc->second._usedBanwidth;
			_ticketAttrMap.erase(itAlloc);
		}
		else
		{
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT (S6EdgeRMPHO , "doFreeResource() streamLink[%s] ticketId[%s] not found" ), strStreamlinkID.c_str(), ticketID.name.c_str());
			return;
		}

		envlog(ZQ::common::Log::L_INFO, CLOGFMT (S6EdgeRMPHO , "doFreeResource() streamLink[%s] ticketId[%s] free resource BandWidth[%lld]" ), strStreamlinkID.c_str(), ticketID.name.c_str(), bandwidth);
		{
			ZQ::common::MutexGuard gd(_linkAttrMapMutex);
			S6EdgeRMLinkAttrMap::iterator it= _linkAttrMap.find(strStreamlinkID);
			if(it==_linkAttrMap.end())
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(S6EdgeRMPHO,"doFreeResource() ticketId[%s] Can't find the streamlink attr through the id %s"),ticketID.name.c_str(), ticket->streamLinkIden.name.c_str());
			}
			else
			{
				S6EdgeRMLinkAttr& linktattr = it->second;
				linktattr._usedBandwidth -= bandwidth;
				if(linktattr._totalBandWidth > 0)
				{
					linktattr._availableBandwidth += bandwidth;
				}
			}
		}

		TianShanIce::ValueMap::iterator itorPD = ticketPD.find(PathTicketPD_Field(s6SessionGroup));
		if(itorPD == ticketPD.end() || itorPD->second.strs.size() < 1)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT (S6EdgeRMPHO , "doFreeResource() streamLink[%s] ticketId[%s] can't find s6SessionGroup key" ), strStreamlinkID.c_str(), ticketID.name.c_str());
			return;
		}
		s6SessionGroup = itorPD->second.strs[0];

		itorPD= ticketPD.find(PathTicketPD_Field(S6SessionId));
		if(itorPD == ticketPD.end() || itorPD->second.strs.size() < 1)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT (S6EdgeRMPHO , "doFreeResource() streamLink[%s] ticketId[%s] can't find S6SessionId key" ), strStreamlinkID.c_str(), ticketID.name.c_str());
			return;
		}
		s6SessionId = itorPD->second.strs[0];

		itorPD = ticketPD.find(PathTicketPD_Field(OnDemandSessionId));
		if(itorPD == ticketPD.end() || itorPD->second.strs.size() < 1)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT (S6EdgeRMPHO , "doFreeResource() streamLink[%s] ticketId[%s] can't find OnDemandSessionId key" ), strStreamlinkID.c_str(), ticketID.name.c_str());
			return;
		}
		OnDemondSession = itorPD->second.strs[0];

		if(!_env.hasOnDemandSession(OnDemondSession))
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT (S6EdgeRMPHO , "doFreeResource() streamLink[%s] ticketId[%s] session not find" ), strStreamlinkID.c_str(), ticketID.name.c_str());
			return;
		}
		_env.removeOnDemandSession(OnDemondSession);
	}
	catch( const Ice::Exception& e)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"doFreeResource() ticketId[%s]caught an ice exception:%s"),ticketID.name.c_str(), e.ice_name().c_str());
		return;
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(S6EdgeRMPHO,"doFreeResource() ticketId[%s]caught an unknown exception(%d)"),ticketID.name.c_str(), SYS::getLastErr());
		return;
	}	

	try
	{
			S6EdgeRMLinkAttrMap::iterator itorLink;
			{
				ZQ::common::MutexGuard gd(_linkAttrMapMutex);
				itorLink =_linkAttrMap.find(strStreamlinkID);
				if(itorLink == _linkAttrMap.end())
				{	
					envlog(ZQ::common::Log::L_WARNING, CLOGFMT(S6EdgeRMPHO,"doFreeResource()ticketId[%s] can't find streamerLink[%s] info"), ticketID.name.c_str(),strStreamlinkID.c_str());
					return;
				}
			}
			S6EdgeRMLinkAttr& s6EdgeRmLinkInfo = itorLink->second;

			/// send TEARDOWN request to EdgeRM 

			/// step 1.3  send request

			ZQTianShan::EdgeRM::S6Session::Ptr  s6Session = S6SessionGroup::openSession(OnDemondSession, s6SessionGroup, true);
			if(!s6Session)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO, "doFreeResource() streamerLink[%s] ticketId[%s] failed to get s6session[%s]"), strStreamlinkID.c_str(), ticketID.name.c_str(),OnDemondSession.c_str());
				return;
			}

			s6Session->setSessionId(s6SessionId);
			ZQTianShan::EdgeRM::S6Client* s6client = s6Session->getS6Client();
			if (!s6client)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "doFreeResource() streamerLink[%s] ticketId[%s] failed to get S6client connection"), strStreamlinkID.c_str(), ticketID.name.c_str());
				s6Session->destroy();
				return ;
			}
			RTSPMessage::AttrMap headers;
//			MAPSET(RTSPMessage::AttrMap, headers, "Content-Type",      "text/parameters");
			MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_REQUIRE, "com.comcast.ngod.s6");
			MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_SESSION, s6SessionId.c_str());
			MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_ONDEMANDSESSIONID, OnDemondSession.c_str());
			MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_REASON, "200 \"User stop\"");

			int currentCSeq = s6client->sendTEARDOWN(*s6Session, NULL, headers);
			if (currentCSeq <= 0 || !s6client->waitForResponse(currentCSeq))
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "doFreeResource() streamerLink[%s]session[%s]s6Session[%s]Group[%s] failed to send teardown message"), 
					strStreamlinkID.c_str(), OnDemondSession.c_str(), s6SessionId.c_str(), s6SessionGroup.c_str());
	            s6Session->destroy();
				return ;
			}
			int resultCode  = s6Session->_resultCode; 
			s6Session->destroy();
	}
	catch (Ice::Exception& ex)
	{	
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO,"doFreeResource() streamerLink[%s] session[%s] teardown caught ice exception [%s]"), 
			strStreamlinkID.c_str(), OnDemondSession.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{	
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMPHO,"doFreeResource() streamerLink[%s] caught unknown exception [%d]"), 
			strStreamlinkID.c_str(), SYS::getLastErr());
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT (S6EdgeRMPHO , "Leave doFreeResource() streamerLink[%s] ticketId[%s], OnDemandSession[%s] took %d ms" ), 
		strStreamlinkID.c_str(), ticketID.name.c_str(), OnDemondSession.c_str(),ZQTianShan::now() - lStart);
}
void S6EdgeRMPHO::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{	
	try
	{
      TianShanIce::ValueMap& ticketPD = ticket->privateData;  

	  ///get streamlinkID
	  TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
	  if ( itLinkId ==ticketPD.end() || itLinkId->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
	  {
		  envlog(ZQ::common::Log::L_WARNING, CLOGFMT(S6EdgeRMPHO,"doCommit() can' get streamlink from ticket"));
		  return ;
	  }
	  std::string strStreamlinkID = itLinkId->second.strs[0];
	  std::string sessionId = sessCtx.sessId;

	  envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO , "doCommit() [%s#%s]" ), strStreamlinkID.c_str(), sessionId.c_str());
	}
	catch (...){}
}
void S6EdgeRMPHO::sendSetParameter(S6EdgeRMLinkAttr& s6EdgeRmLinkInfo)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6EdgeRMPHO , "[%s] check connections to [rtsp://%s:%d]" ), 
		s6EdgeRmLinkInfo._streamLinkID.c_str(), s6EdgeRmLinkInfo._rtspIp.c_str(), s6EdgeRmLinkInfo._rtspPort);

	int64 lStart = ZQ::common::now(); 
	char baseURL[256];
	try
	{
		snprintf(baseURL, sizeof(baseURL)-2, "rtsp://%s:%d/",  s6EdgeRmLinkInfo._rtspIp.c_str(), s6EdgeRmLinkInfo._rtspPort);

		int maxSessionGroups = _env.getMaxSessGroups(s6EdgeRmLinkInfo._rtspIp, s6EdgeRmLinkInfo._rtspPort);

		if(!_env.createSessionGroups(baseURL, _cfg.sessionGroup.maxSessionPerGroup, maxSessionGroups, s6EdgeRmLinkInfo._streamLinkID))
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT (S6EdgeRMPHO , "[%s] failed to check connections to [rtsp://%s:%d]" ), 
				s6EdgeRmLinkInfo._streamLinkID.c_str(), s6EdgeRmLinkInfo._rtspIp.c_str(), s6EdgeRmLinkInfo._rtspPort);
		}
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT (S6EdgeRMPHO , "[%s] failed to check connections to [rtsp://%s:%d] caught unknown exception[%d]" ),
			s6EdgeRmLinkInfo._streamLinkID.c_str(), s6EdgeRmLinkInfo._rtspIp.c_str(), s6EdgeRmLinkInfo._rtspPort, SYS::getLastErr());
	}
  
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT (S6EdgeRMPHO , "[%s] check connections to [rtsp://%s:%d] took %dms" ), 
		s6EdgeRmLinkInfo._streamLinkID.c_str(), s6EdgeRmLinkInfo._rtspIp.c_str(), s6EdgeRmLinkInfo._rtspPort, (int)(ZQ::common::now() - lStart));
/*
#define _TEST
#ifdef _TEST
	std::string sessId = "0000212";
	/// step 1.3  send request
	S6Session::Ptr s6Session = S6SessionGroup::createSession(sessId.c_str(), baseUrl, s6EdgeRmLinkInfo._streamLinkID);
	if(!s6Session)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "stearmLink[%s] session[%s] failed to create s6session"), s6EdgeRmLinkInfo._streamLinkID.c_str(), sessId.c_str());
		return;
	}

	ZQTianShan::EdgeRM::S6Client* s6client = s6Session->getS6Client();
	if (!s6client)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "stearmLink[%s] session[%s] failed to get s6client connection "), s6EdgeRmLinkInfo._streamLinkID.c_str(), sessId.c_str());
		return;
	}

	std::string sessGroup = s6Session->getSessionGroupName();
	//set group status
	RTSPRequest::AttrList parameterNames;
	parameterNames.push_back("session_list");
	//	parameterNames.push_back("connection_timeout");
	RTSPMessage::AttrMap headers;
	headers.insert(std::make_pair("SessionGroup", sessGroup));
	headers.insert(std::make_pair("Require",      "com.comcast.ngod.s6"));

	int currentCSeq = s6client->sendGET_PARAMETER(parameterNames, NULL, headers);
	if (currentCSeq <= 0 || !s6client->waitForResponse(currentCSeq))
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "stearmLink[%s] session[%s] sessionGroup[%s] failed to sendGET_PARAMETER with server timeout"), s6EdgeRmLinkInfo._streamLinkID.c_str(), sessId.c_str(), sessGroup.c_str());
		return ;
	}
#endif*/
}
bool S6EdgeRMPHO::fixupRouteIds(TianShanIce::Transport::StreamLinkPrx& strmLinkPrx, const std::string streamLinkId, S6EdgeRMLinkAttr& linkAttr)
{
	if(linkAttr._svcGrpId > 0)
		return true;
	//formate routenames expression
	ZQ::common::Preprocessor hpp;
	Ice::Int svcGrpId = 0;
	try
	{
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"fixupRouteIds() StreamLink[%s] get serviceGroupId"), streamLinkId.c_str());
		svcGrpId =  strmLinkPrx->getServiceGroupId();
		linkAttr._svcGrpId = svcGrpId;

		envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"fixupRouteIds() StreamLink[%s] serviceGroupId [%d]"), streamLinkId.c_str(), svcGrpId);

		char temp[50] = "";
		snprintf(temp, sizeof(temp)-2, "%d", svcGrpId);
		if(!hpp.define(MACRO_SVCGRP, temp))
		{
			return false;
		}
		for(int i = 0; i != linkAttr._vecQAMID.size(); i++)
		{
			if(!hpp.fixup(linkAttr._vecQAMID[i]))
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6EdgeRMPHO, "fixupRouteIds() expression not match, routeName:%s[Macro:%s]"), linkAttr._vecQAMID[i].c_str(), MACRO_SVCGRP);
				return false;
			}
		}
		std::string strQAMIDList = "";
		int iSize = linkAttr._vecQAMID.size();
		for (int i = 0; i < iSize; i++)
		{
			strQAMIDList += linkAttr._vecQAMID[i];
			if (i < iSize - 1)
				strQAMIDList += ::std::string(", ");
		}
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6EdgeRMPHO,"fixupRouteIds() StreamLink[%s] Route-IDs[%s]"), streamLinkId.c_str(), strQAMIDList.c_str());

	}
	catch(::TianShanIce::InvalidParameter& e)
	{
		envlog(ZQ::common::Log::L_WARNING,CLOGFMT(S6EdgeRMPHO,"fixupRouteIds() StreamLink[%s] failed to get serviceGroupId caught exception [%s]"),streamLinkId.c_str(), e.message.c_str());
	}
	catch (Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_WARNING,CLOGFMT(S6EdgeRMPHO,"fixupRouteIds() StreamLink[%s] failed to get serviceGroupId caught ice exception [%s]"),streamLinkId.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_WARNING,CLOGFMT(S6EdgeRMPHO,"fixupRouteIds() StreamLink[%s] failed to get serviceGroupId caught unknown exception [%d]"),streamLinkId.c_str(), SYS::getLastErr());
	}
	return true;
}
}}//namespace ZQTianShan::AccreditedPath

