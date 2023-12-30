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
// Ident : $Id: TsStorageMediaCluster.ICE $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/StreamSmith/NodeContentStore/ICE/TsStorageMediaCluster.ice $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 2     08-08-12 13:22 Hongquan.zhang
// 
// 1     08-04-28 12:20 Hongquan.zhang
// Do not share with \TianShan\ContentStore\Ice
// any longer
// 
// 7     07-12-10 12:38 Fei.huang
// 
// 6     07-10-15 14:39 Fei.huang
// add interface for NodeContentStore to query the current provision mode
// of Cluster
// 
// 5     07-09-04 13:59 Fei.huang
// 1.7 first merge
// 
// 4     07-08-22 11:20 Fei.huang
// 
// 3     07-08-15 10:35 Fei.huang
// 
// 5     07-08-09 18:02 Fei.huang
// 
// 3     07-07-23 11:55 Fei.huang
// 
// 2     06-12-05 15:24 Fei.huang
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_Storage_MediaCluster_ICE__
#define __ZQ_TianShanIce_Storage_MediaCluster_ICE__

#include "TianShanIce.ICE"
#include "TsStorage.ICE"
#include <Ice/Identity.ice>

module TianShanIce
{

// -----------------------------
// namespace Storage
// -----------------------------
/// Storage represents a media content storage within the TianShan architecture
module Storage
{

// -----------------------------
// Service MediaClusterContentStore
// -----------------------------
///@brief represents the private interface between MediaCluster and Node Contentstore
///
interface MediaClusterContentStore extends ContentStore
{
	/// Register the Node ContentStore as the slave of MediaCluster ContentStore
	///@return the typeof the store
	void registerSlaveNode(string nodeID, string nodeEndpoint, string propagationIP);	

	/// notify slave node to update status of a content. (cluster -> slave)
	void notifyContentProvisioned(string name);

	/// query if the content provisioned. (slave -> cluster)
	bool isContentProvisioned(string name);

	/// get provision mode of cluster
	short getProvisionMode();

	/// query if content in disk
//	bool isContentInDisk(string name);
};

class MediaClusterContent extends Content 
{
	/// return play time in milliseconds
	nonmutating long	getPlayTimeEx();
	
	/// notify that content is being Ingesting
	void				contentIsIngesting( );	
	
	/// notify that complete content ingestion
	void				contentIngestOk( );
};

};
};

#endif // __ZQ_TianShanIce_Storage_MediaCluster_ICE__
