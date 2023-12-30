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
// Ident : $Id: IpEdgePHO.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/pho/IpEdgePHO.h $
// 
// 3     9/05/13 3:26p Zonghuan.xiao
// modify log from glog to mlog because glog cannot confirm which pclog
// used
// 
// 2     3/10/11 2:25p Fei.huang
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 26    08-08-11 15:44 Hongquan.zhang
// 
// 25    08-07-08 16:00 Hongquan.zhang
// 
// 24    08-03-18 14:49 Hongquan.zhang
// check in for StreamLink Penalty and DVBC share link
// 
// 23    08-01-03 15:55 Hui.shao
// move PDSchema namespace
// 
// 22    07-12-14 11:38 Hongquan.zhang
// update Error Code
// 
// 21    07-11-19 11:49 Hongquan.zhang
// 
// 20    07-09-18 12:55 Hongquan.zhang
// 
// 19    07-08-30 15:47 Hongquan.zhang
// 
// 18    07-03-22 17:24 Hongquan.zhang
// 
// 17    07-03-21 13:57 Hongquan.zhang
// 
// 16    07-03-14 12:33 Hongquan.zhang
// 
// 15    07-01-09 15:10 Hongquan.zhang
// 
// 14    06-12-28 16:43 Hongquan.zhang
// 
// 13    06-12-25 12:22 Hongquan.zhang
// 
// 12    06-12-20 11:27 Hongquan.zhang
// 
// 11    06-09-19 11:46 Hui.shao
// 
// 10    06-09-18 19:26 Hui.shao
// use plugin for PHO
// ===========================================================================

#ifndef __ZQTianShan_PathHelper_IpEdge_H__
#define __ZQTianShan_PathHelper_IpEdge_H__

#include "ZQ_common_conf.h"
#include "Exception.h"
#include "Locks.h"
#include "NativeThreadPool.h"
#include "SystemUtils.h"

#include <vector>
#include <string>
#include <list>


#include "../../common/IPathHelperObj.h"

namespace ZQTianShan {
namespace AccreditedPath {

#define STRMLINK_TYPE_IPEDGE_IP					"SeaChange.IpEdge.IP"
#define STRMLINK_TYPE_IPEDGE_DVBC				"SeaChange.IpEdge.DVBC"
#define STRMLINK_TYPE_IPEDGE_DVBCSHARELINK		"SeaChange.IpEdge.DVBC.ShareLink"
#define STRMLINK_TYPE_IPEDGE_IPSHARELINK		"SeaChange.IpEdge.IP.ShareLink"



class IpEdgePHO : public ::Ice::LocalObject, public IStreamLinkPHO , public ZQ::common::NativeThread
{
public:

	IpEdgePHO(IPHOManager& mgr);
	virtual ~IpEdgePHO();

public:

	/// Implementations of IPathHelperObject
	///@{

	/// exports the private data schema of a specific StorageLink or StreamLink type from an IPathHelperObject object
	///@param[in] type	the string type of link
	///@param[out] schema the schema of the acceptable private data bound with this type of StorageLink
	///@return true if succeeded
	virtual bool getSchema(const char* type, ::TianShanIce::PDSchema& schema);

	/// validate the configuration with the schema of the type
	///@param[in] type	the string type of link
	///@param[in] identStr the unique key to identify this link
	///@param[in, out]  configPD the configuration set based on the schema, some value may be adjusted during validation
	///@throw ::TianShanIce::InvalidParameter if some unacceptable configurations are given
	virtual void validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD);

	/// narrow the negotiable resources and context bound within a session based on this specific type
	///@param[in] ticket	access to the ticket that narrow for
	///@param[in] sess	access to the session context that narrow for
	///@return NarrowResult types
	///@sa IPHOManager::doNarrow()
	virtual NarrowResult doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

	/// free a ticket, the PHO must perform the steps to free the resource allocated for this ticket
	///@param[in, out] ticket	access to the ticket of which PHO should free the allocated resource	
	///@sa IPHOManager::doFreeResource()
	///@note this invocation is from ::TianShanIce::Transport::PathTicket::destroy()
	///@note PHO may test ::TianShanIce::Transport::PathTicket::stampComitted, which will be stamped by PHOManager if doNarrow() succeeds,
	///      to check if the resource allocation has been committed	
	virtual void doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket);
	///@}

	/// Implementaions of IStreamLinkPHO
	///@{

	/// evaluates a given StreamLink for its cost. The evaluation is based on the resources collection and other
	/// recognizable context in the given session, when some required fields are not available in the session
	/// context, it will then refer to the hinted private data.\n
	///@param[in] link	access to the StreamLink that about to evaluate
	///@param[in] sess	access to the session context that evaluate for
	///@param[in] hintedPD 	hinted private data in the case they are short in the given session context
	///@param[in] oldCost	the initial cost
	///@return the higher value of the old cost and cost result of this evaluation
	///@note the resource and context in the session may also be updated during evaluation
	///@note when a NULL session has been given, all necessary context updates will be applied into hintedPD instead
	///@sa getStorageLinkSchema() for hintedPD fields
	//virtual Ice::Int doEvaluation(::TianShanIce::Transport::StreamLinkPrx& link, const TianShanIce::SRM::SessionPrx& sess, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap , const ::Ice::Int oldCost);
	virtual Ice::Int doEvaluation(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost) ;

	/// commit the path that has been employed, the helper manager will dispatch the routine to the doCommit() method
	/// of the related path helper objects
	///@param[in] sess	access to the session context that narrow for
	///@param[in] ticketIdent identity to open the allocated ticket
	///@return true if succeeded
	///@sa IPathHelperObject::doCommit()
	virtual void doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

	///@}

	//increase penalty value for a streamer through the streamerId
	//@param streamerId the net id of streamer on which we can't create stream	
	virtual void		increasePenalty( const std::string& streamerId , int maxPenaltyValue  );
	
	//decrease penalty value
	//@param streamerId streamer's Id
	//	if createStream is OK ,streamerId is the id of the streamer on which we can create stream
	// if createStream is failed,streamerId is empty
	// How to decrease the penalty value is up to PHO	
	virtual void		decreasePenalty( const std::string& streamerId );

protected:
	
	///thread run	
	int						run(void);

	virtual Ice::Int eval_IPStrmLink(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap ,const ::Ice::Int oldCost);
	virtual Ice::Int eval_IPShareStrmLink(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap ,const ::Ice::Int oldCost);
	virtual Ice::Int eval_DvbcStrmLink(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap ,const ::Ice::Int oldCost);
	virtual Ice::Int eval_DvbcShareStrmLink(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap ,const ::Ice::Int oldCost);
	
	virtual void validate_IPStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD);
	virtual void validate_IPSharedStrmLinkConfig( const char* identStr, ::TianShanIce::ValueMap& configPD );
	virtual void validate_DvbcStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD);
	virtual void validate_DvbcSharedStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD);
	

	NarrowResult narrow_IPStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
									const SessCtx& sessCtx,
									const TianShanIce::Transport::PathTicketPtr& ticket);

	NarrowResult narrow_IPSharedStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
										const SessCtx& sessCtx,
										const TianShanIce::Transport::PathTicketPtr&  ticket);

	NarrowResult narrow_DvbcStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
										const SessCtx& sessCtx,
										const TianShanIce::Transport::PathTicketPtr&  ticket);
	
	NarrowResult narrow_DvbcSharedStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
										const SessCtx& sessCtx,
										const TianShanIce::Transport::PathTicketPtr&  ticket);
private:	
	int			evalDVBCStreamLinkCost(const std::string& streamLinkID,Ice::Long bw2Alloc,const std::string& sessID );
	
	NarrowResult inner_narrow_DvbcStrmLink(const std::string strmLinkId , 
											const SessCtx& sessCtx,
											const TianShanIce::Transport::PathTicketPtr&  ticket);
	
	NarrowResult inner_narrow_IPStrmLink(const std::string strmLinkId , 
											const SessCtx& sessCtx,
											const TianShanIce::Transport::PathTicketPtr&  ticket);

	IPHOManager*	_phoManager;
private:
	
	typedef struct _streamerAttr 
	{
		std::string		streamerId;
		Ice::Int		penaltyValue;	//Only when this value is equal to or below 0 ,we can order stream on this streamer
	}streamerAttr;

	typedef std::map<std::string , streamerAttr>	streamerAttrMap;
	streamerAttrMap		_streamerMap;
	ZQ::common::Mutex	_streamerMapLock;
	///decreaseCount	
	long	volatile	_decreaseCount;
	ZQ::common::Mutex	_decreaseCountLock;
	SYS::SingleObject	_hEventForDereasePenalty;
	bool				_bQuitDecreasePenaltyThread;

	typedef struct _tagLinkIPAttr
	{
		std::string			_streamLinkID;
		Ice::Long			_totalBandwidth;
		Ice::Long			_availableBandwidth;
		Ice::Long			_usedBandwidth;
		Ice::Int			_totalStreamCount;
		Ice::Int			_usedStreamCount;
		Ice::Int			_minSrcPort;	//if no srcport is set then, minPort and MaxPort are -1
		Ice::Int			_maxSrcPort;
		std::string			_srcStreamIP;
		std::string			_destMacAddress;	//the target mac address , actually this may not be the real target mac address
		std::list<Ice::Int>	_availSrcPort;
	}LinkIPAttr;
	
	typedef struct _tagResourceIPData
	{		
		Ice::Long			_usedBanwidth;
		Ice::Int			_usedSrcPort;
	}ResourceIPData;

	typedef std::map<std::string,LinkIPAttr>		LinkIPAttrMap;
	typedef std::map<std::string,ResourceIPData>	ResourceIPDataMap;//<ticketid,ResourceIPData>

	LinkIPAttrMap						_StreamLinkIPAttrmap;
	ResourceIPDataMap					_ipResourceDataMap;
	ZQ::common::Mutex					_ipResourceLocker;

	//IP Shared StreamLink
	typedef struct _tagIPSharedAttr 
	{
		std::string					_streamLinkId;	//point to real link 
	}IPSharedAttr;

	typedef std::map< std::string  , IPSharedAttr > LinkIPShareAttrMap;
	LinkIPShareAttrMap				_sharedIPLinkAttrmap;
	ZQ::common::Mutex				_sharedIPLinkAttrLocker;

	/*------------------------------------------------------------------------------------
	DVBC format
	------------------------------------------------------------------------------------*/
	typedef struct _tagLinkDVBCAttr
	{
		std::string				_streamLinkID;
		Ice::Long				_totalBandWidth;		//total bandwidth in bytes
		Ice::Long				_availableBandwidth;	//available bandwidth in bytes
		Ice::Long				_usedBandwidth;
		Ice::Int				_pnMin;
		Ice::Int				_pnMax;
		Ice::Int				_totalPNCount;
		Ice::Int				_basePort;				//这个是经过计算的basePort
		Ice::Long				_portMask;
		Ice::Int				_portStep;
		Ice::Int				_basePN;		
		std::list<Ice::Int>		_availablePN;			//current available program number
		std::list<Ice::Int>		_backupPN;				//if _pnMin and _pnMax <0 ,this list contain the original PN data
		
	}LinkDVBCAttr;
	
	typedef std::map<std::string,LinkDVBCAttr>	LinkDVBCAttrMap;
	


	
	typedef struct _tagResourceDVBCData					//this data is used to associated with stream link 
	{
		Ice::Long			_usedBandWidth;			//used bandwidth
		Ice::Int			_usedPN;				//used program number		
	}ResourceDVBCData;

	//NOTE: if target streamlink is deleted,the linked shared link must be deleted too !!!!
	typedef std::map< std::string , ResourceDVBCData >	ResourceDVBCDataMap;//<TicketID,ResourceDVBCData>	

	LinkDVBCAttrMap						_StreamLinkDVBCAttrmap;
	ResourceDVBCDataMap					_dvbcResourceDataMap;
	
	ZQ::common::Mutex					_dvbcResourceLocker;


	typedef struct _tagDVBCSharedAttr 
	{
		std::string					_streamLinkId;	//point to real link 
	}DVBCSharedAttr;

	typedef std::map< std::string  , DVBCSharedAttr > LinkDVBCShareAttrMap;
	LinkDVBCShareAttrMap			_sharedDVBCLinkAttrmap;
	ZQ::common::Mutex				_sharedDVBCLinkAttrLocker;
	
};



}} // namespace

#endif // __ZQTianShan_PathHelper_IpEdge_H__

