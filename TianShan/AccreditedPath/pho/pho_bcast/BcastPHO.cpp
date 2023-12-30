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
// Ident : $Id: $
// Branch: $Name:  $
// Author: 
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/pho/pho_bcast/BcastPHO.cpp $
// 
// 3     1/11/16 4:58p Dejian.fei
// 
// 2     7/13/15 1:00p Li.huang
// 
// 1     7/07/15 4:16p Li.huang
// 

// ===========================================================================

#include "BcastPHO.h"
#include "Log.h"
#include <public.h>
#include <time.h>
#include <algorithm>
#include <TianShanIceHelper.h>

extern ZQ::common::Log* pho_Seachangelog;
#define MLOG (*pho_Seachangelog)

namespace ZQTianShan {
namespace AccreditedPath {
	
///schema for STRMLINK_TYPE_IPEDGE_DVBC
static ::ZQTianShan::ConfItem Mrt_PubPoint[] = {	
	{ "Qam.modulationFormat",	::TianShanIce::vtInts,	false, "0x10" ,false},
	{ "Qam.IP",			::TianShanIce::vtStrings,		false, "192.168.80.138",false },
	{ "Qam.Mac",		::TianShanIce::vtStrings,		false,	"a:b:c:d:e:f" ,false },
	{ "Qam.basePort",	::TianShanIce::vtInts,			false, "4001" ,false},
	{ "Qam.portMask",	::TianShanIce::vtInts,			false, "65280" ,false},
	{ "Qam.portStep",	::TianShanIce::vtInts,			false,	"1",false},
	{ "Qam.symbolRate", ::TianShanIce::vtInts,			false, "50000" ,false},
	{ "Qam.frequency",	::TianShanIce::vtInts,			false, "1150" ,false},
	{ "PN",				::TianShanIce::vtInts,			false, " 5 ~ 20 " ,true},
	{ "TotalBandwidth",	::TianShanIce::vtLongs,			false, "20000" ,false}, // in Kbps
	{ "MRTPubPoint",	::TianShanIce::vtStrings,		false, "" ,false}, // in Kbps
	{ NULL, ::TianShanIce::vtInts, true, "",false },
};

BcastPHO::BcastPHO(IPHOManager& mgr)
	: IStreamLinkPHO(mgr)
{
	_phoManager=&mgr;
//	_helperMgr.registerStreamLinkHelper(STRMLINK_TYPE_MRT_PUBPOINT, *this, NULL);
}

BcastPHO::~BcastPHO()
{
	_helperMgr.unregisterStreamLinkHelper(STRMLINK_TYPE_MRT_PUBPOINT);
}

bool BcastPHO::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;
	
	// address the schema definition
	if (0 == strcmp(type, STRMLINK_TYPE_MRT_PUBPOINT))
		config = Mrt_PubPoint;

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

void BcastPHO::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
		type = "NULL";

	if (0 == strcmp(STRMLINK_TYPE_MRT_PUBPOINT, type))
	{
		validate_BcastStrmLinkConfig(identStr, configPD);
	}
	else
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"BcastPHO",1001,"unsupported type [%s] in BcastPHO for [%s] ", type , identStr );
	}
}

void BcastPHO::validate_BcastStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastPHO, "enter validate a Bcast StreamLink's Configuration: link[%s]"), identStr);

	//retrieve data from configPD	
	//setup a link dvbc attribute instance
	LinkDVBCAttr la;
	la._streamLinkID=identStr;
	::TianShanIce::Variant val;
	val.bRange=false;
	try
	{
		//get qam'name ,if there is no qam name ,take qam ip as it's name


		///get total bandwidth and set available bandwidth
		val=PDField(configPD,"TotalBandwidth");
		if(val.type!=::TianShanIce::vtLongs)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"BcastPHO",1031,"Invalid TotalBandwidth type,should be vtLongs for streamLink[%s]" , identStr );
		}
		if (val.lints.size()==0) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"BcastPHO",1032,"Invalid TotalBandwidth type,no bandwidth is found for streamLink[%s]",identStr );
		}
		la._totalBandWidth=val.lints[0]*1000;
		la._availableBandwidth=la._totalBandWidth;//Available Bandwidth is useless
		la._usedBandwidth = 0;//initialize usedBandwidth as 0 
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"Stream Bcast link [%s] get totalBW[%lld]"),identStr,la._totalBandWidth);

		///get program number and set total pn count
		val=PDField(configPD,"PN");
		if(val.type!=::TianShanIce::vtInts)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgaPHO",1033,"Invalid PN type,should be vtInts for streamLink[%s]",identStr);
		}
		if (val.bRange)
		{
			if (val.ints.size() < 2) 
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgaPHO",1034,"Invalid PN,range=true but content size < 2 , for streamLink[%s]" , identStr );
			}
			::Ice::Int minPn = val.ints[0],  maxPn = val.ints[1];
			if (minPn > maxPn)
			{
				Ice::Int temp = minPn;
				minPn = maxPn ;
				maxPn = temp;
				//std::swap<Ice::Int> (minPn, maxPn);
			}

			la._pnMin = minPn;
			la._pnMax = maxPn;
			for (; minPn <= maxPn; minPn++)
				la._availablePN.push_front(minPn);
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"Stream DVBC link [%s] get ProgramNumber [%d,%d] count=[%d]"),
				identStr,la._pnMin,la._pnMax,la._pnMax+1-la._pnMin);
		}
		else
		{
			if (val.ints.size()==0) 
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgaPHO",1035,"Invalid PN data,data size is 0 , for streamLink[%s]",identStr);
			}
			else if (val.ints.size() == 1)
			{
				//How to deal with multiple values
				la._pnMax = la._pnMin = val.ints[0];
				la._availablePN.push_back(val.ints[0]);
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"Stream DVBC link [%s] get ProgramNumber [%d,%d] count=[%d]"),
					identStr,la._pnMin,la._pnMax,la._pnMax+1-la._pnMin);
			}
			else 
			{
				la._pnMax = la._pnMin = -1;
				for (int  i = 0 ; i < (int) val.ints.size()  ; i++ ) 
				{
					std::list<Ice::Int>::const_iterator itBackup = la._backupPN.begin();					
					for ( ;itBackup != la._backupPN.end() ; itBackup ++ ) 
					{
						if ( *itBackup == val.ints[i] ) 
						{
							MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"Stream DVBC link [%s] with duplicate PN [%d]"),identStr,val.ints[i]);
							ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgaPHO",1036,"streamLink [%s] input has duplicate PN %d",identStr , val.ints[i]);
						}
					}
					la._backupPN.push_back(val.ints[i]);
					la._availablePN.push_back(val.ints[i]);
					MLOG(ZQ::common::Log::L_INFO , CLOGFMT(BcastPHO , "Stream DVBC link[%s] with input PN [%d]"),identStr , val.ints[i]);
				}
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"Stream DVBC link [%s] get ProgramNumber count=[%d]"),
					identStr, la._availablePN.size() );
			}
		}
		la._totalPNCount=la._availablePN.size();

		//get port mask default is 65280 ==>0xFF00		
		try
		{
			val=PDField(configPD,"Qam.portMask");
			if (val.type == TianShanIce::vtInts &&val.ints.size()>0) 
			{
				la._portMask = val.ints[0];
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"Stream DVBC link [%s] get portmask [%x]"),identStr,la._portMask);
			}
			else
			{
				la._portMask = 0xFF00;
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(BcastPHO,"Stream DVBC link [%s] invalid portmask type or data,set it to [%x]"),identStr,0xFF00);
			}
		}
		catch (const TianShanIce::InvalidParameter&) 
		{
			la._portMask = 0xFF00;
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(BcastPHO,"Stream DVBC link [%s] can't get portmask,setit to [%x]"),identStr,0xFF00);
		}

		try
		{
			val = PDField(configPD,"Qam.portStep");
			if (val.type == TianShanIce::vtInts && val.ints.size() >0) 
			{
				la._portStep = val.ints[0];
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"Stream DVBC link [%s] get portstep [%d]"),identStr,la._portStep);
			}
			else
			{
				la._portStep = 1;
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(BcastPHO,"Stream DVBC link [%s] invalid portstep type or data,set it to [%d]"),identStr,la._portStep);
			}
		}
		catch (const TianShanIce::InvalidParameter&) 
		{
			la._portStep = 1;
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(BcastPHO,"Stream DVBC link [%s] can't get portstep,set it to [%d]"),identStr,la._portStep);
		}


		///get base port and format it
		val=PDField(configPD,"Qam.basePort");
		if(val.type!=::TianShanIce::vtInts)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgaPHO",1037,"Invalid Qam.baseport type,should be vtInts ,for streamLink[%s]",identStr);
		}
		la._basePort=val.ints[0] & (unsigned int)la._portMask;	//最后八位置0
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"Stream DVBC link [%s] get baseport[%d] and change it to [%d] with mask[%x]"),
			identStr,val.ints[0],la._basePort,la._portMask);

		///get MRTPubPoint
		val=PDField(configPD,"MRTPubPoint");
		if(val.type!=::TianShanIce::vtStrings)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"BcastPHO",1031,"Invalid MRTPubPoint type,should be vtString for streamLink[%s]" , identStr );
		}
		if (val.strs.size()==0) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"BcastPHO",1032,"Invalid MRTPubPoint type,no MRTPubPoint is found for streamLink[%s]",identStr );
		}
		la._MRTUrl=val.strs[0];

		Ice::Int iServiceState = TianShanIce::stNotProvisioned;
		ZQTianShan::Util::getValueMapDataWithDefault( configPD, _SERVICE_STATE_KEY_, TianShanIce::stNotProvisioned, iServiceState );

		if( iServiceState != TianShanIce::stInService )
		{
			ZQ::common::MutexGuard gd(_dvbcResourceLocker);

			LinkDVBCAttrMap::iterator itAttr=_StreamLinkDVBCAttrmap.find(la._streamLinkID);

			///find out the used bandwidth and program number,then remove it from available resource
			::TianShanIce::Transport::StreamLinkPrx strmLinkPrx=_phoManager->openStreamLink(identStr);
			if(strmLinkPrx)
			{
				//do not clear dvbcResourceDataMap at runtime
				//_dvbcResourceDataMap.clear();

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
						TianShanIce::ValueMap PDData=ticketprx->getPrivateData();
						TianShanIce::SRM::ResourceMap dataMap=ticketprx->getResources();

						::TianShanIce::Variant bandwidth;
						::TianShanIce::Variant pn;

						Ice::Long	lBandwidth;
						Ice::Int	lPn;
						try
						{
							//check the bandwidth resource
							TianShanIce::SRM::ResourceMap::const_iterator it=dataMap.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
							if(it==dataMap.end())
							{
								MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"validate_DvbcStrmLinkConfig ticket %s but no bandwidth resource"),itID->name.c_str());
							}
							else
							{	
								TianShanIce::ValueMap val=it->second.resourceData;
								bandwidth=PDField(PDData,PathTicketPD_Field(bandwidth));
								MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"validate_DvbcStrmLinkConfig() ticket %s used bandwidth %lld"),itID->name.c_str(),bandwidth.lints[0]);
								lBandwidth = bandwidth.lints[0];
								la._availableBandwidth -= lBandwidth;								
								la._usedBandwidth += lBandwidth;
							}

							//check the program number resource
							it=dataMap.find(TianShanIce::SRM::rtMpegProgram);
							if(it==dataMap.end())
							{
								MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"validate_DvbcStrmLinkConfig() ticket %s but no porgram number resource"),itID->name.c_str());
							}
							else
							{
								TianShanIce::ValueMap val=it->second.resourceData;
								pn=PDField(PDData,PathTicketPD_Field(PN));

								//remove the used pn from attr
								int iPn=pn.ints[0];
								std::list<int>::iterator itAvailPN=la._availablePN.begin();
								while (itAvailPN!=la._availablePN.end() && iPn!=*itAvailPN)
								{
									itAvailPN++;
								}								
								if(itAvailPN==la._availablePN.end())
								{
									MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"validate_DvbcStrmLinkConfig() can't find the programnumber %d in available pn list"),pn.ints[0]);
								}
								else
								{
									MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,"validate_DvbcStrmLinkConfig() used porgram %d from available program number list"),pn.ints[0]);
									lPn = iPn;
									la._availablePN.erase(itAvailPN);
								}
							}

							ResourceDVBCData rdData;
							rdData._usedBandWidth	=	lBandwidth;
							rdData._usedPN			=	lPn;
							//_dvbcResourceDataMap.insert(ResourceDVBCDataMap::value_type(itID->name,rdData));
							_dvbcResourceDataMap[itID->name] = rdData;
						}
						catch(::TianShanIce::InvalidParameter& e)
						{
							MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"validate_DvbcStrmLinkConfig() invalidParameter exception is caught:%s"),e.message.c_str());
						}
						catch (...)
						{

						}
					}
				}
			}
			if( itAttr == _StreamLinkDVBCAttrmap.end() )
			{
				_StreamLinkDVBCAttrmap.insert(std::make_pair<std::string,LinkDVBCAttr>(la._streamLinkID,la));
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(BcastPHO,"insert a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedBandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
					identStr,la._totalBandWidth,la._usedBandwidth,la._totalPNCount,la._totalPNCount-(int)la._availablePN.size());
			}
			else
			{	
				itAttr->second	=	la;
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(BcastPHO,"update a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedbandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
					identStr,la._totalBandWidth,la._usedBandwidth,la._totalPNCount,la._totalPNCount-(int)la._availablePN.size());
			}	
		}
		else
		{
			ZQ::common::MutexGuard gd(_dvbcResourceLocker);

			LinkDVBCAttrMap::iterator itAttr=_StreamLinkDVBCAttrmap.find(la._streamLinkID);

			if( itAttr == _StreamLinkDVBCAttrmap.end() )
			{
				_StreamLinkDVBCAttrmap.insert(std::make_pair<std::string,LinkDVBCAttr>(la._streamLinkID,la));
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(BcastPHO,"insert a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedBandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
					identStr,la._totalBandWidth,la._usedBandwidth,la._totalPNCount,la._totalPNCount-(int)la._availablePN.size());
			}
			else
			{
				//TODO: filter out the used program number
				std::vector<Ice::Int> tmpTotal;
				LinkDVBCAttr& attr = itAttr->second;
				for ( Ice::Int iPort = attr._pnMin ; iPort <= attr._pnMax ; iPort ++ )
				{
					tmpTotal.push_back(iPort);
				}

				std::vector<Ice::Int> availOld;
				ZQTianShan::Util::copyListToVector<Ice::Int>( attr._availablePN , availOld );
				std::vector<Ice::Int> availNew;
				ZQTianShan::Util::copyListToVector<Ice::Int>( la._availablePN , availNew );


				std::sort( availOld.begin(),  availOld.end() , std::less<Ice::Int>( ));
				std::sort( availNew.begin(),  availNew.end() , std::less<Ice::Int>( ));

				std::vector<Ice::Int> tmpUsed;
				std::set_difference( tmpTotal.begin() , tmpTotal.end() , availOld.begin() , availOld.end(), std::inserter< std::vector<Ice::Int> >( tmpUsed , tmpUsed.begin( ) ) );

				std::vector<Ice::Int> tmpNew;
				std::set_difference( availNew.begin() , availNew.end() , tmpUsed.begin() , tmpUsed.end() , std::inserter< std::vector<Ice::Int> >( tmpNew , tmpNew.begin() ) );

				la._availablePN.clear();
				ZQTianShan::Util::copyVectorToList<Ice::Int>( tmpNew , la._availablePN );


				itAttr->second	=	la;
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(BcastPHO,"update a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedbandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
					identStr,la._totalBandWidth,la._usedBandwidth,la._totalPNCount,la._totalPNCount-(int)la._availablePN.size());

			}	
		}
	}
	catch(::TianShanIce::BaseException& e)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"tianshan expection caught :%s"),e.message.c_str());		
		e.ice_throw();
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"validate_DvbcStrmLinkConfig() unexpect error"));
	}

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastPHO, "leave validate a DVBC StreamLink's Configuration: link[%s]"), identStr);
}

#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
	if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
 else MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(BcastPHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}

Ice::Int BcastPHO::doEvaluation(LinkInfo& linkInfo, 
								 const SessCtx& sessCtx,
								 TianShanIce::ValueMap& hintPD,
								 const ::Ice::Int oldCost)
{
	std::string &linktype =linkInfo.linkType;

	if (0 == linktype.compare(STRMLINK_TYPE_MRT_PUBPOINT))
		return eval_BcastStrmLink(linkInfo, sessCtx, hintPD, linkInfo.rcMap,oldCost);

	return ::TianShanIce::Transport::OutOfServiceCost;
}

void BcastPHO::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{
	try
	{
		TianShanIce::ValueMap& ticketPD = ticket->privateData;  

		///get streamlinkID
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkId->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(BcastPHO,"doCommit() failed to get streamlink from ticket"));
			return ;
		}
		std::string strStreamlinkID = itLinkId->second.strs[0];
		std::string sessionId = sessCtx.sessId;

		MLOG(ZQ::common::Log::L_INFO, CLOGFMT (BcastPHO , "doCommit() [%s#%s]" ), strStreamlinkID.c_str(), sessionId.c_str());
	}
	catch (...){}
}
IPathHelperObject::NarrowResult BcastPHO::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
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
	if ( 0 == type.compare( STRMLINK_TYPE_MRT_PUBPOINT ) )
	{
		return narrow_BcastStrmLink( strmLink , sessCtx , ticket );
	}

	MLOG( ZQ::common::Log::L_ERROR, CLOGFMT(BcastPHO, "doNarrow() unrecognized stream link type [%s]"), type.c_str() );
	return IPathHelperObject::NR_Unrecognized;
}
int BcastPHO::evalBcastStreamLinkCost(const std::string& streamLinkID,Ice::Long bw2Alloc,const std::string& sessId)
{	
	ZQ::common::MutexGuard gd(_dvbcResourceLocker);
	LinkDVBCAttrMap::iterator it=_StreamLinkDVBCAttrmap.find(streamLinkID);
	if(it == _StreamLinkDVBCAttrmap.end())
	{
		std::string strErr="no streamlink attr is found through the streamlinkID=";
		strErr+=streamLinkID;
		throw strErr;
	}

	int pnCost=( ( it->second._totalPNCount - (int)it->second._availablePN.size() ) *
		(int)::TianShanIce::Transport::MaxCost) / 
		(it->second._totalPNCount);

	//should I put a mutex here???
	int bandwidthCost= ::TianShanIce::Transport::OutOfServiceCost;
	if( it->second._availablePN.size() <= 0 )
	{
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,
			"evalBcastStreamLinkCost() Session[%s] streamlink [%s] return with OutOfServiceCost: pnCount=[%d] and UsedPnCount=[%d] "
			"totalBW=[%lld] UsedBW=[%lld] ,pnCost=[%d]"),
			sessId.c_str(),streamLinkID.c_str(),
			it->second._totalPNCount ,it->second._totalPNCount- (int)it->second._availablePN.size(),
			it->second._totalBandWidth,it->second._usedBandwidth,pnCost);
		return TianShanIce::Transport::OutOfServiceCost;
	}

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(BcastPHO,
		"evalBcastStreamLinkCost() Session[%s] streamlink [%s] has pnCount=[%d] and UsedPnCount=[%d] "
		"totalBW=[%lld] UsedBW=[%lld] ,pnCost=[%d]"),
		sessId.c_str(),streamLinkID.c_str(),
		it->second._totalPNCount ,it->second._totalPNCount- (int)it->second._availablePN.size(),
		it->second._totalBandWidth,it->second._usedBandwidth,pnCost);

	Ice::Long usedBW=it->second._usedBandwidth;
	Ice::Long total=it->second._totalBandWidth;
	if( ( total-usedBW ) >= bw2Alloc)	
	{		
		bandwidthCost=(int) (  (usedBW)*(::TianShanIce::Transport::MaxCost) / total );		
		MLOG(ZQ::common::Log::L_INFO,
			CLOGFMT(BcastPHO,"evalBcastStreamLinkCost() Session[%s] streamlink [%s] allocbandwidth [%lld] "
			"totalBW=[%lld] UsedBW=[%lld] ,bwCost=[%d]"),
			sessId.c_str(),streamLinkID.c_str(),
			bw2Alloc,it->second._totalBandWidth,
			it->second._usedBandwidth,bandwidthCost);
	}
	int returnCost=max(pnCost,bandwidthCost);
	MLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(BcastPHO,"evalBcastStreamLinkCost() Session[%s] streamlink [%s] "
		" return cost=[%d] (pnCost=[%d],BWCost=[%d]) "),
		sessId.c_str(),streamLinkID.c_str(),returnCost,pnCost,bandwidthCost);

	return returnCost;
}
Ice::Int BcastPHO::eval_BcastStrmLink(LinkInfo& linkInfo,
									 const SessCtx& sessCtx,
									 TianShanIce::ValueMap& hintPD,
									 TianShanIce::SRM::ResourceMap& rcMap ,
									 const ::Ice::Int oldCost)
{
	int	newCost = oldCost;
	std::string sessId=sessCtx.sessId;

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastPHO, "[Session[%s] enter eval_DvbcStrmLink() with oldCost=%d"),sessId.c_str(),oldCost);

	if (oldCost > ::TianShanIce::Transport::MaxCost)
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"Session[%s] oldCost cost is bigger than MaxCost,return with OldCost=%d"),sessId.c_str(),oldCost);
		return oldCost;
	}

	// get resource information from session
	::TianShanIce::SRM::ResourceMap resourceMap =sessCtx.resources;	//sess->getReources();
	::Ice::Long bw2Alloc = 0;
	TianShanIce::ValueMap linkPD;
	try
	{
		linkPD = ::TianShanIce::Transport::StreamLinkPrx::checkedCast(linkInfo.linkPrx)->getPrivateData();		
	}
	catch (Ice::Exception& e)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"Session[%s] ice exception is caught:%s"),sessId.c_str(),e.ice_name().c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"Session[%s] unexpect error is threw out when call streamlink's getPrivateData"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	if (1)
	{
		try
		{
			// try to get the bandwidth requirement from the session context
			if (resourceMap.end() != resourceMap.find(::TianShanIce::SRM::rtTsDownstreamBandwidth))
			{
				::TianShanIce::SRM::Resource& tsDsBw = resourceMap[::TianShanIce::SRM::rtTsDownstreamBandwidth];
				if (tsDsBw.resourceData.end() != tsDsBw.resourceData.find("bandwidth") && 
					!tsDsBw.resourceData["bandwidth"].bRange &&
					tsDsBw.resourceData["bandwidth"].lints.size() !=0)
				{
					bw2Alloc = tsDsBw.resourceData["bandwidth"].lints[0];
				}
				else
				{
					MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(BcastPHO, "eval_DvbcStrmLink() unacceptable rtTsDownstreamBandwidth in session: bandwidth(range=%d, size=%d)"),
						tsDsBw.resourceData["bandwidth"].bRange,
						tsDsBw.resourceData["bandwidth"].lints.size());
				}
			}
			else
			{//error if no bandwidth parameter???
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"eval_DvbcStrmLink() Session[%s] unacceptable resource without bandwidth"),sessId.c_str());
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>( MLOG, EXPFMT("IpEdgePHO",1041,"eval_DvbcStrmLink() no 'bandwidth' is found in resoucemap"));
			}
		}
		catch (...)
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPHO, "eval_DvbcStrmLink() Session[%s] can not query the given session for resource info, stop evaluation"),sessId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	else
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(BcastPHO, "eval_DvbcStrmLink() Session[%s] no session specified, use hinted private data only"),sessId.c_str());

	// step 2, adjust if the hintPD also specify the bandwidth to the max of them
	if (hintPD.end() != hintPD.find(PathTicketPD_Field(bandwidth)))
	{
		::TianShanIce::Variant& var = hintPD[PathTicketPD_Field(bandwidth)];
		if (var.type == TianShanIce::vtLongs && var.lints.size() >0)
			bw2Alloc = (bw2Alloc >0) ? max(bw2Alloc, var.lints[0]) : var.lints[0];
	}

	// step 2.1, double check if the bandwidth is valid
	if (bw2Alloc <= 0)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPHO, "eval_DvbcStrmLink() Session[%s] 0 bandwidth has been specified, quit evaluation"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastPHO, "eval_DvbcStrmLink() Session[%s] required bandwidth:%lldbps"),sessId.c_str(), bw2Alloc);

	// 	TianShanIce::Variant valPN;
	// 	valPN.bRange=false;

	TianShanIce::Variant valBandwidth;
	valBandwidth.lints.clear();
	valBandwidth.lints.push_back(bw2Alloc);
	valBandwidth.type=TianShanIce::vtLongs;
	valBandwidth.bRange=false;
	try
	{
		newCost=evalBcastStreamLinkCost(linkInfo.linkIden.name , bw2Alloc , sessId);
		if(newCost > ::TianShanIce::Transport::MaxCost)
		{
			MLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(BcastPHO,"eval_DvbcStrmLink() Session[%s] streamLink=%s return with outofServiceCost with cost=%d"),
				sessId.c_str(),linkInfo.linkIden.name.c_str(),newCost);
			return newCost;
		}

	}
	catch (std::string& str)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"%s"),str.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	catch(...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"Session[%s] unexpect error when eval dvbc streamlink cost"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	//hintPD[PathTicketPD_Field(PN)] = valPN;
	hintPD[PathTicketPD_Field(bandwidth)] = valBandwidth;
	hintPD[PathTicketPD_Field(Qam.mode)] = linkPD["Qam.modulationFormat"];
	hintPD[PathTicketPD_Field(Qam.IP)] = linkPD["Qam.IP"];
	hintPD[PathTicketPD_Field(Qam.basePort)] = linkPD["Qam.basePort"];
	hintPD[PathTicketPD_Field(Qam.symbolRate)] = linkPD["Qam.symbolRate"];
	hintPD[PathTicketPD_Field(Qam.frequency)] = linkPD["Qam.frequency"];
	hintPD[PathTicketPD_Field(Streamer.SpigotID)]=linkPD["Streamer.SpigotID"];
	hintPD[PathTicketPD_Field(destAddr)] = linkPD["Qam.IP"];
	hintPD[PathTicketPD_Field(MRTPubpoint)] = linkPD["MRTPubPoint"];
#pragma message(__MSGLOC__"需要加入数据到resourceMap for ticket")	



	//fill rtEthernetInterface
	PutResourceMapData(rcMap,TianShanIce::SRM::rtEthernetInterface,"destIP",linkPD["Qam.IP"],sessId);
	//port 需要等到narrow的时候才能确定
	PutResourceMapData(rcMap,TianShanIce::SRM::rtEthernetInterface,"destMac",linkPD["Qam.Mac"],sessId);

	///fill rtTsDownstreamBandwidth	
	PutResourceMapData(rcMap,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth",valBandwidth,sessId);


	//fill rtPhysicalChannel
	PutResourceMapData(rcMap,TianShanIce::SRM::rtPhysicalChannel,"channelId",linkPD["Qam.frequency"],sessId);

	//fill rtAtscModulationMode
	PutResourceMapData(rcMap,TianShanIce::SRM::rtAtscModulationMode,"symbolRate",linkPD["Qam.symbolRate"],sessId);

	PutResourceMapData(rcMap,TianShanIce::SRM::rtAtscModulationMode,"modulationFormat",linkPD["Qam.modulationFormat"],sessId);


	MLOG(ZQ::common::Log::L_DEBUG,
		CLOGFMT(BcastPHO,"Session[%s] StreamLink=%s return with cost=%d"),
		sessId.c_str(),linkInfo.linkIden.name.c_str(),max(oldCost,newCost));
	return max(oldCost,newCost);
}


IPathHelperObject::NarrowResult BcastPHO::narrow_BcastStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink,
															   const SessCtx& sessCtx,
															   const TianShanIce::Transport::PathTicketPtr&  ticket)
{	
	std::string	strStrmLinkID;
	try
	{
		strStrmLinkID=strmLink->getIdent().name;
	}
	catch (...) 
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"narrow_DvbcStrmLink() get streamlink's ID failed"));
		return NR_Error;
	}

	//如何narrow呢?
	//step 1.find a unused program number
	int pn=0;
	int port=0;
	TianShanIce::SRM::ResourceMap::iterator itResMap=ticket->resources.find(TianShanIce::SRM::rtEthernetInterface);
	std::string sessId= sessCtx.sessId;

	if(itResMap==ticket->resources.end())
	{
		MLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(BcastPHO,"narrow_DvbcStrmLink() Session[%s] can't find "
			"rtEthernetInterface from ticket resources"),
			sessId.c_str());
		return 	NR_Error;		 
	}
	std::string&	strTicketID = ticket->ident.name;

	TianShanIce::SRM::ResourceMap& resMap = ticket->resources;
	int AvailPnCount=0;
	Ice::Long  bw2Alloc;
	std::string strMRTPubpoint;
	{
		ZQ::common::MutexGuard gd(_dvbcResourceLocker);	
		READ_RES_FIELD(bw2Alloc, resMap, rtTsDownstreamBandwidth, bandwidth, lints);
		LinkDVBCAttrMap::iterator it=_StreamLinkDVBCAttrmap.find(  strStrmLinkID );
		if(it==_StreamLinkDVBCAttrmap.end())
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(BcastPHO,"narrow_DvbcStrmLink() Session[%s] can't find the streamlink attribute through streamlink[%s] for ticket[%s]"),
				sessId.c_str(),strStrmLinkID.c_str(),strTicketID.c_str() );
			return NR_Error;
		}
		if(it->second._availablePN.size()<=0)
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(BcastPHO,"Session[%s] streamlink[%s] not enough available program number,totalPN[%d] usedPN[%d] for ticket[%s]"),
				sessId.c_str(),strStrmLinkID.c_str(),it->second._totalPNCount,it->second._totalPNCount-it->second._availablePN.size(), strTicketID.c_str() );
			return NR_Error;				 
		}
		if (it->second._totalBandWidth <= it->second._usedBandwidth) 
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(BcastPHO,"narrow_DvbcStrmLink() Session[%s] strmLink[%s] no available Badnwidth,totalBW[%lld] usedBW[%lld] for ticket[%s]"),
				sessId.c_str(),strStrmLinkID.c_str(),it->second._totalBandWidth,it->second._usedBandwidth , strTicketID.c_str() );
			return NR_Error;
		}

		//modify pn allocation algorithm
		std::list<int>::iterator itPn=it->second._availablePN.begin();
		static bool bRandInit=false;
		if(!bRandInit)
		{
			srand(time(NULL));
			bRandInit=true;
		}

		int iOffset= 0;
		if( (int)it->second._availablePN.size() > 1 )
		{
			iOffset = rand() % ( it->second._availablePN.size() -1 );
			iOffset=iOffset>5?5:iOffset;
		}
		for(int i=0;i<iOffset ;i++)
			itPn++;

		pn =*itPn;
		it->second._availablePN.erase(itPn);

		//it->second._availableBandwidth-=bw2Alloc;
		it->second._usedBandwidth += bw2Alloc;

		//step 2.calculate the udp port
		port=it->second._basePort + it->second._portStep * (pn);

		AvailPnCount=it->second._availablePN.size();

		strMRTPubpoint = it->second._MRTUrl;
		MLOG(ZQ::common::Log::L_INFO,
			CLOGFMT(BcastPHO,"Session[%s] streamlink=[%s] ticketID=[%s] narrowed with BWAlloc=[%lld] PN=[%d]"
			" Now totalBW=[%lld] usedBW=[%lld] totalPn=[%d] usedPn=[%d]MRTPubpoint[%s]"),
			sessId.c_str(),strStrmLinkID.c_str(),strTicketID.c_str(),
			bw2Alloc,pn,it->second._totalBandWidth , it->second._usedBandwidth,
			it->second._totalPNCount,it->second._totalPNCount-(int)it->second._availablePN.size(), strMRTPubpoint.c_str());
	}	

	TianShanIce::Variant valPN;
	TianShanIce::Variant valPort;
	TianShanIce::Variant valBandwidth;


	valPN.bRange=false;
	valPN.type = TianShanIce::vtInts;
	valPN.ints.clear();
	valPN.ints.push_back(pn);
	valPN.type=TianShanIce::vtInts;
	ticket->privateData[PathTicketPD_Field(PN)]=valPN;
	PutResourceMapData(resMap,TianShanIce::SRM::rtMpegProgram,"Id",valPN,sessId);

	valPort.bRange=false;
	valPort.type = TianShanIce::vtInts;
	valPort.ints.clear();
	valPort.ints.push_back(port);
	valPort.type=TianShanIce::vtInts;
	ticket->privateData[PathTicketPD_Field(destPort)]=valPort;
	PutResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destPort",valPort,sessId);

	valBandwidth.bRange = false;
	valBandwidth.lints.clear();
	valBandwidth.type =TianShanIce::vtLongs;
	valBandwidth.lints.push_back(bw2Alloc);
	ticket->privateData[PathTicketPD_Field(bandwidth)] = valBandwidth;

	TianShanIce::Variant varLinkType;
	TianShanIce::Variant varLinkId;

	varLinkType.bRange = false;
	varLinkType.type = TianShanIce::vtStrings;
	varLinkType.strs.clear();
	varLinkType.strs.push_back( STRMLINK_TYPE_MRT_PUBPOINT );
	ticket->privateData[PathTicketPD_Field(ownerStreamLinkType)] = varLinkType;

	varLinkId.bRange = false;
	varLinkId.type = TianShanIce::vtStrings;
	varLinkId.strs.clear();
	varLinkId.strs.push_back( strStrmLinkID );
	ticket->privateData[PathTicketPD_Field(ownerStreamLinkId)] = varLinkId;
	
	TianShanIce::Variant varMRTPubpoint;
	varMRTPubpoint.bRange = false;
	varMRTPubpoint.type = TianShanIce::vtStrings;
	varMRTPubpoint.strs.clear();
	varMRTPubpoint.strs.push_back(strMRTPubpoint);
	ticket->privateData[PathTicketPD_Field(MRTPubpoint)] = varMRTPubpoint;


	{
		ZQ::common::MutexGuard gd(_dvbcResourceLocker);
		ResourceDVBCData rdd;
		rdd._usedBandWidth = bw2Alloc;
		rdd._usedPN = pn;
		_dvbcResourceDataMap[strTicketID] = rdd;

	}

	MLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(BcastPHO,"narrow_DvbcStrmLink() Session[%s] strmLink[%s] get dvbcresource with"
		" pn=[%d] port=[%d] bw=[%lld] with ticketID=[%s]"),
		sessId.c_str(),strStrmLinkID.c_str(), pn,port,bw2Alloc ,strTicketID.c_str());

	return NR_Narrowed;
}

::Ice::Byte getPenalty(const TianShanIce::ValueMap& privateData ) 
{
	//privateData
	::TianShanIce::ValueMap::const_iterator it = privateData.find("StreamerPenalty_value");
	if( it == privateData.end() )
	{
		return 0;
	}
	else
	{
		TianShanIce::Variant var = it->second;
		if( var.type == TianShanIce::vtInts  && var.ints.size() > 0 )
		{
			::Ice::Byte penalty = static_cast<Ice::Byte>(var.ints[0]);
			return penalty;
		}
		else
		{
			return 0;
		}
	}
}

void BcastPHO::increasePenalty ( const std::string& streamerId , int maxPenaltyValue )
{
	ZQ::common::MutexGuard gd(_streamerMapLock);
	streamerAttrMap::iterator it = _streamerMap.find ( streamerId );
	if ( it != _streamerMap.end ( ) ) 
	{
		MLOG(ZQ::common::Log::L_INFO ,
			CLOGFMT( IpEdgePHO , "set stremer[%s]'s penalty value to max penalty value[%d]" ),
			streamerId.c_str () ,  maxPenaltyValue );

#pragma message(__MSGLOC__" !!!!! Add a configuration for MAX_PENALTY value !!!!!!!!!!!!!! ")
		it->second.penaltyValue = maxPenaltyValue;
	}
	else
	{
		streamerAttr attr;
		attr.streamerId = streamerId;
		attr.penaltyValue = maxPenaltyValue;
		MLOG(ZQ::common::Log::L_INFO ,
			CLOGFMT( IpEdgePHO , "set stremer[%s]'s penalty value to max penalty value[%d]" ),
			streamerId.c_str () ,  maxPenaltyValue );
		_streamerMap.insert ( streamerAttrMap::value_type( streamerId , attr ) );
	}	
}
void BcastPHO::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
{
	try
	{
		TianShanIce::ValueMap& ticketPD = ticket->privateData;

		Ice::Identity& ticketID = ticket->ident;

		//get destroy reason if there is any
		Ice::Int penaltyValue = getPenalty(ticketPD);
		if( penaltyValue > 0 )
		{

			TianShanIce::Transport::StreamLinkPrx streamLink = _phoManager->openStreamLink (ticket->streamLinkIden);			
			std::string streamerId = streamLink->getStreamerId();
			if(!streamerId.empty())
				increasePenalty( streamerId , penaltyValue );
		}

		TianShanIce::ValueMap::const_iterator itLinkType = ticketPD.find( PathTicketPD_Field(ownerStreamLinkType) ) ;
		if ( itLinkType == ticketPD.end()  || itLinkType->second.type != TianShanIce::vtStrings ||itLinkType->second.strs.size() == 0  ) 
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"no ticket owner link type is found for ticket[%s], this ticket may not be narrowed") , ticketID.name.c_str() );
			return;
		}
		std::string strLinkType = itLinkType->second.strs[0];
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkType->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() can' get streamlink from ticket[%s], this ticket may not be narrowed"),ticketID.name.c_str() );
			return ;
		}
		std::string strStramlinkID = itLinkId->second.strs[0];

		TianShanIce::Variant val;


		if( strcmp(STRMLINK_TYPE_MRT_PUBPOINT,strLinkType.c_str())==0 )
		{//dvbc mode
			ZQ::common::MutexGuard gd(_dvbcResourceLocker);

			//find if there is a allocated dvbc resource
			ResourceDVBCDataMap::iterator itAlloc=_dvbcResourceDataMap.find(ticketID.name);
			if( itAlloc == _dvbcResourceDataMap.end() )
			{//if not,return without free any resource
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(BcastPHO,"doFreeResource() no allocated dvbc resource with tickID=[%s]"),
					ticketID.name.c_str());
				return;
			}

			LinkDVBCAttrMap::iterator it=_StreamLinkDVBCAttrmap.find(strStramlinkID);
			if(it==_StreamLinkDVBCAttrmap.end())
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"doFreeResource() can't find strmlink attr through streamlink[%s] for ticket[%s]"),strStramlinkID.c_str() , ticketID.name.c_str() );
				return;
			}

			try
			{
				//	TianShanIce::ValueMap& value=ticket->privateData;

				TianShanIce::Variant val;
				TianShanIce::SRM::ResourceMap& resMap=ticket->resources;
				try
				{
					val=GetResourceMapData(resMap,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth");
				}
				catch (TianShanIce::InvalidParameter&)
				{
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() can't find bandwith in resources,invalid resources for ticket[%s]"), ticketID.name.c_str());
				}
				if (val.type!=TianShanIce::vtLongs || val.lints.size()<=0|| val.lints[0]!= itAlloc->second._usedBandWidth ) 
				{
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() can't find bandwith in resources or invalid bandwidth data for ticket[%s]"),ticketID.name.c_str() );
				}				
				try
				{
					val=GetResourceMapData(resMap,TianShanIce::SRM::rtMpegProgram,"Id");
				}
				catch(TianShanIce::InvalidParameter&)
				{
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() can't find program number in resources for ticket[%s]"),ticketID.name.c_str());
				}
				if ((val.type!=TianShanIce::vtInts) || (val.ints.size()==0) ||(val.ints[0]!=itAlloc->second._usedPN)) 
				{
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() can't find program number in resources or invalid pn data for ticket[%s] "),ticketID.name.c_str() );
				}
			}
			catch (Ice::Exception& ex) 
			{
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() ice exception [%s] error when validate the ticket private data for ticket[%s]"),
					ex.ice_name().c_str(),ticketID.name.c_str());
			}
			catch (...) 
			{
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() unexpetc error when validate the ticket private data for ticket[%s]") , ticketID.name.c_str() );
			}


			//free resource allocated by narrow function
			Ice::Long bandwidth=itAlloc->second._usedBandWidth;
			it->second._availableBandwidth+=bandwidth;
			it->second._usedBandwidth -= bandwidth;

			it->second._usedBandwidth = it->second._usedBandwidth > 0 ? it->second._usedBandwidth : 0;

			{
				int pn=itAlloc->second._usedPN;
				if ( it->second._pnMax < 0 && it->second._pnMin < 0  ) 
				{
					bool bReleased = false;
					std::list<Ice::Int>& backLst = it->second._backupPN;
					std::list<Ice::Int>::const_iterator itBackup = backLst.begin();
					for (  ; itBackup != backLst.end() ; itBackup++) 
					{
						if ( pn == *itBackup ) 
						{
							it->second._availablePN.push_back(pn);
							MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() streamlink [%s]"
								" [ticket %s] with PN=[%d] BW=[%lld] and now totalBW=%lld usedBW=%lld  totalPN=[%d] usedPN=[%d]"),
								strStramlinkID.c_str(),ticketID.name.c_str(),
								pn,bandwidth,
								it->second._totalBandWidth,it->second._usedBandwidth,
								it->second._backupPN.size(),it->second._backupPN.size()-(int)it->second._availablePN.size());
							bReleased = true;
						}
					}
					if ( !bReleased) 
					{
						MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() streamlink [%s]"
							" [ticket %s] with PN=[%d](discard pnMax[%d] pnMin[%d]) BW=[%lld] and now totalBW=%lld usedBW=%lld  totalPN=[%d] usedPN=[%d]"),
							strStramlinkID.c_str(),ticketID.name.c_str(),
							pn, it->second._pnMax,it->second._pnMin, bandwidth,
							it->second._totalBandWidth,it->second._usedBandwidth,
							it->second._backupPN.size() , it->second._backupPN.size() - (int)it->second._availablePN.size());
					}
				}
				else if (pn<=it->second._pnMax &&pn>=it->second._pnMin) 
				{
					it->second._availablePN.push_back(pn);
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() streamlink [%s]"
						" [ticket %s] with PN=[%d] BW=[%lld] and now totalBW=%lld usedBW=%lld  totalPN=[%d] usedPN=[%d]"),
						strStramlinkID.c_str(),ticketID.name.c_str(),
						pn,bandwidth,
						it->second._totalBandWidth,it->second._usedBandwidth,
						it->second._totalPNCount,it->second._totalPNCount-(int)it->second._availablePN.size());

				}	
				else
				{
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(BcastPHO,"doFreeResource() streamlink [%s]"
						" [ticket %s] with PN=[%d](discard pnMax[%d] pnMin[%d]) BW=[%lld] and now totalBW=%lld usedBW=%lld  totalPN=[%d] usedPN=[%d]"),
						strStramlinkID.c_str(),ticketID.name.c_str(),
						pn, it->second._pnMax,it->second._pnMin, bandwidth,
						it->second._totalBandWidth,it->second._usedBandwidth,
						it->second._totalPNCount,it->second._totalPNCount-(int)it->second._availablePN.size());

				}	
			}			
			_dvbcResourceDataMap.erase(itAlloc);
		}
		else
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"doFreeResource() unrecognised stram link [%s] type [%s] for ticket[%s]"),
				strStramlinkID.c_str() ,strLinkType.c_str() , ticketID.name.c_str() );
			return;
		}
	}
	catch (TianShanIce::BaseException& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"doFreeResource() catch a tianshanice exception:%s"),ex.message.c_str());
		return;
	}
	catch(Ice::Exception& e)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"doFreeResource() catch a ice exception:%s"),e.ice_name().c_str());
		return;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(BcastPHO,"doFreeResource() catch a unexpect exception"));
		return;
	}	

}

}} // namespace




