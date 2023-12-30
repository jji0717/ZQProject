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
// Ident : $Id: TsStreamer.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ComcastNGOD/NSS/NSSEx.ICE $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 4     08-12-26 10:35 Xiaoming.li
// impl new playlist interface playItem
// 
// 3     08-07-21 15:32 Xiaoming.li
// modify nonmutating to ["cp:const", "freeze:read"]
// 
// 2     08-07-14 14:54 Xiaoming.li
// 
// 1     08-06-13 11:23 Xiaoming.li
// 
// 1     08-04-03 12:12 Hui.shao
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE --output-dir .. $(InputDir)\$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE --index "TianShanIce::Streamer::NGODStreamServer::SessionIdx,TianShanIce::Streamer::NGODStreamServer::NGODStreamEx,sessKey,case-sensitive" SessionIdx --output-dir .. $(InputDir)\$(InputName).ice

#ifndef __ZQ_ComcastNGOD_NSSEX_ICE__
#define __ZQ_ComcastNGOD_NSSEX_ICE__

#include "NSS.ICE"
#include <Ice/Identity.ice>

module TianShanIce
{

module Streamer
{

module NGODStreamServer
{

struct TransHeader
{
	string	clientId;
	string	destination;
	string	clientPort;
	string	sopName;
	string	bandWidth;
};
sequence <TransHeader> transHeaderList;

struct stItem
{
	string	assetId;
	string	providerId;
	int		itemNum;
	//int		whereInsert;		///where the item insert before
	//int		currentUserCtrlNum;	///current item user control number
	//int		inTimeOffset;
	//int		outTimeOffset;
	//int		criticalStart;
	//bool	spliceIn;
	//bool	spliceOut;
	//bool	forceNormal;
};
sequence <stItem> ItemList;

// -----------------------------
// class NGODStreamEx
// -----------------------------
/// represents a streaming session on a NGOD-compatbile Streaming Server
["freeze:write"]
class NGODStreamEx extends TianShanIce::Streamer::NGODStreamServer::NGODStream
{
	///volume string used in RTSP
	string	volume;	
	
	///specify the client address
	transHeaderList	tansportHeaderList;
	
	///play item list
	ItemList	assetId;
	
	///current played item number
	//int		currentItemNum;
	
	///string of pathTicket proxy
	string	pathTicketStr;

	///get the NGOD client volume
	///@return the NGOD client volume
	["cpp:const", "freeze:read"] string getVolume();

	///get the TransportHeader list
	///@return ngod TransportHeader list
	["cpp:const", "freeze:read"] transHeaderList getTransportHeader();

	///get the asset id list
	///@return the asset id list
	["cpp:const", "freeze:read"] ItemList getAssetId();
};
 
}; // module NGODStreamServer

}; // Streamer

}; // TianShanIce
#endif // __ZQ_ComcastNGOD_NSSEX_ICE__
