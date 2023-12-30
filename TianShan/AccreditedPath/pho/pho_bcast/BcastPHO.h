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
// Author: l.huang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/pho/pho_bcast/BcastPHO.h $
// 
// 1     7/07/15 4:16p Li.huang
// 
// ===========================================================================

#ifndef __ZQTianShan_PathHelper_BCAST_H__
#define __ZQTianShan_PathHelper_BCAST_H__

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

#define STRMLINK_TYPE_MRT_PUBPOINT				"xor-media.bcast.mrt-pubpoint"

class BcastPHO : public ::Ice::LocalObject, public IStreamLinkPHO 
{
public:

	BcastPHO(IPHOManager& mgr);
	virtual ~BcastPHO();

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
	virtual IPathHelperObject::NarrowResult doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

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

protected:

	int evalBcastStreamLinkCost(const std::string& streamLinkID,Ice::Long bw2Alloc,const std::string& sessId);
	virtual Ice::Int eval_BcastStrmLink(LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, TianShanIce::SRM::ResourceMap& rcMap ,const ::Ice::Int oldCost);
	virtual void validate_BcastStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD);
	NarrowResult narrow_BcastStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, const SessCtx& sessCtx, const TianShanIce::Transport::PathTicketPtr&  ticket);


	void increasePenalty ( const std::string& streamerId , int maxPenaltyValue);

protected:
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
		std::string             _MRTUrl;
		
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


private:	
	IPHOManager*	_phoManager;
private:
	
};

}} // namespace

#endif // __ZQTianShan_PathHelper_BCAST_H__

