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
// Ident : $Id: IpEdgePHO.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History:
#include "IpEdgePHO.h"
#include "Log.h"

#include <time.h>
#include "public.h"

namespace ZQTianShan {
namespace AccreditedPath {
	
///schema for STRMLINK_TYPE_IPEDGE_IP
static ConfItem IpEdge_IP[] = {
	{ "TotalBandwidth",	::TianShanIce::vtLongs,	false, "20000"}, // in Kbps
//	{ "SourceStreamerIP",::TianShanIce::vtStrings,false,"127.0.0.1"},
	{ "SOP_Name",TianShanIce::vtStrings,false,"" },
	{ "SOP_Group",TianShanIce::vtStrings,false,""},
//	{ "DestinationIP", ::TianShanIce::vtStrings,true, "127.0.0.1"},	
	{ NULL, ::TianShanIce::vtInts, true, NULL },
};



IpEdgePHO::IpEdgePHO(IPHOManager& mgr)
	: IStreamLinkPHO(mgr)
{
	_phoManager=&mgr;
	//_helperMgr.registerStreamLinkHelper(STRMLINK_TYPE_NGOD_SS, *this, NULL);
}

IpEdgePHO::~IpEdgePHO()
{	
	_helperMgr.unregisterStreamLinkHelper(STRMLINK_TYPE_NGOD_SS);
}

bool IpEdgePHO::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;
	
	// address the schema definition
	if (0 == strcmp(type, STRMLINK_TYPE_NGOD_SS))
		config = IpEdge_IP;
	// no matches
	if (NULL == config)
	{
		return false;
	}
	// copy the schema to the output
	for (::ZQTianShan::ConfItem *item = config; item && item->keyname; item ++)
	{
		::TianShanIce::PDElement elem;
		elem.keyname = item->keyname;
		elem.optional = item->optional;
		elem.defaultvalue.type= item->type;
		schema.push_back(elem);
	}
	return true;
}

void IpEdgePHO::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
		type = "NULL";

	if (0 == strcmp(STRMLINK_TYPE_NGOD_SS, type))
		validate_IPStrmLinkConfig(identStr, configPD);	
	else 
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>("IpEdgePHO",0,"unsupported type [%s] in IpEdgePHOForNGOD", type);
}

Ice::Int IpEdgePHO::doEvaluation(LinkInfo& linkInfo, 
								 const SessCtx& sessCtx,
								 TianShanIce::ValueMap& hintPD, 
								 const ::Ice::Int oldCost)
{
	std::string& linktype= linkInfo.linkType;

	if (0 == linktype.compare(STRMLINK_TYPE_NGOD_SS))
		return eval_IPStrmLink(linkInfo, sessCtx, hintPD, linkInfo.rcMap,oldCost);	
	return ::TianShanIce::Transport::OutOfServiceCost;
}
#ifdef _DEBUG
void printfitit(const char* p)
{
	printf("%s",p);
}
#endif//_DEBUG
void IpEdgePHO::validate_IPStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(IpEdgePHO, "validate a IP StreamLink's Configuration: link[%s]"), identStr);
#ifdef _DEBUG	
	::ZQTianShan::dumpValueMap(configPD, "  " STRMLINK_TYPE_NGOD_SS ": ",printfitit);
#endif _DEBUG
	LinkIPAttr la;
	la._streamLinkID=identStr;
	::TianShanIce::Variant val;
	val.bRange=false;
	try
	{
		
		ZQ::common::MutexGuard gd(_ipResourceLocker);

		//TotalBandwidth
		val=PDField(configPD,"TotalBandwidth");
		//ASSETVARIANT(val,::TianShanIce::vtLongs);
		if(val.type != ::TianShanIce::vtLongs)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgePHO",0,"TotalBanwidth should be vtLongs");
		}
		if(val.lints.size()==0)
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() no TotalbandWidth attribute in streamLink %s"),identStr);
			return;
		}
		la._toalBandwidth=val.lints[0]*1000;
		la._availableBandwidth=la._toalBandwidth;
		la._usedBandwidth = 0; //set it to 0 when initialize
		
		val=PDField(configPD,"SOP_Name");
		if (val.strs.size() > 0 )
		{
			
			//ASSETVARIANT(val,::TianShanIce::vtStrings);
			if(val.type != TianShanIce::vtStrings)
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgePHO",0,"SOP_Name should be vtStrings");
			}			
			la._strSOPName = val.strs[0];		
		}
		else
		{
			if(val.type != TianShanIce::vtStrings)
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgePHO",0,"SOP_Name should be vtStrings");
			}
			
			la._strSOPName = "";
		}
		
		val=PDField(configPD,"SOP_Group");
		if (val.strs.size() > 0 )
		{
			
			//ASSETVARIANT(val,::TianShanIce::vtStrings);
			if(val.type != TianShanIce::vtStrings)
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgePHO",0,"SOP_Group should be vtStrings");
			}			
			la._strSOPGroup = val.strs[0];		
		}
		else
		{
			if(val.type != TianShanIce::vtStrings)
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgePHO",0,"SOP_Group should be vtStrings");
			}
			
			la._strSOPGroup = "";
		}
	
		LinkIPAttrMap::iterator itAttr=_StreamLinkIPAttrmap.find(la._streamLinkID);

			::TianShanIce::Transport::StreamLinkPrx strmLinkPrx=_phoManager->openStreamLink(identStr);
			if(strmLinkPrx)
			{
				_ipResourceDataMap.clear();
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
						TianShanIce::ValueMap PDdata=ticketprx->getPrivateData();
						TianShanIce::SRM::ResourceMap dataMap=ticketprx->getResources();

						::TianShanIce::Variant bandwidth;
						::TianShanIce::Variant port;
						std::string	ticketID = ticketprx->getIdent().name;
						try
						{
							TianShanIce::SRM::ResourceMap::const_iterator it=dataMap.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
							if(it==dataMap.end())
							{
								glog(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig ticket [%s] but no bandwidth resource"),itID->name.c_str());
							}
							else
							{	
								TianShanIce::ValueMap val=it->second.resourceData;
								bandwidth=PDField(PDdata,PathTicketPD_Field(bandwidth));
								glog(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() ticket %s used bandwidth [%lld]"),itID->name.c_str(),bandwidth.lints[0]);
								la._availableBandwidth -= bandwidth.lints[0];
								la._usedBandwidth += bandwidth.lints[0];
								ResourceIPData rid;
								rid._usedBandwidth = bandwidth.lints[0];
								_ipResourceDataMap.insert(ResourceIPDataMap::value_type(ticketID,rid));
							}
						}
						catch(::TianShanIce::InvalidParameter& e)
						{
							glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() invalidParameter exception is caught:[%s]"),e.message.c_str());
						}
						catch (...)
						{							
							glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() UNknown exception is caught "));
						}
					}
				}
			}			
			
		if(itAttr==_StreamLinkIPAttrmap.end())
		{			
			//list all ticket by link and get it
		
			_StreamLinkIPAttrmap.insert(std::make_pair<std::string,LinkIPAttr>(la._streamLinkID,la));
			glog(ZQ::common::Log::L_INFO,
				CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() insert a ip streamlink with streamlink id =[%s] totalBandwidth=[%lld](BPS) usedBandWidth=[%lld](BPS)"),
				identStr,la._toalBandwidth,la._usedBandwidth);
		}
		else
		{
			itAttr->second=la;
			glog(ZQ::common::Log::L_INFO,
				CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() update a ip streamlink with streamlink id =[%s] totalBandWidth=[%lld](BPS) availableBandwidth=[%lld](BPS)"),
				identStr,la._toalBandwidth,la._usedBandwidth);
		}
	}
	catch (::Ice::Exception& e)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() catch a ice exception :[%s]"),e.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() unexpect error"));
	}
}



#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
  if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
 else glog(ZQ::common::Log::L_WARNING, CLOGFMT(IpEdgePHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}



Ice::Int IpEdgePHO::eval_IPStrmLink(LinkInfo& linkInfo,
									const SessCtx& sessCtx,
									TianShanIce::ValueMap& hintPD,
									TianShanIce::SRM::ResourceMap& rcMap , 
									const ::Ice::Int oldCost)
{	
	::Ice::Int newCost = oldCost;
	
	std::string sessId= sessCtx.sessId;

	glog(ZQ::common::Log::L_DEBUG, 
			CLOGFMT(IpEdgePHO,"Session [%s] enter eval_IPStreamLink() with OldCost=%d"),
			sessId.c_str(),oldCost);

	if (newCost > ::TianShanIce::Transport::MaxCost)
	{	
		glog(ZQ::common::Log::L_ERROR,
			CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] return oldCost=%d because oldCost > MaxCost"),
			sessId.c_str(),newCost);
		return newCost;
	}


	// step 1. read the link private data that need to be evaluated, please refer to 
	::Ice::Long totalBW =0;
	::Ice::Long usedBW =0;
		// step 1.2 get resource information from session
	::TianShanIce::SRM::ResourceMap		resourceMap;
	::Ice::Long							bw2Alloc = 0;
	std::string							destAddr = "";
	int									destPort =0;
	int									serverPort = 0;
	std::string							streamerSrcIP = "";
	std::string							sopName = "";
	std::string							sopGroup = "";

		
	try
	{	
		//get needed bandwidth from resourceMap
		TianShanIce::SRM::ResourceMap  ResMap = sessCtx.resources;
		READ_RES_FIELD(bw2Alloc, ResMap, rtTsDownstreamBandwidth, bandwidth, lints);
		glog( ZQ::common::Log::L_DEBUG,
			CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] get Needed bandwidth %lld from resourceMap"),
				sessId.c_str(),bw2Alloc);
		if (hintPD.end() != hintPD.find(PathTicketPD_Field(bandwidth)))
		{
			::TianShanIce::Variant& var = hintPD[PathTicketPD_Field(bandwidth)];
			if (var.type == TianShanIce::vtLongs && var.lints.size() >0)
			{
				bw2Alloc = (bw2Alloc >0) ? max(bw2Alloc, var.lints[0]) : var.lints[0];
				glog( ZQ::common::Log::L_ERROR,
					CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] bandwidth is adjusted by hinted PrivateData to %lld"),
					sessId.c_str(),bw2Alloc);
			}
		}
	
		
		TianShanIce::Variant			var;
		TianShanIce::ValueMap			ValMap = sessCtx.privdata;
	
		//get destination ip
		if( ValMap.find(NGOD_RES_PREFIX(destination))==ValMap.end() )
		{
			glog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] no %s was found in privateData"),
				sessId.c_str(),NGOD_RES_PREFIX(destination));
			return TianShanIce::Transport::OutOfServiceCost;
		}
		else
		{
			var = ValMap[NGOD_RES_PREFIX(destination)];
			if( (var.type != TianShanIce::vtStrings) || (var.strs.size()==0) )
			{
				glog(ZQ::common::Log::L_DEBUG,
					CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] invalid"
					" %s data type,should be string.Or data size == 0 . Return with outOfServiceCost"),
					sessId.c_str() , NGOD_RES_PREFIX(destination));
				return TianShanIce::Transport::OutOfServiceCost;
			}//bRange is ignored		
			else
			{
				destAddr = var.strs[0];
			}
		}
		//get server port,if not exist ,set it to 0
		if(ValMap.find(NGOD_RES_PREFIX(server_port))==ValMap.end())
		{
			glog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] no %s is found ,set it to 0"),
				sessId.c_str(),NGOD_RES_PREFIX(server_port)	);
			serverPort = 0;
		}
		else
		{
			var = ValMap[NGOD_RES_PREFIX(server_port)];
			if( (var.type != TianShanIce::vtInts) || (var.ints.size() == 0)  )
			{
				glog(ZQ::common::Log::L_DEBUG,
					CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Sesion[%s] invalid %s data type,"
					"should be int,Or data Size == 0.Set it to 0"),
					sessId.c_str(),NGOD_RES_PREFIX(server_port));
				serverPort = 0;
			}
			else
			{
				serverPort = var.ints[0];
			}
		}

		//get client port
		if(ValMap.find(NGOD_RES_PREFIX(client_port))==ValMap.end())
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] no %s is found,return outOfServiceCost"),
				sessId.c_str(),NGOD_RES_PREFIX(client_port) );
			return TianShanIce::Transport::OutOfServiceCost;
		}
		else
		{
			var = ValMap[NGOD_RES_PREFIX(client_port)];
			if( (var.type != TianShanIce::vtInts ) || (var.ints.size()==0) )
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Sesion[%s] invalid %s data type,"
					"should be int,Or data Size == 0"),
					sessId.c_str(),NGOD_RES_PREFIX(client_port));
				return TianShanIce::Transport::OutOfServiceCost;
			}
			else
			{
				destPort = var.ints[0];
			}
		}
		
		bool	bGetSopName= false ;
		bool	bGetSopGroup = false;
		//get SOP name
		if( ValMap.find(NGOD_RES_PREFIX(sop_name))==ValMap.end() )
		{
			glog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] no %s was found in privateData"),
				sessId.c_str() , NGOD_RES_PREFIX(sop_name));
			bGetSopName	= false;
		}
		else
		{
			var = ValMap[NGOD_RES_PREFIX(sop_name)];
			if( (var.type != TianShanIce::vtStrings) || (var.strs.size() == 0) )
			{
				glog(ZQ::common::Log::L_DEBUG,
					CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] invalid"
					" %s data type,should be string.Or data size == 0 "),
					sessId.c_str() , NGOD_RES_PREFIX(sop_name));
				return TianShanIce::Transport::OutOfServiceCost;
			}
			else
			{
				sopName  = var.strs[0];
				glog(ZQ::common::Log::L_DEBUG,
						CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] "
						"get  [%s] = [%s]" ),
						sessId.c_str(), NGOD_RES_PREFIX(sop_name),sopName.c_str());
				bGetSopName = true;
			}
		}


		//get sop group
		if( ValMap.find(NGOD_RES_PREFIX(sop_group))==ValMap.end() )
		{
			glog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] no %s was found in privateData"),
				sessId.c_str() , NGOD_RES_PREFIX(sop_group));
			bGetSopGroup	= false;
		}
		else
		{
			var = ValMap[NGOD_RES_PREFIX(sop_group)];
			if( (var.type != TianShanIce::vtStrings) || (var.strs.size() == 0) )
			{
				glog(ZQ::common::Log::L_DEBUG,
					CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] invalid"
					" %s data type,should be string.Or data size == 0 "),
					sessId.c_str() , NGOD_RES_PREFIX(sop_group));
				return TianShanIce::Transport::OutOfServiceCost;
			}
			else
			{
				sopGroup  = var.strs[0];
				glog(ZQ::common::Log::L_DEBUG,
						CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] "
						"get  [%s] = [%s]"),
						sessId.c_str(), NGOD_RES_PREFIX(sop_group),sopName.c_str());
				bGetSopGroup =true;
				
			}
		}
		if(!bGetSopGroup && !bGetSopName)
		{
			glog(ZQ::common::Log::L_ERROR,
				CLOGFMT(IpEdgePHO, "eval_IPStreamLink() Session[%s] "
				"neither SOP name nor SOP Group is found,return with OutOfServiceCost"),
				  sessId.c_str());
			return TianShanIce::Transport::OutOfServiceCost;
		}	

		//Check the cost of bandwidth and port
		{
			std::string streamLinkID = linkInfo.linkIden.name;
			ZQ::common::MutexGuard gd(_ipResourceLocker);			
			LinkIPAttrMap::iterator it = _StreamLinkIPAttrmap.find(streamLinkID);
			if( it ==  _StreamLinkIPAttrmap.end() )
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] no StreamLink with id[%s]is found,"
					"return with OutOfServiceCost"),
					sessId.c_str(),streamLinkID.c_str());
				return TianShanIce::Transport::OutOfServiceCost;					 
			}
			if(bGetSopName)
			{
				if(sopName   !=   it->second._strSOPName)
				{
					glog(ZQ::common::Log::L_ERROR,
							CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] sopname[%s] does not match the record [%s]"),
							sessId.c_str(),sopName.c_str() , it->second._strSOPName.c_str()	);
					return TianShanIce::Transport::OutOfServiceCost;
				}
			}
			if(bGetSopGroup)
			{
				if(sopGroup   !=   it->second._strSOPGroup)
				{
					glog(ZQ::common::Log::L_ERROR,
							CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] Sop_Group[%s] does not match the record [%s]"),
							sessId.c_str(),sopGroup.c_str() , it->second._strSOPGroup.c_str()	);
					return TianShanIce::Transport::OutOfServiceCost;
				}
			}
			
			const LinkIPAttr&	li = it->second;
			totalBW = li._toalBandwidth;
			usedBW = li._usedBandwidth;
			if( bw2Alloc > (totalBW-usedBW) )
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] not enough bandwidth for needed Bandwidth [%lld],"
					"current TotalBW=[%lld] usedBW=[%lld]"),
					sessId.c_str(),bw2Alloc,li._toalBandwidth,li._usedBandwidth);
				return TianShanIce::Transport::OutOfServiceCost;
			}
			
			int bwCost =(int) (( li._usedBandwidth )*TianShanIce::Transport::MaxCost / li._toalBandwidth);
//			it->second._availableBandwidth -= bw2Alloc;
//			it->second._usedBandwidth += bw2Alloc;
			//calculate the bandwidth in narrow state

			//int portCost = (li._totalPortCount - li._availablePorts.size() )*TianShanIce::Transport::OutOfServiceCost/li._totalPortCount;
			//newCost  =  bwCost > portCost ? bwCost : portCost;			
			newCost = bwCost;	

			glog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] with bwCost=[%d] retuned"),
				sessId.c_str(), bwCost );			
		}
		//fill the attribute into ticket data so the streamer can use it
		
		{//fill bandwidth
			
			TianShanIce::Variant valBW;
			valBW.lints.clear();
			valBW.lints.push_back( bw2Alloc );
			valBW.type = TianShanIce::vtLongs;
			valBW.bRange = false;			
			hintPD[PathTicketPD_Field(bandwidth)] = valBW;
			
			TianShanIce::Variant valDestAddr;
			valDestAddr.strs.clear();
			valDestAddr.strs.push_back(destAddr);
			valDestAddr.type =TianShanIce::vtStrings;
			valDestAddr.bRange = false;
			hintPD[PathTicketPD_Field(destAddr)] = valDestAddr;
			
			TianShanIce::Variant valDestPort;
			valDestPort.ints.clear();
			valDestPort.ints.push_back(destPort);
			valDestPort.type = TianShanIce::vtInts;
			valDestPort.bRange = false;
			hintPD[PathTicketPD_Field(destPort)] = valDestPort;
					
			TianShanIce::Variant valSourceStreamIP;
			valSourceStreamIP.strs.clear();
			valSourceStreamIP.strs.push_back(streamerSrcIP);
			valSourceStreamIP.type =TianShanIce::vtStrings;
			valSourceStreamIP.bRange = false;
			hintPD[PathTicketPD_Field(sourceStreamIP)] = valSourceStreamIP;
			
			
					
			TianShanIce::Variant valServerPort;
			valServerPort.ints.clear();
			valServerPort.ints.push_back(serverPort);
			valServerPort.type =  TianShanIce::vtInts;
			valServerPort.bRange = false;
			hintPD[PathTicketPD_Field(StreamServerPort)] =  valServerPort;
			

			TianShanIce::Variant valDestMac;
			valDestMac.strs.clear();
			valDestMac.strs.push_back("1:2:3:4:5:6");
			valDestMac.type = TianShanIce::vtStrings;
			valDestMac.bRange = false;
			hintPD[PathTicketPD_Field(destMac)] = valDestMac;
		
			PutResourceMapData(rcMap,TianShanIce::SRM::rtEthernetInterface,"destIP",valDestAddr);
			PutResourceMapData(rcMap,TianShanIce::SRM::rtEthernetInterface,"destPort",valDestPort);
			//valDestPort
#pragma message(__MSGLOC__"TODO:Dest MAC address should be used or not?")
			//put a dummy mac address
			//rtEthernetInterface
			PutResourceMapData(rcMap,TianShanIce::SRM::rtEthernetInterface,"destMac",valDestMac);

			PutResourceMapData(rcMap,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth",valBW);
			
#ifdef _DEBUG
			printf("###################\n");
			dumpResourceMap(rcMap);
#endif
		}

		return newCost;
	}
	catch (TianShanIce::BaseException& ex) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] TianShan  Exception :%s"),ex.message.c_str());
		return TianShanIce::Transport::OutOfServiceCost;
	}
	catch (::Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] Ice Exception :%s"),ex.ice_name().c_str());
		return TianShanIce::Transport::OutOfServiceCost;
	}
	catch (...) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] Unkown Exception") );
		return TianShanIce::Transport::OutOfServiceCost;
	}
}

IPathHelperObject::NarrowResult IpEdgePHO::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, 
													const SessCtx& sessCtx)
{
	if (!ticket)
		return NR_Unrecognized;

	std::string type;
	::TianShanIce::Transport::StreamLinkPrx strmLink;
	try {
		strmLink = _helperMgr.openStreamLink(ticket->streamLinkIden);
		type = strmLink->getType();
	}
	catch(...) {}
	if (0 == type.compare(STRMLINK_TYPE_NGOD_SS))
		return narrow_IPStrmLink(strmLink, sessCtx, ticket);
	else return IPathHelperObject::NR_Unrecognized;
}

IPathHelperObject::NarrowResult IpEdgePHO::narrow_IPStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
															const SessCtx& sessCtx,
															const TianShanIce::Transport::PathTicketPtr& ticket)
{
	std::string sessId= sessCtx.sessId;


	try
	{
		std::string linkIDName = strmLink->getIdent().name;
		std::string ticketID = ticket->ident.name;
		glog(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"Session[%s] narrowed ticket with ticketid=%s"),
						sessId.c_str(),ticketID.c_str());

		ResourceIPData rid;		
		TianShanIce::SRM::ResourceMap& resMap=ticket->resources;
		Ice::Long	bw2Alloc;
		Ice::Int	port = 0;
		READ_RES_FIELD(bw2Alloc, resMap, rtTsDownstreamBandwidth, bandwidth, lints);
		rid._usedBandwidth	=	bw2Alloc;
		{
			//allocate a port
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			LinkIPAttrMap::iterator it = _StreamLinkIPAttrmap.find(linkIDName);
			if( it == _StreamLinkIPAttrmap.end() )
			{				
				glog(ZQ::common::Log::L_ERROR,
						CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() Session[%s] no streamLink with ID [%s] is found"),
						sessId.c_str(), linkIDName.c_str());
				return NR_Error;
			}
			if ( (it->second._toalBandwidth - it->second._usedBandwidth) < bw2Alloc) 
			{
				glog(ZQ::common::Log::L_ERROR,
						CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() Session[%s] No Available bandwidth TotalBW[%lld] usedBW[%lld] needBW[%lld]"),
						sessId.c_str(), linkIDName.c_str(),it->second._toalBandwidth,
						it->second._usedBandwidth,bw2Alloc);
				return NR_Error;
			}
			it->second._usedBandwidth += bw2Alloc;			
			glog(ZQ::common::Log::L_INFO,
				CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() Session[%s] Ticket[%s] narrow with "
						"BW[%lld] and now totalBW[%lld] usedBW[%lld]"),
						sessId.c_str(),ticketID.c_str(),bw2Alloc,it->second._toalBandwidth,it->second._usedBandwidth);
			
			_ipResourceDataMap[ticketID]=rid;
		}
	}
	catch (const Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"Ice exception when narrow :[%s]"),ex.ice_name().c_str());
		return NR_Error;
	}	
	return NR_Narrowed;
}


void IpEdgePHO::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
{
	try
	{
		::TianShanIce::Transport::StreamLinkPrx strmlinkPrx=ticket->getStreamLink();
		if(!strmlinkPrx)
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"doFreeResource() can' get streamlink from ticket"));
			return ;
		}
		std::string strStramlinkID=strmlinkPrx->getIdent().name;
		std::string	strLinkType=strmlinkPrx->getType();
		

		TianShanIce::Variant val;
		Ice::Identity ticketID=ticket->getIdent();

		if (strcmp(STRMLINK_TYPE_NGOD_SS,strLinkType.c_str())==0)
		{//IP mode
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			ResourceIPDataMap::iterator itAlloc=_ipResourceDataMap.find(ticketID.name);
			if(itAlloc==_ipResourceDataMap.end())
			{
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(IpEdgePHO,"doFreeResource() no allocated resource with ticketID=%s"),ticketID.name.c_str());
				return;
			}
			ResourceIPData& ipData = itAlloc->second;			
			TianShanIce::ValueMap value=ticket->getPrivateData();
			LinkIPAttrMap::iterator it=_StreamLinkIPAttrmap.find(strStramlinkID);
			if(it==_StreamLinkIPAttrmap.end())
			{
				glog(ZQ::common::Log::L_ERROR,
						CLOGFMT(IpEdgePHO,"doFreeResource() can't find strmlink attr through streamlink id=%s"),
						strStramlinkID.c_str());
				return;
			}
			val=PDField(value,PathTicketPD_Field(bandwidth));
			if( (val.type !=TianShanIce::vtLongs) || (val.lints.size()==0) )
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(IpEdgePHO,"doFreeResource() ticket %s has a invalid bandwidth data"),
					ticketID.name.c_str());
			}
			if(ipData._usedBandwidth != val.lints[0])
			{
				glog(ZQ::common::Log::L_INFO,
					CLOGFMT(IpEdgePHO,"doFreeResource() ticket %s's privateData Bandwidth %lld doesn't match the record %lld"),
					 ticketID.name.c_str(),val.lints[0],ipData._usedBandwidth);
			}
			Ice::Long bandwidth= ipData._usedBandwidth;//val.lints[0];
			//it->second._availableBandwidth+=bandwidth;
			it->second._usedBandwidth -= bandwidth;
//			Ice::Int port=ipData._usedPort;
//			it->second._availablePorts.push_back(port);
			_ipResourceDataMap.erase(itAlloc);
			glog(ZQ::common::Log::L_INFO,
					CLOGFMT(IpEdgePHO,"doFreeResource() free ip resource from ticket [%s] with bandwidth=[%lld] "
					"now totalBW=[%lld] usedBW=[%lld] "),
					ticketID.name.c_str(),bandwidth,it->second._toalBandwidth,it->second._usedBandwidth);
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"doFreeResource() unrecognised stram link type"));
			return;
		}
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"doFreeResource() catch a tianshanice exception:%s"),ex.message.c_str());
		return;
	}

	catch(Ice::Exception& e)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"doFreeResource() catch a ice exception:%s"),e.ice_name().c_str());
		return;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"doFreeResource() catch a unexpect exception"));
		return;
	}	
}

void IpEdgePHO::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{
	
}

}} // namespace



