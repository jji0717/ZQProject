// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: DenyList.h,v 1.5 2004/07/29 05:12:23 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : define the deny list
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/DenyList.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 5     04-11-08 18:01 Hui.shao
// const paramters
// 
// 4     04-08-20 14:30 Kaliven.lee
// 
// 3     04-08-19 11:53 Kaliven.lee
// modify function getNode
// Revision 1.5  2004/07/29 05:12:23  shao
// moved preference process into class conversation
//
// Revision 1.4  2004/07/22 09:04:50  shao
// no message
//
// Revision 1.3  2004/07/22 06:17:48  shao
// adjusted some method/paramter names, and added comments
//
// Revision 1.2  2004/07/07 02:48:04  shao
// load() to load the deny list setting from a configuraton
//
// Revision 1.1  2004/06/25 06:45:30  shao
// created
//
// ===========================================================================

#ifndef	__ZQ_DenyList_H__
#define	__ZQ_DenyList_H__

#include "InetAddr.h"
#include "Locks.h"

#include <vector>

// -----------------------------
// class DenyList
// -----------------------------
/// keeps a list of InetAddress, with mask for widecasting, to compare
typedef struct _node
{
	ZQ::common::InetAddress host;
	ZQ::common::InetMaskAddress mask;
	// TODO: IPv6 wildcast is not supported
	int port;
} node_t;
class DenyList
{
public:

	/// Constructor.
	DenyList() {} 

	/// destructor
	~DenyList() { clear(); }
public:

	/// returns the number of records in the list
	/// @return        the number of records
	int size() { return _nodes.size(); }

	/// add a single record to the list
	/// @param denyaddr  address string, can be a widecast such as "192.168.0.*" 
	/// @param port      port number to match, 0 if ignored
	/// @return          true if successful
	bool add(const char* denyaddr, const int port=0);

	/// clear all records in the list
	void clear(void);

	/// compare an address with the list to see if it match any one
	/// @param addr		 Inet address to look for
	/// @param port      port number to match, 0 if ignored
	/// @return          true if matched any one in the list
	bool match(const ZQ::common::InetHostAddress addr, const port=0);

	/// add by kaliven.lee for service application shell to access the denfylist
	bool getNode(node_t * pNode,DWORD pos = 0);


protected:
	typedef std::vector< node_t > nodes_t;
	nodes_t _nodes;
	ZQ::common::Mutex _nodesLock;
};

#endif // __ZQ_DenyList_H__
