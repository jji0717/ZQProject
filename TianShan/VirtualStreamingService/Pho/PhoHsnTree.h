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

#ifndef __ZQTianShan_PathHelper_HsnTree_H__
#define __ZQTianShan_PathHelper_HsnTree_H__


#include "ZQ_common_conf.h"

#include <vector>
#include <string>
#include <list>

#include "Exception.h"
#include "Locks.h"
#include "NativeThread.h"
#include "SystemUtils.h"



#include "IPathHelperObj.h"

namespace ZQTianShan {
namespace AccreditedPath {


#define STRMLINK_TYPE_HsnTree		"SeaChange.VSS.HSNTree"
#define HsnTreeMaxStreamCount		"MaxStreamCount"
#define HsnTreeQammodulationFormat	"Qam.modulationFormat"
#define HsnTreeQamIP				"Qam.IP"
#define HsnTreeQamport				"Qam.port"
#define HsnTreeQamMac				"Qam.Mac"
#define HsnTreeQamsymbolRate		"Qam.symbolRate"
#define HsnTreeQamfrequency			"Qam.frequency"
#define HsnTreePN					"PN"


class PHOHsnTree;

class ReplicaSubscriberI : public TianShanIce::ReplicaSubscriber , public ZQ::common::NativeThread
{
public :
	ReplicaSubscriberI( PHOHsnTree& phoTree );
	virtual ~ReplicaSubscriberI();
public:
	
	virtual void	updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& , const ::TianShanIce::Replicas&, const ::Ice::Current& = ::Ice::Current()) ;

	void			stop( );

	bool			isStreamerAvailable( const std::string& streamerId );

	void			checkStreamer( const std::string& streamerId , const std::string& streamerEndpoint);

protected:
	
	int				run( );

private:
	
	PHOHsnTree&			mPhoTree;
	int32				mMaxUpdateInterval;
	SYS::SingleObject		mEventHandle;
	bool				mbQuit;
};

typedef IceUtil::Handle<ReplicaSubscriberI> ReplicaSubscriberIPtr;

class PHOHsnTree : public ::Ice::LocalObject, public IStreamLinkPHO
{
public:

	PHOHsnTree(IPHOManager& mgr);
	virtual ~PHOHsnTree();

public:
	
	bool					init( );

	void					uninit( );
	
	virtual bool			getSchema( const char* type, ::TianShanIce::PDSchema& schema);

	virtual void			validateConfiguration( const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD);

	virtual NarrowResult	doNarrow( const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

	virtual void			doFreeResources( const ::TianShanIce::Transport::PathTicketPtr& ticket);

	virtual Ice::Int		doEvaluation( LinkInfo& linkInfo, 
											const SessCtx& sessCtx, 
											TianShanIce::ValueMap& hintPD, 
											const ::Ice::Int oldCost);

	virtual void			doCommit( const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);


protected:
	
	void					increasePenalty( const std::string& streamerId , int maxPenaltyValue );


	void					decreasePenalty( const std::string& streamerId );

	virtual Ice::Int		eval_IPStrmLink(LinkInfo& linkInfo, 
											const SessCtx& sessCtx, 
											TianShanIce::ValueMap& hintPD, 
											TianShanIce::SRM::ResourceMap& rcMap , 
											const ::Ice::Int oldCost );
	
	virtual void			validate_IPStrmLinkConfig(const char* identStr, ::TianShanIce::ValueMap& configPD);


	NarrowResult			narrow_IPStrmLink(::TianShanIce::Transport::StreamLinkPrx& strmLink, 
													const SessCtx& sessCtx,
													const TianShanIce::Transport::PathTicketPtr& ticket);


	IPHOManager*	_phoManager;

private:
	
	friend class ReplicaSubscriberI;

	typedef struct _streamerAttr 
	{
		std::string		streamerId;
		Ice::Int		penaltyValue;	//Only when this value is equal to or below 0 ,we can order stream on this streamer
		Ice::Long		lastUpdateTime;
		bool			bStatus;
		_streamerAttr()
		{
			bStatus			= false;
			penaltyValue	= 0;
			lastUpdateTime	= 0;
		}
	}streamerPenaltyAttr;

	typedef std::map<std::string , streamerPenaltyAttr>	streamerPenaltyAttrMap;
	streamerPenaltyAttrMap			_streamerMap;
	ZQ::common::Mutex				_streamerMapLock;
	
	ReplicaSubscriberIPtr			mStremerReplicaManager;
	Ice::CommunicatorPtr			mIc;
	Ice::ObjectAdapterPtr			mAdapter;

	typedef struct _tagLinkIPAttr
	{
		std::string			_streamLinkID;
		//Ice::Int			_maxStreamCount;
		//Ice::Int			_usedStreamCount;
		Ice::Int			_QammodulationFormat;
		std::string			_QamIP;
		Ice::Int			_Qamport;
		std::string			_QamMac;
		Ice::Int			_QamsymbolRate;
		Ice::Int			_Qamfrequency;
		Ice::Int			_PN;	
		Ice::Int			_usedPN;
	}LinkIPAttr;

	typedef struct _tagResourceIPData
	{		
		Ice::Int			_usedPN;
	}ResourceIPData;
	//Ice::Int				_usePN;


	typedef std::map<std::string,LinkIPAttr>		LinkIPAttrMap;
	typedef std::map<std::string,ResourceIPData>	ResourceIPDataMap;//<ticketid,ResourceIPData>

	LinkIPAttrMap						_StreamLinkIPAttrmap;
	ResourceIPDataMap					_ipResourceDataMap;
	
	ZQ::common::Mutex					_ipResourceLocker;
};

//#define NGOD_RES_PREFIX(X)	"NGOD.R2."#X

}} // namespace

#endif //__ZQTianShan_PathHelper_HsnTree_H__
