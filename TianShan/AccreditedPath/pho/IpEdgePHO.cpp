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
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/pho/IpEdgePHO.cpp $
// 
// 11    1/11/16 4:57p Dejian.fei
// 
// 10    10/15/14 11:39a Hongquan.zhang
// 
// 9     12/12/13 1:32p Hui.shao
// %lld
// 
// 8     9/05/13 3:26p Zonghuan.xiao
// modify log from glog to mlog because glog cannot confirm which pclog
// used
// 
// 7     12/20/12 11:50a Hui.shao
// 
// 6     9/18/12 2:49p Hongquan.zhang
// 
// 5     9/17/12 3:42p Hongquan.zhang
// 
// 4     3/17/11 5:04p Fei.huang
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
// 50    09-08-19 15:51 Hongquan.zhang
// 
// 49    08-12-02 18:15 Hongquan.zhang
// 
// 48    08-11-24 15:29 Hongquan.zhang
// 
// 47    08-08-11 15:44 Hongquan.zhang
// 
// 46    08-07-08 16:00 Hongquan.zhang
// 
// 44    08-03-24 17:47 Build
// check in for 64bit build
// 
// 43    08-03-18 14:49 Hongquan.zhang
// check in for StreamLink Penalty and DVBC share link
// 
// 42    08-01-03 18:22 Hongquan.zhang
// 
// 41    08-01-03 15:55 Hui.shao
// move PDSchema namespace
// 
// 40    07-12-25 16:37 Hongquan.zhang
// 
// 39    07-12-25 16:23 Hongquan.zhang
// Change the schema type
// Now bRange can be TRUE and FALSE
// 
// 38    07-12-14 11:38 Hongquan.zhang
// update Error Code
// 
// 37    07-11-19 11:49 Hongquan.zhang
// 
// 36    07-10-08 13:39 Yixin.tian
// 
// 35    07-09-18 12:55 Hongquan.zhang
// 
// 34    07-08-30 15:39 Hongquan.zhang
// 
// 33    07-08-01 10:47 Hongquan.zhang
// 
// 32    07-07-02 12:07 Hongquan.zhang
// 
// 31    07-06-27 10:01 Hongquan.zhang
// 
// 30    07-05-24 11:19 Hongquan.zhang
// 
// 29    07-05-16 16:55 Hongquan.zhang
// 
// 12    06-09-18 19:26 Hui.shao
// use plugin for PHO
// 
// 11    06-09-05 12:44 Hui.shao
// 
// 10    06-08-28 18:12 Hui.shao
// 
// 9     06-08-25 14:28 Hui.shao
// 
// 8     06-08-24 19:22 Hui.shao
// 
// 7     06-08-11 11:08 Hui.shao
// 
// 6     06-08-10 15:46 Hui.shao
// 
// 5     06-08-10 12:46 Hui.shao
// moved bandwidth into the privateData
// 
// ===========================================================================

#include "IpEdgePHO.h"
#include "Log.h"
#include <public.h>
#include <time.h>
#include <algorithm>
#include <TianShanIceHelper.h>

extern ZQ::common::Log* pho_Seachangelog;
#define MLOG (*pho_Seachangelog)

namespace ZQTianShan {
namespace AccreditedPath {
	
///schema for STRMLINK_TYPE_IPEDGE_IP
static ConfItem IpEdge_IP[] = {
	{ "TotalBandwidth",		::TianShanIce::vtLongs,		false,	"20000",	false }, // in Kbps
	{ "MaxStreamCount",		::TianShanIce::vtInts,		false,	"80",		false },
	{ "SourceIp" ,			::TianShanIce::vtStrings,	false,	"0.0.0.0",	false },
	{ "DestMac" ,			::TianShanIce::vtStrings,	false,	"",			false },
	{ "SourcePort" ,		::TianShanIce::vtInts ,		false,	"30000",	false },
	{ NULL,					::TianShanIce::vtInts,		true,	"" ,		false },
};

///schema for STRMLINK_TYPE_IPEDGE_IPSHARELINK
static ::ZQTianShan::ConfItem IpEdge_IP_ShareLink[]= {
	{"LinkId",	::TianShanIce::vtStrings,	false , "" , false	},
	{NULL, ::TianShanIce::vtInts, true, "",false },
};

///schema for STRMLINK_TYPE_IPEDGE_DVBC
static ::ZQTianShan::ConfItem IpEdge_DVBC[] = {	
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
	{ NULL, ::TianShanIce::vtInts, true, "",false },
};

///schema for STRMLINK_TYPE_IPEDGE_DVBCSHARELINK
static ::ZQTianShan::ConfItem IpEdge_DVBC_ShareLink[]= {
	{"LinkId",	::TianShanIce::vtStrings,	false , "" , false	},
	{NULL, ::TianShanIce::vtInts, true, "",false },
};

IpEdgePHO::IpEdgePHO(IPHOManager& mgr)
	: IStreamLinkPHO(mgr)
{
	_bQuitDecreasePenaltyThread = false;
	_phoManager=&mgr;
#ifndef PHO_SEACHANGE_EXPORTS // if is a plugin, let the InitPHO do this
	_helperMgr.registerStreamLinkHelper(STRMLINK_TYPE_IPEDGE_IP, *this, NULL);
	_helperMgr.registerStreamLinkHelper(STRMLINK_TYPE_IPEDGE_DVBC, *this, NULL);
	_helperMgr.registerStreamLinkHelper(STRMLINK_TYPE_IPEDGE_DVBCSHARELINK, *this, NULL);
	_helperMgr.registerStreamLinkHelper(STRMLINK_TYPE_IPEDGE_IPSHARELINK, *this, NULL);
#endif // PHO_SEACHANGE_EXPORTS
	start ();
}

IpEdgePHO::~IpEdgePHO()
{
	_bQuitDecreasePenaltyThread = true;
	_hEventForDereasePenalty.signal();
	waitHandle (1000);
	_helperMgr.unregisterStreamLinkHelper(STRMLINK_TYPE_IPEDGE_IP);
	_helperMgr.unregisterStreamLinkHelper(STRMLINK_TYPE_IPEDGE_DVBC);
	_helperMgr.unregisterStreamLinkHelper(STRMLINK_TYPE_IPEDGE_DVBCSHARELINK);
	_helperMgr.unregisterStreamLinkHelper(STRMLINK_TYPE_IPEDGE_IPSHARELINK);
}

bool IpEdgePHO::getSchema(const char* type, ::TianShanIce::PDSchema& schema)
{
	::ZQTianShan::ConfItem *config = NULL;
	
	// address the schema definition
	if (0 == strcmp(type, STRMLINK_TYPE_IPEDGE_IP))
		config = IpEdge_IP;
	else if (0 == strcmp(type, STRMLINK_TYPE_IPEDGE_DVBC))
		config = IpEdge_DVBC;
	else if ( 0 == strcmp(type, STRMLINK_TYPE_IPEDGE_DVBCSHARELINK) ) 
		config = IpEdge_DVBC_ShareLink;
	else if ( 0 == strcmp(type , STRMLINK_TYPE_IPEDGE_IPSHARELINK ) )
		config = IpEdge_IP_ShareLink;
	
	// no matches
	if (NULL == config)
		return false;

	// copy the schema to the output
	for (::ZQTianShan::ConfItem *item = config; item && item->keyname; item ++)
	{
		::TianShanIce::PDElement elem;
		elem.keyname = item->keyname;
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

void IpEdgePHO::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	if (NULL == type)
		type = "NULL";
	
	//initialize the penalty value to 0 if no streamer is found in _streamerMap
	try
	{
		std::string streamerId = "";
		TianShanIce::Transport::StreamLinkPrx strmLinkPrx = _phoManager->openStreamLink (identStr);
		if (strmLinkPrx) 
		{
			streamerId = strmLinkPrx->getStreamerId ();
			ZQ::common::MutexGuard gd(_streamerMapLock);
			if ( _streamerMap.find ( streamerId ) == _streamerMap.end () ) 
			{
				streamerAttr attr;
				attr.streamerId = streamerId;
				attr.penaltyValue = 0;
				_streamerMap.insert(streamerAttrMap::value_type( streamerId , attr ) ) ;
				MLOG(ZQ::common::Log::L_INFO , 
					CLOGFMT(IpEdgePHO , "validate_IPStrmLinkConfig() update streamer[%s] with penalty[%d] through stremlink[%s]" ),
					streamerId.c_str() ,
					0,
					identStr );
			}
		}
		else
		{
			MLOG(ZQ::common::Log::L_ERROR ,
				CLOGFMT(IpEdgePHO , "validate_IPStrmLinkConfig() can't get streamerlink[%s]"), identStr);
		}	
		
	}
	catch ( const Ice::ObjectNotExistException& )
	{//do nothing
	}

	if (0 == strcmp(STRMLINK_TYPE_IPEDGE_IP, type))
	{
		validate_IPStrmLinkConfig(identStr, configPD);
	}
	else if (0 == strcmp(STRMLINK_TYPE_IPEDGE_DVBC, type))
	{
		validate_DvbcStrmLinkConfig(identStr, configPD);
	}
	else if ( 0 == strcmp(STRMLINK_TYPE_IPEDGE_DVBCSHARELINK,type) )
	{
		validate_DvbcSharedStrmLinkConfig(identStr,configPD);
	}
	else if( 0 == strcmp(STRMLINK_TYPE_IPEDGE_IPSHARELINK , type ) )
	{
		validate_IPSharedStrmLinkConfig(identStr ,configPD );
	}
	else
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgePHO",1001,"unsupported type [%s] in IpEdgePHO for [%s] ", type , identStr );
	}
	configPD.erase( _SERVICE_STATE_KEY_ );
}

void IpEdgePHO::increasePenalty ( const std::string& streamerId , int maxPenaltyValue )
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
int IpEdgePHO::run ()
{
	while ( !_bQuitDecreasePenaltyThread ) 
	{
		_hEventForDereasePenalty.wait();
		if ( _bQuitDecreasePenaltyThread ) 
		{
			break;
		}
		long lDecreaseCount = 0;
		{
			ZQ::common::MutexGuard gd( _decreaseCountLock );
			lDecreaseCount = _decreaseCount;
			_decreaseCount = 0;
		}
		if (lDecreaseCount > 0)
		{
			ZQ::common::MutexGuard gd( _streamerMapLock );
			streamerAttrMap::iterator it = _streamerMap.begin ( ) ; 
			for ( ; it != _streamerMap.end () ; it ++ ) 
			{
				if ( it->second.penaltyValue > 0 ) 
				{
					it->second.penaltyValue -= lDecreaseCount;
					if ( it->second.penaltyValue <0  ) 
					{
						it->second.penaltyValue = 0;
					}
					MLOG(ZQ::common::Log::L_INFO , 
						CLOGFMT( IpEdgePHO , "adjust streamer [%s] 's penalty to [%d] according to decreaseCount[%d]"),
						it->first.c_str() ,
						it->second.penaltyValue , lDecreaseCount);
				}
			}
		}
	}
	return 1;
}

void IpEdgePHO::decreasePenalty ( const std::string& streamerId )
{
	//in oder to decrease,should I put a thread request ?
	{
		ZQ::common::MutexGuard gd( _decreaseCountLock );	
		if ( _decreaseCount < 0) 
		{
			_decreaseCount =0;
		}
		_decreaseCount ++ ;
	}
	 _hEventForDereasePenalty.signal();
}

Ice::Int IpEdgePHO::doEvaluation(LinkInfo& linkInfo, 
								 const SessCtx& sessCtx,
								 TianShanIce::ValueMap& hintPD,
								 const ::Ice::Int oldCost)
{
	std::string &linktype =linkInfo.linkType;


	{
		//Check the target streamer's penalty,if the penalty value  > 0
		//return with out-of-service cost
		ZQ::common::MutexGuard gd(_streamerMapLock);
		streamerAttrMap::const_iterator it = _streamerMap.find ( linkInfo.streamerId );
		if ( it != _streamerMap.end () ) 
		{
			if ( it->second.penaltyValue > 0 ) 
			{
				MLOG(ZQ::common::Log::L_ERROR , CLOGFMT( IpEdgePHO , "doEvaluation() get the streamer[%s] 's penalty [%d] > 0,return with out-of-service cost" ) ,
					linkInfo.streamerId.c_str () ,
					it->second.penaltyValue 	);
				decreasePenalty(linkInfo.streamerId);
				return TianShanIce::Transport::OutOfServiceCost;
			}
			else
			{
				decreasePenalty(linkInfo.streamerId);
			}
		}
		else
		{
			decreasePenalty(linkInfo.streamerId);
			MLOG(ZQ::common::Log::L_WARNING , CLOGFMT( IpEdgePHO , "doEvaluation() can't get the streamer[%s] 's penalty value" ) , linkInfo.streamerId.c_str () );
		}
	}

	if (0 == linktype.compare(STRMLINK_TYPE_IPEDGE_IP))
		return eval_IPStrmLink(linkInfo, sessCtx, hintPD, linkInfo.rcMap,oldCost);
	
	if (0 == linktype.compare(STRMLINK_TYPE_IPEDGE_DVBC))
		return eval_DvbcStrmLink(linkInfo, sessCtx, hintPD,linkInfo.rcMap, oldCost);

	if (0 == linktype.compare(STRMLINK_TYPE_IPEDGE_DVBCSHARELINK )) 
		return eval_DvbcShareStrmLink(linkInfo,sessCtx,hintPD,linkInfo.rcMap,oldCost);

	if( 0 == linktype.compare(STRMLINK_TYPE_IPEDGE_IPSHARELINK))
		return eval_IPShareStrmLink( linkInfo , sessCtx , hintPD , linkInfo.rcMap ,oldCost );

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
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IpEdgePHO, " enter validate a IP StreamLink's Configuration: link[%s]"), identStr);

	LinkIPAttr la;
	la._streamLinkID=identStr;
	::TianShanIce::Variant val;
	val.bRange=false;
	try
	{
		// get TotalBandwidth
		val=PDField(configPD,"TotalBandwidth");
		if(val.type!=::TianShanIce::vtLongs)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1011,"Invalid TotalBandwidth type,should be vtLongs for StreamLink[%s]",identStr);
		}
		if (val.lints.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgaPHO",1012,"Invalid TotalBandwidth ,lints size() == 0  for streamLink[%s]",identStr);
		}
		la._totalBandwidth=val.lints[0]*1000;
		la._availableBandwidth=la._totalBandwidth;
		la._usedBandwidth = 0;
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"stream IP link[%s] get totalBW[%lld]"),identStr,la._totalBandwidth);
		
		//get maxStreamCount
		try
		{
			val=PDField(configPD,"MaxStreamCount");
			if (val.type == TianShanIce::vtInts && val.ints.size() > 0) 
			{
				la._totalStreamCount = val.ints[0];
				la._usedStreamCount = 0;
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"stream IP link[%s] get totalStreamCount[%d]"),identStr,la._totalStreamCount);
			}
			else
			{
				la._totalStreamCount = 80;
				la._usedStreamCount = 0;
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IpEdgePHO,"stream IP link[%s] ,invalid maxStreamCount type or data,set it to default [80]"),identStr);
			}
		}
		catch (const ::TianShanIce::InvalidParameter& ) 
		{//maxStreamCount isn't found

			la._totalStreamCount = 80;//hard code the default value
			la._usedStreamCount = 0;
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IpEdgePHO,"stream IP link[%s] ,MaxStreamCount is not found,set it to default [80]"),identStr);
		}

		try
		{
			val = PDField( configPD , "DestMac");
			if ( val.type == TianShanIce::vtStrings && val.strs.size() > 0 ) 
			{
				la._destMacAddress = val.strs[0];
				MLOG(ZQ::common::Log::L_INFO , 
					CLOGFMT(IpEdgePHO , "stream IP link[%s] , set destMacAddress to [%s]"),
					identStr , la._destMacAddress.c_str() );
			}
			else
			{
				la._destMacAddress = "";
				MLOG(ZQ::common::Log::L_INFO ,
					CLOGFMT(IpEdgePHO , "stream IP link[%s] , destMacAddress is not found ,set it to empty"),
					identStr );
			}
		}
		catch ( const ::TianShanIce::InvalidParameter& ) 
		{
			la._destMacAddress = "";
			MLOG( ZQ::common::Log::L_INFO ,
				CLOGFMT(IpEdgePHO ,"Stream IP link[%s] , destMacAddress is not found ,set it to empty" ),
				identStr );
		}

		//get source stream IP
		try
		{
			val =  PDField (configPD,"SourceIp");
			if (val.type == TianShanIce::vtStrings && val.strs.size() > 0 )
			{
				la._srcStreamIP = val.strs[0];
				MLOG(ZQ::common::Log::L_DEBUG , CLOGFMT(IpEdgePHO,"Stream IP link [%s] ,get Source Streamer Ip [%s]"),
												identStr , la._srcStreamIP.c_str ()	);
			}
			else
			{
				la._srcStreamIP = "0.0.0.0";
				MLOG(ZQ::common::Log::L_WARNING , CLOGFMT(IpEdgePHO,"Stream IP link[%s] ,Can't get Source Streamer Ip,set it to [%s]"),
												identStr , "0.0.0.0");
			}
		}
		catch (const TianShanIce::InvalidParameter&)
		{
			la._srcStreamIP = "0.0.0.0";
			MLOG(ZQ::common::Log::L_WARNING , CLOGFMT(IpEdgePHO,"Stream IP link[%s] ,Can't get Source Streamer Ip,set it to [%s]"),
												identStr , "0.0.0.0");
		}

		//get source stream port
		bool	bGetSourcePort =  false;;
		try
		{
			val = PDField (configPD ,"SourcePort" );
			bGetSourcePort =true;
		}
		catch (const TianShanIce::InvalidParameter&)
		{
			la._minSrcPort = la._maxSrcPort =-1;
			la._availSrcPort.clear ();
			bGetSourcePort = false;
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(IpEdgePHO,"Stream IP link[%s],Can't get stream source port,set it to -1,that is leave this value to vstrm"),
											identStr);
		}
		if (bGetSourcePort)
		{
			
			if ( val.type == TianShanIce::vtInts && val.ints.size () >0  )
			{
				la._availSrcPort.clear ();
				if (val.bRange == true )
				{
					if (val.ints.size () < 2 )
					{
						ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgaPHO",1021,"Stream IP link[%s],bRange = true but data size <2",identStr);
					}
					la._minSrcPort = val.ints[0];
					la._maxSrcPort = val.ints[1];
					if ( la._minSrcPort > la._maxSrcPort)
					{
						Ice::Int tempPort = la._maxSrcPort;
						la._maxSrcPort = la._minSrcPort;
						la._minSrcPort = tempPort;
					}
					MLOG(ZQ::common::Log::L_DEBUG , CLOGFMT(IpEdgePHO,"Stream IP link[%s] , get minSourcePort[%d] MaxSourcePort[%d]"),
												identStr , la._minSrcPort ,la._maxSrcPort );
					la._availSrcPort.clear ();
					for (int iPort = la._minSrcPort ; iPort < la._maxSrcPort ; iPort++ )
					{
						la._availSrcPort.push_front (iPort);
					}
				}
				else
				{
					la._minSrcPort = la._maxSrcPort = val.ints[0];
					MLOG(ZQ::common::Log::L_DEBUG , CLOGFMT(IpEdgePHO,"Stream IP link[%s] , get minSourcePort[%d] MaxSourcePort[%d]"),
												identStr , la._minSrcPort ,la._maxSrcPort );
					la._availSrcPort.clear ();
					la._availSrcPort.push_front (la._maxSrcPort);
				}
			}
			else
			{
				la._minSrcPort = la._maxSrcPort = -1;
				la._availSrcPort.clear ();
				MLOG(ZQ::common::Log::L_WARNING ,  CLOGFMT(IpEdgePHO ,"Stream IP link[%s] ,Can't get source port,set it to -1,that is leave it to Vstrm" ),
					identStr);
			}
		}

		Ice::Int iServiceState = TianShanIce::stNotProvisioned;
		ZQTianShan::Util::getValueMapDataWithDefault( configPD, _SERVICE_STATE_KEY_, TianShanIce::stNotProvisioned, iServiceState );


		if( iServiceState != TianShanIce::stInService )
		{
			::TianShanIce::Transport::StreamLinkPrx strmLinkPrx=_phoManager->openStreamLink(identStr);
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
						//TianShanIce::ValueMap data=ticketprx->getPrivateData();
						TianShanIce::SRM::ResourceMap dataMap=ticketprx->getResources();

						::TianShanIce::Variant	bandwidth;
						::TianShanIce::Variant	srcPort;
						try
						{
							TianShanIce::SRM::ResourceMap::const_iterator it=dataMap.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
							if(it==dataMap.end())
							{
								MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig ticket %s but no bandwidth resource"),itID->name.c_str());
							}
							else
							{	
								TianShanIce::ValueMap	PDData	=	ticketprx->getPrivateData();


								std::string ownerStreamlinkId;
								ZQTianShan::Util::getValueMapDataWithDefault(PDData,PathTicketPD_Field(ownerStreamLinkId),"",ownerStreamlinkId );
								if( ownerStreamlinkId.empty() )
								{
									MLOG(ZQ::common::Log::L_WARNING,
										CLOGFMT(NgodSop,"validate_IPStrmLinkConfig() ticket %s found, but no onwer stream link id, skip"),
										itID->name.c_str());
									continue;
								}

								bandwidth=PDField(PDData,PathTicketPD_Field(bandwidth));
								if (bandwidth.lints.size()==0) 
								{
									MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() ticket %s found a invalid bandwidth,maybe db error"),itID->name.c_str());
									continue;
								}
								MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() ticket %s used bandwidth %lld"),itID->name.c_str(),bandwidth.lints[0]);
								
								Ice::Int usedSrcPort = 0; 
								try
								{
									srcPort = PDField (PDData,PathTicketPD_Field(srcPort));
									if (srcPort.ints.size ()>0)
									{
										usedSrcPort = srcPort.ints[0];
										bool bFindAvailPort = false;
										std::list<Ice::Int>::iterator itPort = la._availSrcPort.begin ();
										for ( ; itPort != la._availSrcPort.end () ; itPort ++ )
										{
											if ( *itPort == usedSrcPort )
											{
												bFindAvailPort = true;
												break;
											}
										}
										if ( bFindAvailPort )
										{
											MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig ticket %s used srcPort[%d]"),
																itID->name.c_str(), usedSrcPort );
											la._availSrcPort.erase(itPort);
										}
										else
										{
											MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig ticket %s seemed used srcPort[%d] but can't find in the context"),
																itID->name.c_str(), usedSrcPort );
										}
									}
								}
								catch (const TianShanIce::InvalidParameter&)
								{
									MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig ticket %s no srcPort is used"),
																itID->name.c_str()  );
								}
								la._availableBandwidth	-= bandwidth.lints[0];
								la._usedBandwidth		+= bandwidth.lints[0];
								la._usedStreamCount		++;

								ResourceIPData rid;
								rid._usedBanwidth	= bandwidth.lints[0];
								rid._usedSrcPort	= usedSrcPort;
								{
									_ipResourceDataMap[itID->name] = rid;
								}

							}
						}
						catch(::TianShanIce::InvalidParameter& e)
						{
							MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() invalidParameter exception is caught:%s"),e.message.c_str());
						}
						catch (...)
						{
						}
					}
				}
			}
			
			ZQ::common::MutexGuard gd(_ipResourceLocker);				
			LinkIPAttrMap::iterator itAttr=_StreamLinkIPAttrmap.find(la._streamLinkID);

			if(itAttr==_StreamLinkIPAttrmap.end())
			{//list all ticket by link and get it		
				_StreamLinkIPAttrmap.insert(std::make_pair<std::string,LinkIPAttr>(la._streamLinkID,la));
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() insert a ip streamlink with streamlink id =%s totalBW=%lld usedBW=%lld totalStrmCount=%d usedStrmCount=%d"),
					identStr,la._totalBandwidth,la._usedBandwidth,la._totalStreamCount,la._usedStreamCount);
			}
			else
			{
				la._usedBandwidth	= itAttr->second._usedBandwidth;
				la._usedStreamCount	= itAttr->second._usedStreamCount;

				itAttr->second		= la;
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() update a ip streamlink with streamlink id =%s totalBW=%lld usedBW=%lld totalStrmCount=%d usedStrmCount=%d"),
					identStr,la._totalBandwidth,la._usedBandwidth,la._totalStreamCount,la._usedStreamCount);
			}
		}
		else
		{
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			LinkIPAttrMap::iterator itIpAttr = _StreamLinkIPAttrmap.find( identStr );
			if ( itIpAttr != _StreamLinkIPAttrmap.end() )
			{
				la._usedStreamCount		= itIpAttr->second._usedStreamCount;
				la._usedBandwidth		= itIpAttr->second._usedBandwidth;
				itIpAttr->second		= la;
			}
			else
			{
				_StreamLinkIPAttrmap[ identStr ]	= la;
			}
		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	catch (const ::Ice::Exception& e)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() catch a ice exception :%s"),e.ice_name().c_str());
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_IPStrmLinkConfig() unexpect error"));
	}
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IpEdgePHO, "leave validate a IP StreamLink's Configuration: link[%s]"), identStr);
}


void IpEdgePHO::validate_DvbcStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IpEdgePHO, "enter validate a DVBC StreamLink's Configuration: link[%s]"), identStr);

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
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgaPHO",1031,"Invalid TotalBandwidth type,should be vtLongs for streamLink[%s]" , identStr );
		}
		if (val.lints.size()==0) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgaPHO",1032,"Invalid TotalBandwidth type,no bandwidth is found for streamLink[%s]",identStr );
		}
		la._totalBandWidth=val.lints[0]*1000;
		la._availableBandwidth=la._totalBandWidth;//Available Bandwidth is useless
		la._usedBandwidth = 0;//initialize usedBandwidth as 0 
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] get totalBW[%lld]"),identStr,la._totalBandWidth);

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
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] get ProgramNumber [%d,%d] count=[%d]"),
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
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] get ProgramNumber [%d,%d] count=[%d]"),
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
							MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] with duplicate PN [%d]"),identStr,val.ints[i]);
							ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgaPHO",1036,"streamLink [%s] input has duplicate PN %d",identStr , val.ints[i]);
						}
					}
					la._backupPN.push_back(val.ints[i]);
					la._availablePN.push_back(val.ints[i]);
					MLOG(ZQ::common::Log::L_INFO , CLOGFMT(IpEdgePHO , "Stream DVBC link[%s] with input PN [%d]"),identStr , val.ints[i]);
				}
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] get ProgramNumber count=[%d]"),
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
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] get portmask [%x]"),identStr,la._portMask);
			}
			else
			{
				la._portMask = 0xFF00;
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] invalid portmask type or data,set it to [%x]"),identStr,0xFF00);
			}
		}
		catch (const TianShanIce::InvalidParameter&) 
		{
			la._portMask = 0xFF00;
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] can't get portmask,setit to [%x]"),identStr,0xFF00);
		}

		try
		{
			val = PDField(configPD,"Qam.portStep");
			if (val.type == TianShanIce::vtInts && val.ints.size() >0) 
			{
				la._portStep = val.ints[0];
				MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] get portstep [%d]"),identStr,la._portStep);
			}
			else
			{
				la._portStep = 1;
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] invalid portstep type or data,set it to [%d]"),identStr,la._portStep);
			}
		}
		catch (const TianShanIce::InvalidParameter&) 
		{
			la._portStep = 1;
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] can't get portstep,set it to [%d]"),identStr,la._portStep);
		}


		///get base port and format it
		val=PDField(configPD,"Qam.basePort");
		if(val.type!=::TianShanIce::vtInts)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgaPHO",1037,"Invalid Qam.baseport type,should be vtInts ,for streamLink[%s]",identStr);
		}
		la._basePort=val.ints[0] & (unsigned int)la._portMask;	//最后八位置0
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"Stream DVBC link [%s] get baseport[%d] and change it to [%d] with mask[%x]"),
						identStr,val.ints[0],la._basePort,la._portMask);
		
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
								MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"validate_DvbcStrmLinkConfig ticket %s but no bandwidth resource"),itID->name.c_str());
							}
							else
							{	
								TianShanIce::ValueMap val=it->second.resourceData;
								bandwidth=PDField(PDData,PathTicketPD_Field(bandwidth));
								MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"validate_DvbcStrmLinkConfig() ticket %s used bandwidth %lld"),itID->name.c_str(),bandwidth.lints[0]);
								lBandwidth = bandwidth.lints[0];
								la._availableBandwidth -= lBandwidth;								
								la._usedBandwidth += lBandwidth;
							}

							//check the program number resource
							it=dataMap.find(TianShanIce::SRM::rtMpegProgram);
							if(it==dataMap.end())
							{
								MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"validate_DvbcStrmLinkConfig() ticket %s but no porgram number resource"),itID->name.c_str());
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
									MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_DvbcStrmLinkConfig() can't find the programnumber %d in available pn list"),pn.ints[0]);
								}
								else
								{
									MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"validate_DvbcStrmLinkConfig() used porgram %d from available program number list"),pn.ints[0]);
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
							MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_DvbcStrmLinkConfig() invalidParameter exception is caught:%s"),e.message.c_str());
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
					CLOGFMT(IpEdgePHO,"insert a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedBandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
					identStr,la._totalBandWidth,la._usedBandwidth,la._totalPNCount,la._totalPNCount-(int)la._availablePN.size());
			}
			else
			{	
				itAttr->second	=	la;
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(IpEdgePHO,"update a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedbandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
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
					CLOGFMT(IpEdgePHO,"insert a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedBandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
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
					CLOGFMT(IpEdgePHO,"update a dvbc streamlink attribute with streamlink id is [%s] totalBandWidth=[%lld] usedbandwidth=[%lld] totalPn=[%d] usedPn[%d]"),
					identStr,la._totalBandWidth,la._usedBandwidth,la._totalPNCount,la._totalPNCount-(int)la._availablePN.size());

			}	
		}
	}
	catch(::TianShanIce::BaseException& e)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"tianshan expection caught :%s"),e.message.c_str());		
		e.ice_throw();
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"validate_DvbcStrmLinkConfig() unexpect error"));
	}
	
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IpEdgePHO, "leave validate a DVBC StreamLink's Configuration: link[%s]"), identStr);
}

#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
  if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
 else MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(IpEdgePHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}

Ice::Int IpEdgePHO::eval_IPStrmLink(LinkInfo& linkInfo,
									const SessCtx& sessCtx, 
									TianShanIce::ValueMap& hintPD,
									TianShanIce::SRM::ResourceMap& rcMap ,
									const ::Ice::Int oldCost)
{	
	::Ice::Int newCost = oldCost;	
	std::string sessId=sessCtx.sessId;
	
	if (!linkInfo.linkPrx)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"Session [%s] no Streamlink is attached,return with OutOfServiceCost"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IpEdgePHO,"Session [%s] enter eval_IPStreamLink() with OldCost=%d"),
								sessId.c_str(),oldCost);

	if ( newCost > ::TianShanIce::Transport::MaxCost )
	{	
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,
			"eval_IPStreamLink() Session[%s] return oldCost=%d because oldCost>=MaxCost"),sessId.c_str(),newCost);
		return newCost;
	}

	// step 1. read the link private data that need to be evaluated, please refer to 
	::Ice::Long totalBW =0;
	::Ice::Long usedBW =0;
		// step 1.2 get resource information from session
	::TianShanIce::SRM::ResourceMap resourceMap = sessCtx.resources;
	::Ice::Long bw2Alloc = 0;
	std::string destAddr, destMac;
	int		    destPort =0;

	if (0)
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(IpEdgePHO, 
			"eval_IPStreamLink() no session specified, use hinted private data only"));
	}
	else
	{	
		// try to get the bandwidth requirement from the session context
		READ_RES_FIELD(bw2Alloc, resourceMap, rtTsDownstreamBandwidth, bandwidth, lints);

		// try to get the IP requirement from the session context
		READ_RES_FIELD(destAddr, resourceMap, rtEthernetInterface, destIP, strs);
		READ_RES_FIELD(destPort, resourceMap, rtEthernetInterface, destPort, ints);

		// try to get the EthernetInterface requirement from the session context
		READ_RES_FIELD(destMac, resourceMap, rtEthernetInterface, destMac, strs);

	}

	// step 2 adjust if the hintPD also specify parameters

	// step 2.1, adjust if the hintPD also specify the bandwidth to the max of them
	if (hintPD.end() != hintPD.find(PathTicketPD_Field(bandwidth)))
	{
		::TianShanIce::Variant& var = hintPD[PathTicketPD_Field(bandwidth)];
		if (var.type == TianShanIce::vtLongs && var.lints.size() >0)
			bw2Alloc = (bw2Alloc >0) ? max(bw2Alloc, var.lints[0]) : var.lints[0];
	}

	// step 2.2, adjust if destAddr was not specified in resource but the hintPD specify it
	if (destAddr.empty() && hintPD.end() != hintPD.find( PathTicketPD_Field(destAddr) ) )
	{
		::TianShanIce::Variant& var = hintPD[ PathTicketPD_Field(destAddr) ];
		if (var.strs.size() >0)
			destAddr = var.strs[0];
	}

	// step 2.3, adjust if destMac was not specified in resource but the hintPD specify it
	if ( destMac.empty() && hintPD.end() != hintPD.find( PathTicketPD_Field(destMac) ) )
	{
		::TianShanIce::Variant& var = hintPD[PathTicketPD_Field(destMac)];
		if (var.strs.size() >0)
			destMac = var.strs[0];
	}

	// step 2.4, adjust if destPort was not specified in resource but the hintPD specify it
	if (destPort<=0 && hintPD.end() != hintPD.find(PathTicketPD_Field(destPort)))
	{
		::TianShanIce::Variant& var = hintPD[PathTicketPD_Field(destPort)];
		if (var.ints.size() >0)
			destPort = var.ints[0];
	}

	// step 2.5, double check if the bandwidth is valid
	if (bw2Alloc <= 0)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IpEdgePHO, "eval_IPStreamLink() Session[%s] 0 bandwidth has been specified, quit evaluation"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	// step 2.6, double check if the destination is valid
	if (destPort <= 0 || destAddr.empty())
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IpEdgePHO, "eval_IPStreamLink() Session[%s]  illegal destination %s:%d has been specified, quit evaluation"), sessId.c_str(),destAddr.c_str(), destPort);
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	// step 3. start evaluation.
	//         the following evaluation is only based on bandwidth for pure IP streaming
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(IpEdgePHO, "eval_IPStreamLink() Session[%s] requested allocation: %s[%s]:%d; bandwidth:%lldbps"),sessId.c_str(), destAddr.c_str(), destMac.c_str(), destPort, bw2Alloc);
	
	int	totalStrmCount = 0;
	int usedStrmCount = 0;
	std::string	strConfigedDestMac = "";
	{
		
		ZQ::common::MutexGuard gd(_ipResourceLocker);
		LinkIPAttrMap::iterator it=_StreamLinkIPAttrmap.find(linkInfo.linkIden.name);
		if(it==_StreamLinkIPAttrmap.end())
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"Session[%s] Can't find the streamlink attr through the id %s"),sessId.c_str(),linkInfo.linkIden.name.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
		totalBW = it->second._totalBandwidth;
		usedBW  = it->second._usedBandwidth;
		if( bw2Alloc > (totalBW-usedBW) )
		{
			MLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(IpEdgePHO,"Session[%s] no enought bandwidth Required bw=%lld and used BW=%lld totalBW=%lld"),
					sessId.c_str(),bw2Alloc,usedBW,totalBW);
			return TianShanIce::Transport::OutOfServiceCost;
		}
		usedStrmCount = it->second._usedStreamCount;
		totalStrmCount = it->second._totalStreamCount;
		if ( totalStrmCount <= usedStrmCount ) 
		{
			MLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(IpEdgePHO,"Session[%s] no enought bandwidth Required TotalStrmCount[%d] usedStrmCount[%d],no available strmCount "),
					sessId.c_str(), totalStrmCount ,usedStrmCount);
			return TianShanIce::Transport::OutOfServiceCost;
		}
		
		strConfigedDestMac = it->second._destMacAddress;

		// step 3.1. find all the allocation by StreamLinks
		MLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT(IpEdgePHO,"Session[%s] current available bandwidth is %lld and needed bandwidth is %lld"),
			sessId.c_str(),totalBW-usedBW,bw2Alloc);
		//it->second._availableBandwidth -= bw2Alloc;
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

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"Session[%s] bandwidth cost[%d] with usedBandwidth[%lld] totalBandwidth[%lld]"),
								sessId.c_str(),newCost,usedBW,totalBW);

	int strmCountCost = usedStrmCount*::TianShanIce::Transport::MaxCost / totalStrmCount;

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"Session[%s] streamcount cost[%d] with usedStreamCount[%d] totalStreamCount[%d]"),
								sessId.c_str(),strmCountCost,usedStrmCount,totalStrmCount);

	newCost = newCost > strmCountCost ? strmCountCost : newCost;
	// end of the evaluation
	newCost = max(oldCost, newCost);

	if (newCost <= ::TianShanIce::Transport::MaxCost)
	{
		bool	bNATPenetrating =false;
		//Check if there is NAT 
		TianShanIce::SRM::ResourceMap::const_iterator itEthernet = resourceMap.find (TianShanIce::SRM::rtEthernetInterface);
		if ( itEthernet != resourceMap.end())
		{
			const ::TianShanIce::SRM::Resource& resEthernet = itEthernet->second;			
			TianShanIce::ValueMap::const_iterator itNatData = resEthernet.resourceData.find ("natPenetrating");
			if (itNatData != resEthernet.resourceData.end ())
			{//get NAT Penetrating data
				const TianShanIce::Variant varPenetrating = itNatData->second;
				if ( varPenetrating.type == TianShanIce::vtInts && varPenetrating.ints.size() > 0)
				{
					if ( varPenetrating.ints[0] != 0)
					{
						bNATPenetrating = true;
					}
				}
			}			
		}
		if (bNATPenetrating)
		{
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			LinkIPAttrMap::const_iterator it=_StreamLinkIPAttrmap.find(linkInfo.linkIden.name);
			const LinkIPAttr& ipAttr = it->second;
			if (ipAttr._srcStreamIP.empty ())
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"Session[%s] NATPenetrating  Enabled but no Available Stream Src IP,return with [OutOfServiceCost]"),
					sessId.c_str() );
				return 	::TianShanIce::Transport::OutOfServiceCost;
			}
			if ( ipAttr._availSrcPort.size () <= 0)
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"Session[%s] NATPenetrating  Enabled but no Available Stream Src Port,return with [OutOfServiceCost]"),
										sessId.c_str()  );
				return 	::TianShanIce::Transport::OutOfServiceCost;
			}
			else
			{				
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"Session[%s] NATPenetrating Enabled and src port is OK"), sessId.c_str());
			}
		}

		// sounds like a potential allocation, fill in the hintedPD with link information
		try
		{
			//fill useful resource data into rcMap 
			::TianShanIce::SRM::Resource res;
			res.attr	=	TianShanIce::SRM::raMandatoryNonNegotiable;
			res.status	=	TianShanIce::SRM::rsRequested;

			::TianShanIce::Variant value;
			value.bRange = false;

			//////////////////////////////////////////////////////////////////////////
			//set NATPenetrating
			if ( bNATPenetrating )
			{
				value.type = TianShanIce::vtInts;
				value.bRange = false;
				value.ints.clear ();
				value.ints.push_back (1);
				hintPD[PathTicketPD_Field(NATPenetrating)] = value;
			}
			
			//////////////////////////////////////////////////////////////////////////			
			// a. bandwidth
			res.resourceData.clear();
			value.type = TianShanIce::vtLongs;
			value.lints.clear();
			value.lints.push_back(bw2Alloc);
			MLOG(ZQ::common::Log::L_DEBUG,
					CLOGFMT(IpEdgePHO,"eval_IPStrmLink() Session[%s] set bandwidth to %lld"),
					sessId.c_str(),bw2Alloc);
			hintPD[PathTicketPD_Field(bandwidth)] = value;
			
			
			res.resourceData["bandwidth"]=value;
			res.status=TianShanIce::SRM::rsAssigned;
			rcMap[TianShanIce::SRM::rtTsDownstreamBandwidth]=res;
			

			//////////////////////////////////////////////////////////////////////////
			// b. dest ip and port
			res.resourceData.clear();
			value.type = TianShanIce::vtInts;
			value.ints.clear();
			value.ints.push_back(destPort);
			MLOG(ZQ::common::Log::L_DEBUG,
				CLOGFMT(IpEdgePHO,"eval_IPStrmLink() Session[%s] set destPort to %d"),
				sessId.c_str(),destPort);
			hintPD[PathTicketPD_Field(destPort)] = value;
			res.resourceData["destPort"]=value;


			value.type = TianShanIce::vtStrings;
			value.strs.clear();
			value.strs.push_back(destAddr);
			MLOG(ZQ::common::Log::L_DEBUG,
				CLOGFMT(IpEdgePHO,"eval_IPStrmLink() Session[%s] set destAddr to %s"),
				sessId.c_str(),destAddr.c_str());
			hintPD[PathTicketPD_Field(destAddr)] = value;
			res.resourceData["destIP"]=value;
			
		


			//////////////////////////////////////////////////////////////////////////
			// d. dest mac address
			if ( !strConfigedDestMac.empty() ) 
			{//configured destMac is not empty
				value.type = TianShanIce::vtStrings;
				value.strs.clear();
				value.strs.push_back(strConfigedDestMac);
				MLOG(ZQ::common::Log::L_DEBUG,
					CLOGFMT(IpEdgePHO,"eval_IPStrmLink() Session[%s] set destMac to configuration destMac [%s] "),
					sessId.c_str(), strConfigedDestMac.c_str() );

				hintPD[PathTicketPD_Field(destMac)] = value;
				res.resourceData["destMac"]=value;	
			}
			else if (!destMac.empty())
			{
				value.type = TianShanIce::vtStrings;
				value.strs.clear();
				value.strs.push_back(destMac);
				MLOG(ZQ::common::Log::L_DEBUG,
					CLOGFMT(IpEdgePHO,"eval_IPStrmLink() Session[%s] set destMac to %s"),
					sessId.c_str(),destMac.c_str());
				hintPD[PathTicketPD_Field(destMac)] = value;
				res.resourceData["destMac"]=value;				
			}
			else
			{
				MLOG(ZQ::common::Log::L_DEBUG,
						CLOGFMT(IpEdgePHO,"eval_IPStrmLink() Session[%s] no destMac"),
						sessId.c_str() );
			}
			
			res.status=TianShanIce::SRM::rsAssigned;
			rcMap[TianShanIce::SRM::rtEthernetInterface]=res;			

		}
		catch(...)
		{
			MLOG(ZQ::common::Log::L_ERROR, 
				CLOGFMT(IpEdgePHO, "eval_IPStreamLink() Session[%s] exception caught while fillin the ticket private data"),
				sessId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	MLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT(IpEdgePHO,"eval_IPStreamLink() Session[%s] return with newCost=%d"),
			sessId.c_str(),max(oldCost,newCost));
	// step 4. return the higher as the cost
	return max(oldCost, newCost);
}

Ice::Int	IpEdgePHO::eval_IPShareStrmLink(	LinkInfo& linkInfo, 
												 const SessCtx& sessCtx, 
												 TianShanIce::ValueMap& hintPD, 
												 TianShanIce::SRM::ResourceMap& rcMap ,
												 const ::Ice::Int oldCost)
{
	MLOG(ZQ::common::Log::L_DEBUG , CLOGFMT(IpEdgePHO,"eval_IPShareStrmLink() using share link with LinkId[%s]"),
		linkInfo.linkIden.name.c_str());
	std::string strLinkId = "";
	{
		ZQ::common::MutexGuard gd(_sharedIPLinkAttrLocker);
		LinkIPShareAttrMap::const_iterator it = _sharedIPLinkAttrmap.find( linkInfo.linkIden.name );
		if( it != _sharedIPLinkAttrmap.end( ) )
		{
			strLinkId = it->second._streamLinkId;
			MLOG(ZQ::common::Log::L_INFO , CLOGFMT(IpEdgePHO , "eval_IPShareStrmLink() find target linkId[%s]" ),
					strLinkId.c_str() );
		}
		else
		{
			MLOG(ZQ::common::Log::L_ERROR , 
				CLOGFMT(IpEdgePHO , "eval_IPShareStrmLink() no shared IP link with ID [%s] ,return with OutOfServiceCost"),
				linkInfo.linkIden.name.c_str() );
			return TianShanIce::Transport::OutOfServiceCost;
		}
	}
	LinkInfo lInfo = linkInfo;
	{
		ZQ::common::MutexGuard lockIpRes(_ipResourceLocker);
		LinkIPAttrMap::const_iterator itIP = _StreamLinkIPAttrmap.find(strLinkId);
		if( itIP == _StreamLinkIPAttrmap.end() )
		{
			MLOG(ZQ::common::Log::L_ERROR, 
				CLOGFMT(IpEdgePHO , "eval_IPShareStrmLink() target streamlink [%s] is not found" ),
				strLinkId.c_str() );
			return 	::TianShanIce::Transport::OutOfServiceCost;
		}
		try
		{
			lInfo.linkIden.name = strLinkId;
			lInfo.linkPrx = _helperMgr.openStreamLink( strLinkId.c_str() );
		}
		catch (const Ice::Exception& ex) 
		{
			MLOG(ZQ::common::Log::L_ERROR , 
				CLOGFMT(IpEdgePHO , "eval_IPShareStrmLink() catch ice exception [%s] when open streamLink [%s]"),
				ex.ice_name().c_str() , strLinkId.c_str() );
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
		catch (...) 
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IpEdgePHO,"eval_IPShareStrmLink() catch an unknown exception when open streamLink[%s]"),
				strLinkId.c_str() );
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	Ice::Int retCost = eval_IPStrmLink(lInfo,sessCtx,hintPD,rcMap,oldCost);
	return retCost;		
}

Ice::Int IpEdgePHO::eval_DvbcShareStrmLink(LinkInfo& linkInfo, 
										   const SessCtx& sessCtx, 
										   TianShanIce::ValueMap& hintPD,
										   TianShanIce::SRM::ResourceMap& rcMap ,
										   const ::Ice::Int oldCost)
{
	MLOG(ZQ::common::Log::L_DEBUG,
		CLOGFMT(IpEdgePHO,"eval_DvbcShareStrmLink() using shared link with linkID[%s]"),
		linkInfo.linkIden.name.c_str());
	std::string strLinkId = "";
	{
		ZQ::common::MutexGuard gd(_sharedDVBCLinkAttrLocker);
		//_sharedDVBCLinkAttrmap
		LinkDVBCShareAttrMap::const_iterator it = _sharedDVBCLinkAttrmap.find( linkInfo.linkIden.name );
		if ( it != _sharedDVBCLinkAttrmap.end() ) 
		{
			strLinkId = it->second._streamLinkId;
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO , "eval_DvbcShareStrmLink() find the target linkID [%s]" ),
				strLinkId.c_str() );
		}
		else
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO , "eval_DvbcShareStrmLink() no shared dvbc link with ID [%s] ,return with OutOfServiceCost" ),
				linkInfo.linkIden.name.c_str() );
			return 	::TianShanIce::Transport::OutOfServiceCost;			 
		}
	}

	LinkInfo lInfo = linkInfo;
	{
		ZQ::common::MutexGuard lockDVBCres(_dvbcResourceLocker);
		LinkDVBCAttrMap::const_iterator itDvbc = _StreamLinkDVBCAttrmap.find( strLinkId );
		if (itDvbc == _StreamLinkDVBCAttrmap.end( ) ) 
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IpEdgePHO , "eval_DvbcShareStrmLink() target streamlink [%s] is not found" ),
				strLinkId.c_str());
			return 	::TianShanIce::Transport::OutOfServiceCost;
		}	
		try
		{			
			lInfo.linkIden.name = strLinkId ;
			lInfo.linkPrx = _helperMgr.openStreamLink(strLinkId.c_str() );		
		}
		catch ( const Ice::Exception& ex )
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IpEdgePHO , "eval_DvbcShareStrmLink() catch ice exception [%s] when open streamlink [%s]"),
				ex.ice_name().c_str() , strLinkId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
		catch (...)
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IpEdgePHO , "eval_DvbcShareStrmLink() catch unknown exception when open streamlink [%s]"),
				strLinkId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	Ice::Int retCost =  eval_DvbcStrmLink(lInfo , sessCtx , hintPD , rcMap , oldCost );
	return retCost;
}

int IpEdgePHO::evalDVBCStreamLinkCost(const std::string& streamLinkID,Ice::Long bw2Alloc,const std::string& sessId)
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
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,
			"evalDVBCStreamLinkCost() Session[%s] streamlink [%s] return with OutOfServiceCost: pnCount=[%d] and UsedPnCount=[%d] "
			"totalBW=[%lld] UsedBW=[%lld] ,pnCost=[%d]"),
			sessId.c_str(),streamLinkID.c_str(),
			it->second._totalPNCount ,it->second._totalPNCount- (int)it->second._availablePN.size(),
			it->second._totalBandWidth,it->second._usedBandwidth,pnCost);
		return TianShanIce::Transport::OutOfServiceCost;
	}
	
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,
				"evalDVBCStreamLinkCost() Session[%s] streamlink [%s] has pnCount=[%d] and UsedPnCount=[%d] "
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
			CLOGFMT(IpEdgePHO,"evalDVBCStreamLinkCost() Session[%s] streamlink [%s] allocbandwidth [%lld] "
					"totalBW=[%lld] UsedBW=[%lld] ,bwCost=[%d]"),
					sessId.c_str(),streamLinkID.c_str(),
					bw2Alloc,it->second._totalBandWidth,
					it->second._usedBandwidth,bandwidthCost);
	}
	int returnCost=max(pnCost,bandwidthCost);
	MLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(IpEdgePHO,"evalDVBCStreamLinkCost() Session[%s] streamlink [%s] "
				" return cost=[%d] (pnCost=[%d],BWCost=[%d]) "),
		sessId.c_str(),streamLinkID.c_str(),returnCost,pnCost,bandwidthCost);

	return returnCost;
}

Ice::Int IpEdgePHO::eval_DvbcStrmLink(LinkInfo& linkInfo,
									  const SessCtx& sessCtx,
									  TianShanIce::ValueMap& hintPD,
									  TianShanIce::SRM::ResourceMap& rcMap ,
									  const ::Ice::Int oldCost)
{
	int	newCost = oldCost;
	std::string sessId=sessCtx.sessId;

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IpEdgePHO, "[Session[%s] enter eval_DvbcStrmLink() with oldCost=%d"),sessId.c_str(),oldCost);

	if (oldCost > ::TianShanIce::Transport::MaxCost)
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"Session[%s] oldCost cost is bigger than MaxCost,return with OldCost=%d"),sessId.c_str(),oldCost);
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
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"Session[%s] ice exception is caught:%s"),sessId.c_str(),e.ice_name().c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"Session[%s] unexpect error is threw out when call streamlink's getPrivateData"),sessId.c_str());
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
					MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(IpEdgePHO, "eval_DvbcStrmLink() unacceptable rtTsDownstreamBandwidth in session: bandwidth(range=%d, size=%d)"),
									tsDsBw.resourceData["bandwidth"].bRange,
									tsDsBw.resourceData["bandwidth"].lints.size());
				}
			}
			else
			{//error if no bandwidth parameter???
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"eval_DvbcStrmLink() Session[%s] unacceptable resource without bandwidth"),sessId.c_str());
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>( MLOG, EXPFMT("IpEdgePHO",1041,"eval_DvbcStrmLink() no 'bandwidth' is found in resoucemap"));
			}
		}
		catch (...)
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IpEdgePHO, "eval_DvbcStrmLink() Session[%s] can not query the given session for resource info, stop evaluation"),sessId.c_str());
			return ::TianShanIce::Transport::OutOfServiceCost;
		}
	}
	else
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(IpEdgePHO, "eval_DvbcStrmLink() Session[%s] no session specified, use hinted private data only"),sessId.c_str());

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
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IpEdgePHO, "eval_DvbcStrmLink() Session[%s] 0 bandwidth has been specified, quit evaluation"),sessId.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IpEdgePHO, "eval_DvbcStrmLink() Session[%s] required bandwidth:%lldbps"),sessId.c_str(), bw2Alloc);

// 	TianShanIce::Variant valPN;
// 	valPN.bRange=false;

	TianShanIce::Variant valBandwidth;
	valBandwidth.lints.clear();
	valBandwidth.lints.push_back(bw2Alloc);
	valBandwidth.type=TianShanIce::vtLongs;
	valBandwidth.bRange=false;
	try
	{
		newCost=evalDVBCStreamLinkCost(linkInfo.linkIden.name , bw2Alloc , sessId);
		if(newCost > ::TianShanIce::Transport::MaxCost)
		{
			MLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(IpEdgePHO,"eval_DvbcStrmLink() Session[%s] streamLink=%s return with outofServiceCost with cost=%d"),
					sessId.c_str(),linkInfo.linkIden.name.c_str(),newCost);
			return newCost;
		}

	}
	catch (std::string& str)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"%s"),str.c_str());
		return ::TianShanIce::Transport::OutOfServiceCost;
	}
	catch(...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"Session[%s] unexpect error when eval dvbc streamlink cost"),sessId.c_str());
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
					CLOGFMT(IpEdgePHO,"Session[%s] StreamLink=%s return with cost=%d"),
					sessId.c_str(),linkInfo.linkIden.name.c_str(),max(oldCost,newCost));
	return max(oldCost,newCost);
}

IPathHelperObject::NarrowResult IpEdgePHO::doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, 
													const SessCtx& sessCtx)
{
	if (!ticket)
		return NR_Unrecognized;

	std::string type;
	::TianShanIce::Transport::StreamLinkPrx strmLink;
	try 
	{
		strmLink = _helperMgr.openStreamLink(ticket->streamLinkIden);
		type = strmLink->getType();
	}
	catch(...) {}
	if ( 0 == type.compare( STRMLINK_TYPE_IPEDGE_IP ) )
	{
		return narrow_IPStrmLink( strmLink , sessCtx , ticket );
	}
	else if (0 == type.compare( STRMLINK_TYPE_IPEDGE_DVBC ))
	{
		return narrow_DvbcStrmLink( strmLink , sessCtx ,  ticket );
	}
	else if ( 0 == type.compare( STRMLINK_TYPE_IPEDGE_DVBCSHARELINK) ) 
	{
		return narrow_DvbcSharedStrmLink( strmLink , sessCtx , ticket );
	}
	else if ( 0 == type.compare(STRMLINK_TYPE_IPEDGE_IPSHARELINK) )
	{
		return narrow_IPSharedStrmLink( strmLink , sessCtx , ticket );
	}
	else return IPathHelperObject::NR_Unrecognized;
}

IPathHelperObject::NarrowResult IpEdgePHO::narrow_IPStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink,
															const SessCtx& sessCtx,
															 const TianShanIce::Transport::PathTicketPtr&  ticket)
{
	std::string	strStrmLinkID;
	try
	{
		strStrmLinkID=strmLink->getIdent().name;
	}
	catch (const Ice::Exception& ex) 
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() caught %s when get streamlink's identity"), ex.ice_name().c_str() );
		return NR_Error;
	}
	catch (...) 
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() get streamlink's ID failed"));
		return NR_Error;
	}
	return inner_narrow_IPStrmLink( strStrmLinkID , sessCtx , ticket );
}

IPathHelperObject::NarrowResult  IpEdgePHO::inner_narrow_IPStrmLink( const std::string strmLinkId , 
																		const SessCtx& sessCtx, 
																		const TianShanIce::Transport::PathTicketPtr& ticket)
{
	std::string sessId = sessCtx.sessId;
	
	std::string	ticketID = ticket->ident.name;

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IpEdgePHO,"Session[%s] narrowed ticket with ticketid=%s"),
						sessId.c_str(),ticketID.c_str());
	{
		//hintPD[PathTicketPD_Field(NATPenetrating)] = value;
		bool bEnableNATPenetrating = false;
		std::string			strDestMac ="";
		const TianShanIce::ValueMap& priData = ticket->privateData;
		TianShanIce::ValueMap::const_iterator itMap = priData.find (PathTicketPD_Field(NATPenetrating));
		if ( itMap != priData.end () )
		{
			const TianShanIce::Variant varNat = itMap->second;
			if( (varNat.type == TianShanIce::vtInts) && 
				(varNat.bRange == false) && 
				(varNat.ints.size () > 0) &&
				(varNat.ints[0] == 1) )
			{
				bEnableNATPenetrating = true;
			}
		}

		char szNatSessID[11] = {0};

		if (bEnableNATPenetrating)
		{//find the mac
			itMap = priData.find (PathTicketPD_Field(destMac));
			if ( itMap != priData.end () )
			{
				const TianShanIce::Variant varDestMac = itMap->second;
				if ( varDestMac.type == TianShanIce::vtStrings &&
					  varDestMac.strs.size () >  0   ) 
				{
					strDestMac = varDestMac.strs[0];
				}
			}

			static bool bInitRamdom =false;
			if (!bInitRamdom) 
			{
				srand (time(NULL));
				bInitRamdom = true;
			}
			
			if (strDestMac.length () != 6) 
			{
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IpEdgePHO ,
						"narrow_IPStrmLink Session[%s] NATPenetrating Enabled and DestMac detected,but invalid destMac format [%s]"),
						sessId.c_str(),strDestMac.c_str() );

				time_t now = time(0);
				struct tm __t;
#ifdef ZQ_OS_MSWIN
				_gmtime64_s(&__t, &now);
#else
				gmtime_r(&now, &__t);	
#endif
				uint id = (uint)__t.tm_hour<< 28 | 
						  (uint)__t.tm_min << 22 | 
						  (uint)__t.tm_sec << 16;
				static int sLastLWord =0;				
				id = id | (__t.tm_mday<<14 )| (sLastLWord++);
				sprintf(szNatSessID,"%010u",id);
				MLOG(ZQ::common::Log::L_INFO,
						CLOGFMT(IpEdgePHO,"narrow_IPStrmLink Session[%s] Generating a Poke Hole Session ID[%s]"),
						sessId.c_str(), szNatSessID );
			}
			else
			{
				sprintf ( szNatSessID , "%s%04d" , strDestMac.c_str () ,rand() % 10000 );
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"narrow_IPStrmLink Session[%s] Generating a Poke Hole Session ID[%s] using MAC[%s]"),
							sessId.c_str(), szNatSessID , strDestMac.c_str () );
			}
		}
		ResourceIPData rid;
		rid._usedSrcPort = 0;
		TianShanIce::SRM::ResourceMap& resMap=ticket->resources;
		Ice::Long	bw2Alloc = 0;
		Ice::Long	bwTotal = 0;
		Ice::Long	bwUsed = 0;
		Ice::Int	iTotalSrcPort = 0;
		Ice::Int	iUsedPort = 0;
		READ_RES_FIELD(bw2Alloc, resMap, rtTsDownstreamBandwidth, bandwidth, lints);
		std::string	strLinkIdent = strmLinkId ; //strmLink->getIdent().name;
		{
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			
			LinkIPAttrMap::iterator itIPmap = _StreamLinkIPAttrmap.find(strLinkIdent);
			if ( itIPmap == _StreamLinkIPAttrmap.end())
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() Session[%s] No streamlink with ID[%s] is found"),
					sessId.c_str(),strLinkIdent.c_str());
				return NR_Error;
			}
			LinkIPAttr& ipAttr = itIPmap->second;
			if ( ipAttr._usedStreamCount >= ipAttr._totalStreamCount) 
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() Session[%s] not enough streamcount,totalStrmCount[%d] usedStrmCount[%d] for strmLink[%s]"),
					sessId.c_str() , ipAttr._totalStreamCount,ipAttr._usedStreamCount,strLinkIdent.c_str()	);
				return NR_Error;
			}			
			
			if (bEnableNATPenetrating ) 
			{		
				if ( ipAttr._availSrcPort.size () <= 0 ) 
				{
					MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() Session[%s] Enabled NATPenetrating but no available source stream port"), sessId.c_str());
					return NR_Error;
				}
			}	
			
			if ( ipAttr._usedBandwidth + bw2Alloc > ipAttr._totalBandwidth) 
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() Session[%s] not enough bandwidth,totalBandwidth[%d] usedBandwidth[%d] for strmLink[%s]"),
					sessId.c_str() , ipAttr._totalBandwidth ,
					ipAttr._usedBandwidth , strLinkIdent.c_str()	);
				return NR_Error;
			}
			bwTotal = ipAttr._totalBandwidth;
			rid._usedBanwidth=bw2Alloc;
			
			if (bEnableNATPenetrating) 
			{
				int iSrcPortOffset = 0;
				if ( ipAttr._availSrcPort.size () > 2 ) 
				{
					iSrcPortOffset = rand() % (ipAttr._availSrcPort.size ()-1);
					iSrcPortOffset = iSrcPortOffset > 5? 5 : iSrcPortOffset;
				}
				std::list<Ice::Int>::iterator itSrcPort = ipAttr._availSrcPort.begin ();
				for ( int iSrcPortStep = 0 ; iSrcPortStep < 5; iSrcPortStep++ ) 
					itSrcPort++;
				
				rid._usedSrcPort = *itSrcPort;

				ipAttr._availSrcPort.erase (itSrcPort);			
				iTotalSrcPort = ipAttr._maxSrcPort - ipAttr._minSrcPort + 1;
				iUsedPort = iTotalSrcPort - ipAttr._availSrcPort.size ();
			}
			ipAttr._usedStreamCount ++ ;		
			ipAttr._usedBandwidth += bw2Alloc;
			bwUsed = ipAttr._usedBandwidth;
		}

		if (bEnableNATPenetrating) 
		{			
			//put the source port into ticket resource	
			TianShanIce::Variant varPokeHoleID;
			varPokeHoleID.type = TianShanIce::vtStrings;
			varPokeHoleID.bRange = false;
			varPokeHoleID.strs.clear ();
			varPokeHoleID.strs.push_back (szNatSessID);
			ticket->privateData[PathTicketPD_Field(SrcPort)] = varPokeHoleID;
			PutResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"pokeholeSession",varPokeHoleID,sessId);
			MLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() Session[%s] narrowed with Poke hole SessionID[%s]"),sessId.c_str() , szNatSessID);
		}
		
		TianShanIce::Variant varLinkType;
		TianShanIce::Variant varLinkId;
		
		varLinkType.bRange = false;
		varLinkType.type = TianShanIce::vtStrings;
		varLinkType.strs.clear();
		varLinkType.strs.push_back( STRMLINK_TYPE_IPEDGE_IP );
		ticket->privateData[PathTicketPD_Field(ownerStreamLinkType)] = varLinkType;
		
		varLinkId.bRange = false;
		varLinkId.type = TianShanIce::vtStrings;
		varLinkId.strs.clear();
		varLinkId.strs.push_back( strLinkIdent );
		ticket->privateData[PathTicketPD_Field(ownerStreamLinkId)] = varLinkId;
		
		{
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			_ipResourceDataMap[ticketID] = rid;			
		}
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"narrow_IPStrmLink() Session[%s] narrowed with NeedBW[%lld]"
				"  and UsedBW[%lld] TotalBW[%lld] TotalAvailSrcPort[%d] usedSrcPort[%d]  EnableNAT[%d]"),
				sessId.c_str() , bw2Alloc , bwUsed , bwTotal , iTotalSrcPort , iUsedPort , bEnableNATPenetrating	);
		
	}
	return NR_Narrowed;		
}

IPathHelperObject::NarrowResult IpEdgePHO::narrow_IPSharedStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink,
																   const SessCtx& sessCtx, 
																   const TianShanIce::Transport::PathTicketPtr& ticket)
{
	try
	{
		std::string	strLinkType = strmLink->getType( );
		if ( strcmp( strLinkType.c_str() , STRMLINK_TYPE_IPEDGE_IPSHARELINK) != 0 ) 
		{
			MLOG(ZQ::common::Log::L_ERROR, 
				CLOGFMT( IpEdgePHO , "narrow_IPSharedStrmLink() the strmLink type is[%s] and does not match [%s]" ),
				strLinkType.c_str(),
				STRMLINK_TYPE_IPEDGE_IPSHARELINK);
			return NR_Error;
		}
		std::string strLinkId = strmLink->getIdent().name;
		std::string strTragetLinkId = "";
		
		{
			ZQ::common::MutexGuard gd(_sharedIPLinkAttrLocker);
			LinkIPShareAttrMap::const_iterator it = _sharedIPLinkAttrmap.find(strLinkId);
			if ( it == _sharedIPLinkAttrmap.end() )
			{
				MLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(IpEdgePHO,"narrow_IPSharedStrmLink() can't find target linkId through link[%s]"),
					strLinkId.c_str() );
				return NR_Error;					 
			}
			strTragetLinkId = it->second._streamLinkId;
		}
		MLOG(ZQ::common::Log::L_INFO , 
			CLOGFMT(IpEdgePHO , "narrow_IPSharedStrmLink() find a target link[%s] with linkId[%s]"),
			strTragetLinkId.c_str() , strLinkId.c_str()	);
		return inner_narrow_IPStrmLink( strTragetLinkId , sessCtx , ticket );
	}
	catch ( const Ice::Exception& ex ) 
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO , "narrow_IPSharedStrmLink() catch ice exception:%s" ),ex.ice_name().c_str());
		return NR_Error;
	}
	catch (... ) 
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO , "narrow_IPSharedStrmLink() catch unknown exception " ) );
		return NR_Error;
	}
}

IPathHelperObject::NarrowResult IpEdgePHO::narrow_DvbcSharedStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
														const SessCtx& sessCtx,
														const TianShanIce::Transport::PathTicketPtr&  ticket)
{
	try
	{
		std::string strLinkType = strmLink->getType();
		if (strcmp( strLinkType.c_str() , STRMLINK_TYPE_IPEDGE_DVBCSHARELINK ) !=0 ) 
		{
			MLOG(ZQ::common::Log::L_ERROR,
						CLOGFMT(IpEdgePHO,"narrow_DvbcSharedStrmLink() the streamLink type is [%s] and does not match [%s]"),
						strLinkType.c_str()	,
						STRMLINK_TYPE_IPEDGE_DVBCSHARELINK );
			return NR_Error;
		}
		std::string strLinkId = strmLink->getIdent().name;
		std::string strTargetLinkId = "";
		{
			ZQ::common::MutexGuard gd( _sharedDVBCLinkAttrLocker );
			LinkDVBCShareAttrMap::const_iterator it = _sharedDVBCLinkAttrmap.find(strLinkId);
			if ( it == _sharedDVBCLinkAttrmap.end()) 
			{
				MLOG(ZQ::common::Log::L_ERROR,
						CLOGFMT(IpEdgePHO , "narrow_DvbcSharedStrmLink() Can't find target linkId through link[%s]" ),
						strLinkId.c_str());
				return NR_Error;					 
			}
			strTargetLinkId = it->second._streamLinkId;			
		}
		MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IpEdgePHO , "narrow_DvbcSharedStrmLink() find a target link[%s] with LinkId[%s]"),
				strTargetLinkId.c_str() , strLinkId.c_str());
		return inner_narrow_DvbcStrmLink(strTargetLinkId,sessCtx,ticket);
	}
	catch ( const Ice::Exception& ex ) 
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO , "narrow_DvbcSharedStrmLink() catch ice exception:%s" ),ex.ice_name().c_str());
		return NR_Error;
	}
	catch (... ) 
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO , "narrow_DvbcSharedStrmLink() catch unknown exception " ) );
		return NR_Error;
	}
}

IPathHelperObject::NarrowResult IpEdgePHO::inner_narrow_DvbcStrmLink(const std::string strStrmLinkID , 
											const SessCtx& sessCtx,
											const TianShanIce::Transport::PathTicketPtr&  ticket)
{
	//如何narrow呢?
	//step 1.find a unused program number
	int pn=0;
	int port=0;
	TianShanIce::SRM::ResourceMap::iterator itResMap=ticket->resources.find(TianShanIce::SRM::rtEthernetInterface);
	std::string sessId= sessCtx.sessId;

	if(itResMap==ticket->resources.end())
	{
		MLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(IpEdgePHO,"narrow_DvbcStrmLink() Session[%s] can't find "
					"rtEthernetInterface from ticket resources"),
					sessId.c_str());
		return 	NR_Error;		 
	}
	std::string&	strTicketID = ticket->ident.name;
	
	TianShanIce::SRM::ResourceMap& resMap = ticket->resources;
	int AvailPnCount=0;
	Ice::Long  bw2Alloc;
	{
		ZQ::common::MutexGuard gd(_dvbcResourceLocker);	
		READ_RES_FIELD(bw2Alloc, resMap, rtTsDownstreamBandwidth, bandwidth, lints);
		LinkDVBCAttrMap::iterator it=_StreamLinkDVBCAttrmap.find(  strStrmLinkID );
		if(it==_StreamLinkDVBCAttrmap.end())
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IpEdgePHO,"narrow_DvbcStrmLink() Session[%s] can't find the streamlink attribute through streamlink[%s] for ticket[%s]"),
				sessId.c_str(),strStrmLinkID.c_str(),strTicketID.c_str() );
			return NR_Error;
		}
		if(it->second._availablePN.size()<=0)
		{
			MLOG(ZQ::common::Log::L_ERROR,
							CLOGFMT(IpEdgePHO,"Session[%s] streamlink[%s] not enough available program number,totalPN[%d] usedPN[%d] for ticket[%s]"),
							sessId.c_str(),strStrmLinkID.c_str(),it->second._totalPNCount,it->second._totalPNCount-it->second._availablePN.size(), strTicketID.c_str() );
			return NR_Error;				 
		}
		if (it->second._totalBandWidth <= it->second._usedBandwidth) 
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IpEdgePHO,"narrow_DvbcStrmLink() Session[%s] strmLink[%s] no available Badnwidth,totalBW[%lld] usedBW[%lld] for ticket[%s]"),
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
		MLOG(ZQ::common::Log::L_INFO,
							CLOGFMT(IpEdgePHO,"Session[%s] streamlink=[%s] ticketID=[%s] narrowed with BWAlloc=[%lld] PN=[%d]"
							" Now totalBW=[%lld] usedBW=[%lld] totalPn=[%d] usedPn=[%d]"),
							sessId.c_str(),strStrmLinkID.c_str(),strTicketID.c_str(),
							bw2Alloc,pn,it->second._totalBandWidth , it->second._usedBandwidth,
							it->second._totalPNCount,it->second._totalPNCount-(int)it->second._availablePN.size() );
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
	varLinkType.strs.push_back( STRMLINK_TYPE_IPEDGE_DVBC );
	ticket->privateData[PathTicketPD_Field(ownerStreamLinkType)] = varLinkType;

	varLinkId.bRange = false;
	varLinkId.type = TianShanIce::vtStrings;
	varLinkId.strs.clear();
	varLinkId.strs.push_back( strStrmLinkID );
	ticket->privateData[PathTicketPD_Field(ownerStreamLinkId)] = varLinkId;


	{
		ZQ::common::MutexGuard gd(_dvbcResourceLocker);
		ResourceDVBCData rdd;
		rdd._usedBandWidth = bw2Alloc;
		rdd._usedPN = pn;
		_dvbcResourceDataMap[strTicketID] = rdd;
		
	}

	MLOG(ZQ::common::Log::L_INFO,
			CLOGFMT(IpEdgePHO,"narrow_DvbcStrmLink() Session[%s] strmLink[%s] get dvbcresource with"
			" pn=[%d] port=[%d] bw=[%lld] with ticketID=[%s]"),
			sessId.c_str(),strStrmLinkID.c_str(), pn,port,bw2Alloc ,strTicketID.c_str());

	return NR_Narrowed;
}

IPathHelperObject::NarrowResult IpEdgePHO::narrow_DvbcStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink,
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
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"narrow_DvbcStrmLink() get streamlink's ID failed"));
		return NR_Error;
	}
	return inner_narrow_DvbcStrmLink(strStrmLinkID , sessCtx , ticket );
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
void IpEdgePHO::doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket)
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
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"no ticket owner link type is found for ticket[%s], this ticket may not be narrowed") , ticketID.name.c_str() );
			return;
		}
		std::string strLinkType = itLinkType->second.strs[0];
		TianShanIce::ValueMap::const_iterator itLinkId = ticketPD.find( PathTicketPD_Field(ownerStreamLinkId) );
		if ( itLinkId ==ticketPD.end() || itLinkType->second.type != TianShanIce::vtStrings || itLinkId->second.strs.size() ==0 ) 
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() can' get streamlink from ticket[%s], this ticket may not be narrowed"),ticketID.name.c_str() );
			return ;
		}
		std::string strStramlinkID = itLinkId->second.strs[0];
		

		//hack for STRMLINK_TYPE_IPEDGE_DVBCSHARELINK
		if ( strcmp( STRMLINK_TYPE_IPEDGE_DVBCSHARELINK , strLinkType.c_str() ) == 0 ) 
		{
			{
				ZQ::common::MutexGuard gd(_sharedDVBCLinkAttrLocker);
				LinkDVBCShareAttrMap::const_iterator it  = _sharedDVBCLinkAttrmap.find( strStramlinkID );
				if ( it == _sharedDVBCLinkAttrmap.end() ) 
				{
					MLOG(ZQ::common::Log::L_ERROR,
						CLOGFMT(IpEdgePHO,"doFreeResources() can't find target link with DVBC shared linkId [%s] for ticket[%s]"),
						strStramlinkID.c_str() ,ticketID.name.c_str() );
					return ;
				}
				
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(IpEdgePHO , "doFreeResources() find target StreamLinkId[%s] through DVBC shared link Id[%s] for ticket[%s]" ),
					it->second._streamLinkId.c_str(),strStramlinkID.c_str() , ticketID.name.c_str() );
				strStramlinkID = it->second._streamLinkId;
				strLinkType = STRMLINK_TYPE_IPEDGE_DVBC;
			}			
		}
		else if ( strcmp( STRMLINK_TYPE_IPEDGE_IPSHARELINK , strLinkType.c_str() ) == 0 ) 
		{
			{
				ZQ::common::MutexGuard gd(_sharedIPLinkAttrLocker);
				LinkIPShareAttrMap::const_iterator it = _sharedIPLinkAttrmap.find(strStramlinkID);
				if( it == _sharedIPLinkAttrmap.end() )
				{
					
					MLOG(ZQ::common::Log::L_ERROR , 
						CLOGFMT(IpEdgePHO ,"doFreeResources() can't find target link with IP shared linkId [%s]" ),
						strStramlinkID.c_str() );
					return;
				}
				MLOG(ZQ::common::Log::L_INFO ,
					CLOGFMT(IpEdgePHO,"doFreeResources() find target streamLinkId[%s] through IP shared linkid[%s] for ticket[%s]"),
					it->second._streamLinkId.c_str() , strStramlinkID.c_str(),ticketID.name.c_str() );
				strStramlinkID = it->second._streamLinkId;
				strLinkType = STRMLINK_TYPE_IPEDGE_IP;
			}
		}

		TianShanIce::Variant val;
		

		if( strcmp(STRMLINK_TYPE_IPEDGE_DVBC,strLinkType.c_str())==0 )
		{//dvbc mode
			ZQ::common::MutexGuard gd(_dvbcResourceLocker);

			//find if there is a allocated dvbc resource
			ResourceDVBCDataMap::iterator itAlloc=_dvbcResourceDataMap.find(ticketID.name);
			if( itAlloc == _dvbcResourceDataMap.end() )
			{//if not,return without free any resource
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(IpEdgePHO,"doFreeResource() no allocated dvbc resource with tickID=[%s]"),
					ticketID.name.c_str());
				return;
			}

			LinkDVBCAttrMap::iterator it=_StreamLinkDVBCAttrmap.find(strStramlinkID);
			if(it==_StreamLinkDVBCAttrmap.end())
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"doFreeResource() can't find strmlink attr through streamlink[%s] for ticket[%s]"),strStramlinkID.c_str() , ticketID.name.c_str() );
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
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() can't find bandwith in resources,invalid resources for ticket[%s]"), ticketID.name.c_str());
				}
				if (val.type!=TianShanIce::vtLongs || val.lints.size()<=0|| val.lints[0]!= itAlloc->second._usedBandWidth ) 
				{
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() can't find bandwith in resources or invalid bandwidth data for ticket[%s]"),ticketID.name.c_str() );
				}				
				try
				{
					val=GetResourceMapData(resMap,TianShanIce::SRM::rtMpegProgram,"Id");
				}
				catch(TianShanIce::InvalidParameter&)
				{
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() can't find program number in resources for ticket[%s]"),ticketID.name.c_str());
				}
				if ((val.type!=TianShanIce::vtInts) || (val.ints.size()==0) ||(val.ints[0]!=itAlloc->second._usedPN)) 
				{
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() can't find program number in resources or invalid pn data for ticket[%s] "),ticketID.name.c_str() );
				}
			}
			catch (Ice::Exception& ex) 
			{
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() ice exception [%s] error when validate the ticket private data for ticket[%s]"),
						ex.ice_name().c_str(),ticketID.name.c_str());
			}
			catch (...) 
			{
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() unexpetc error when validate the ticket private data for ticket[%s]") , ticketID.name.c_str() );
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
							MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() streamlink [%s]"
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
						MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() streamlink [%s]"
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
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() streamlink [%s]"
						" [ticket %s] with PN=[%d] BW=[%lld] and now totalBW=%lld usedBW=%lld  totalPN=[%d] usedPN=[%d]"),
						strStramlinkID.c_str(),ticketID.name.c_str(),
						pn,bandwidth,
						it->second._totalBandWidth,it->second._usedBandwidth,
						it->second._totalPNCount,it->second._totalPNCount-(int)it->second._availablePN.size());
					
				}	
				else
				{
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IpEdgePHO,"doFreeResource() streamlink [%s]"
						" [ticket %s] with PN=[%d](discard pnMax[%d] pnMin[%d]) BW=[%lld] and now totalBW=%lld usedBW=%lld  totalPN=[%d] usedPN=[%d]"),
						strStramlinkID.c_str(),ticketID.name.c_str(),
						pn, it->second._pnMax,it->second._pnMin, bandwidth,
						it->second._totalBandWidth,it->second._usedBandwidth,
						it->second._totalPNCount,it->second._totalPNCount-(int)it->second._availablePN.size());
					
				}	
			}			
			_dvbcResourceDataMap.erase(itAlloc);
		}
		else if (strcmp(STRMLINK_TYPE_IPEDGE_IP,strLinkType.c_str())==0)
		{//IP mode
			ZQ::common::MutexGuard gd(_ipResourceLocker);
			ResourceIPDataMap::iterator itAlloc=_ipResourceDataMap.find(ticketID.name);
			if(itAlloc==_ipResourceDataMap.end())
			{
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(IpEdgePHO,"doFreeResource() streamlink [%s] ticket[%s] no allocated IP resource"),
					strStramlinkID.c_str() , ticketID.name.c_str() );
				return;
			}			
			//TianShanIce::ValueMap value=ticket->getPrivateData();
			LinkIPAttrMap::iterator it=_StreamLinkIPAttrmap.find(strStramlinkID);
			if( it == _StreamLinkIPAttrmap.end() )
			{
				MLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(IpEdgePHO,"doFreeResource() streamlink [%s] ticket[%s] can't find strmlink attr "),
					strStramlinkID.c_str(), ticketID.name.c_str());
				return;
			}
			LinkIPAttr& ipAttr = it->second;
			ResourceIPData& rid=itAlloc->second;

			if (rid._usedSrcPort > 0 &&(rid._usedSrcPort <= ipAttr._maxSrcPort && rid._usedSrcPort>= ipAttr._minSrcPort)) 
			{
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(IpEdgePHO,"doFreeResource() streamlink [%s] ticket[%s] return available used source port[%d],and MaxPort[%d] MinPort[%d]"),
					strStramlinkID.c_str() ,ticketID.name.c_str (),
					rid._usedSrcPort,ipAttr._maxSrcPort,ipAttr._minSrcPort);
				//check if there is already have a port in availPort list
				std::list<Ice::Int>::iterator itPort = ipAttr._availSrcPort.begin ();
				bool bFoundPort =false ;
				for ( ;  itPort != ipAttr._availSrcPort.end () ; itPort++ ) 
				{
					if (rid._usedSrcPort == *itPort) 
					{
						bFoundPort = true;
						MLOG(ZQ::common::Log::L_ERROR,
							CLOGFMT(IpEdgePHO,"doFreeResource() streamlink [%s] ticket[%s] There is already a port[%d] in AvailableList"),
							strStramlinkID.c_str() ,ticketID.name.c_str (),	rid._usedSrcPort);
						break;
					}
				}
				if (!bFoundPort) 
				{
					ipAttr._availSrcPort.push_front (rid._usedSrcPort);
				}
			}

			ipAttr._usedBandwidth -= rid._usedBanwidth;
			ipAttr._usedStreamCount -- ;

			ipAttr._usedBandwidth		= ipAttr._usedBandwidth > 0 ? ipAttr._usedBandwidth : 0 ;
			ipAttr._usedStreamCount		= ipAttr._usedStreamCount > 0 ? ipAttr._usedStreamCount : 0 ;


			int iTotalSrcPort= ipAttr._maxSrcPort - ipAttr._minSrcPort + 1;
			int iUsedSrcPort = iTotalSrcPort- ipAttr._availSrcPort.size();

			MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(IpEdgePHO,"doFreeResource() streamlink [%s] ticket[%s]  "
					"free ip resource with bandwidth=[%lld] srcPort =[%d]   ,"
					"now totalbandwidth=[%lld] usedBandwidth=[%lld] totalStrmCount=[%d]"
					" usedStrmCount=[%d] totalSrcPort[%d]  usedSrcPort=[%d] "),
					strStramlinkID.c_str (),
					ticketID.name.c_str(),
					rid._usedBanwidth,
					rid._usedSrcPort,					
					ipAttr._totalBandwidth,
					ipAttr._usedBandwidth,
					ipAttr._totalStreamCount,
					ipAttr._usedStreamCount,					
					iTotalSrcPort,
					iUsedSrcPort);

			_ipResourceDataMap.erase(itAlloc);
		}
		else
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"doFreeResource() unrecognised stram link [%s] type [%s] for ticket[%s]"),
			 	strStramlinkID.c_str() ,strLinkType.c_str() , ticketID.name.c_str() );
			return;
		}
	}
	catch (TianShanIce::BaseException& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"doFreeResource() catch a tianshanice exception:%s"),ex.message.c_str());
		return;
	}
	catch(Ice::Exception& e)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"doFreeResource() catch a ice exception:%s"),e.ice_name().c_str());
		return;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IpEdgePHO,"doFreeResource() catch a unexpect exception"));
		return;
	}	
}

void IpEdgePHO::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx)
{
	
}

void IpEdgePHO::validate_IPSharedStrmLinkConfig( const char* identStr, ::TianShanIce::ValueMap& configPD )
{
	MLOG(ZQ::common::Log::L_DEBUG , CLOGFMT(IpEdgePHO,"enter validate a IP shared StreamLink's configuration :link[%s]") ,identStr );
	IPSharedAttr lsa;
	try
	{
		ZQ::common::MutexGuard gd(_sharedIPLinkAttrLocker);
		TianShanIce::Variant var;
		var.bRange = false;
		var = PDField(configPD,"LinkId");
		if ( var.type != TianShanIce::vtStrings ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,"IpEdgePHO", 1051 ,"Invalid LinkId type,should be vtString");
		}
		if ( var.strs.size() == 0 ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgePHO", 1052 ,"Invalid LinkId with size() == 0 ");
		}
		std::string& strTempId = var.strs[0];
		std::string::size_type slashPos = strTempId.find("/");
		if ( slashPos != std::string::npos ) 
		{
			strTempId = strTempId.substr( slashPos+1 );
		}
		lsa._streamLinkId = strTempId;
		MLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(IpEdgePHO, "Update IP shared stream link with id[%s] and target LinkId[%s]"),
				identStr , lsa._streamLinkId.c_str() );
		_sharedIPLinkAttrmap[identStr] = lsa;
		
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	MLOG(ZQ::common::Log::L_DEBUG , CLOGFMT( IpEdgePHO , "leave validate a IP shared streamlink's configuration: link[%s]" ),identStr);
}

void IpEdgePHO::validate_DvbcSharedStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD)
{
	MLOG(ZQ::common::Log::L_DEBUG , CLOGFMT( IpEdgePHO , "enter validate a DVBC shared streamlink's configuration: link[%s]" ),identStr);
	DVBCSharedAttr dsa;
	try
	{
		ZQ::common::MutexGuard gd(_sharedDVBCLinkAttrLocker);
		TianShanIce::Variant var ;
		var.bRange = false;
		var = PDField(configPD , "LinkId");
		if ( var.type != ::TianShanIce::vtStrings ) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgePHO", 1051 ,"Invalid LinkId type,should be vtString");
		}
		if (var.strs.size() == 0) 
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("IpEdgePHO", 1052 ,"Invalid LinkId with size() == 0 ");
		}
		std::string& strTempId = var.strs[0];
		std::string::size_type slashPos = strTempId.find("/");
		if ( slashPos != std::string::npos ) 
		{
			strTempId = strTempId.substr( slashPos+1 );
		}
		dsa._streamLinkId = strTempId;
		MLOG(ZQ::common::Log::L_INFO,
			CLOGFMT( IpEdgePHO , "Update DVBC shared stream link with id[%s] and target LinkId[%s]" ),
			identStr , dsa._streamLinkId.c_str());
		_sharedDVBCLinkAttrmap[identStr] = dsa;		
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	MLOG(ZQ::common::Log::L_DEBUG , CLOGFMT( IpEdgePHO , "leave validate a DVBC shared streamlink's configuration: link[%s]" ),identStr);
}

}} // namespace




