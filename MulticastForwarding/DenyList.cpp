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
// Ident : $Id: DenyList.cpp,v 1.6 2004/07/29 05:12:23 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : impl the deny list
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/DenyList.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 5     04-11-18 16:25 Hui.shao
// accept leading '1' as mask
// 
// 4     04-08-20 14:30 Kaliven.lee
// 
// 3     04-08-19 11:53 Kaliven.lee
// modify function getNode
// Revision 1.6  2004/07/29 05:12:23  shao
// moved preference process into class conversation
//
// Revision 1.5  2004/07/22 09:04:50  shao
// no message
//
// Revision 1.4  2004/07/22 06:17:48  shao
// adjusted some method/paramter names, and added comments
//
// Revision 1.3  2004/07/07 11:42:39  shao
// no message
//
// Revision 1.2  2004/07/07 02:48:04  shao
// load() to load the deny list setting from a configuraton
//
// Revision 1.1  2004/06/25 06:45:30  shao
// created
//
// ===========================================================================

#include "DenyList.h"
#include "McastFwdConf.h"

// -----------------------------
// class DenyList
// -----------------------------

void DenyList::clear(void)
{
	ZQ::common::Guard< ZQ::common::Mutex > guard(_nodesLock);
	_nodes.clear();
}


bool DenyList::add(const char* denyaddr, const int port /*=0*/)
{
	if (denyaddr == NULL || *denyaddr ==0x00)
		return false;

	node_t node;
	node.mask = "255.255.255.255"; // as a host address
	if (node.host.setAddress(denyaddr))
		node.mask = (node.host.family() == PF_INET) 
					? "255.255.255.255"
					: "ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff";
	else
	{
		// now trust as that has wildcast elements in the string
		std::string subnetaddr;
		std::string maskaddr;

		std::string addr = denyaddr;
		int pos = addr.find_first_of("/");
		
		if (pos !=std::string::npos)
		{
			// if this is in xxx.xxx.xxx.xxx/xx format
			int bits = atoi(addr.substr(pos+1).c_str());
			if (bits <=0)
				return false; // illeagle
			
			addr = addr.substr(0, pos);

			if (!node.host.setAddress(addr.c_str()))
				return false;

			node.mask = ZQ::common::InetMaskAddress(node.host.family(), bits);
		}
		else
		{
			// in xxx.xxx.xxx.* format

			// TODO: IPv6 wildcast is not supported

			// test if it is a valid numeric IPv4 address
			if (addr.find_first_not_of("0123456789.* ") !=std::string::npos)
				return false;

			// now build the wildcast node
			const int len = addr.length();
			int i =0, elem=0;
			while (i < len)
			{
				i = addr.find_first_not_of(".", i);
				if (i == std::string::npos)
					break;

				int j = addr.find_first_of (".", i);
				std::string addrElem;
				if (j == std::string::npos)
				{
					addrElem = addr.substr(i);
					i = len;
				}
				else
				{
					addrElem = addr.substr(i, j-i);
					i=j+1;
				}

				elem ++;

				if (addrElem == "*")
				{
					subnetaddr += "0.";
					maskaddr   += "0.";
				}
				else
				{
					subnetaddr += addrElem + ".";
					maskaddr   +="255.";
				}
			}

			if(elem!=4)
				return false; // illegal address if meet here

			// set the node value pair
			node.mask = maskaddr.substr(0, maskaddr.length()-1).c_str();
			node.host = subnetaddr.substr(0, subnetaddr.length()-1).c_str();
		}

//		node.host = node.mask & node.host;
	}

	// add the new node into the list
    ZQ::common::Guard< ZQ::common::Mutex > guard(_nodesLock);
	node.port = port;
	_nodes.push_back(node);

	return true;
}

bool DenyList::match(const ZQ::common::InetHostAddress addr, const int port /*=0*/)
{
	// scan the deny list
	for(nodes_t::iterator it = _nodes.begin(); it< _nodes.end(); it ++)
	{
		if (it->port !=0 && it->port !=port)
			continue;

		ZQ::common::InetHostAddress add2test = addr;
		
		// check if the family is matched
		if (add2test.family() != it->host.family())
			continue;

		// IPv6 address verification
		if (add2test.family() == PF_INET6)
		{
			//TODO: wildcast for IPv6
			if (add2test == it->host)
				return true;

			continue;
		}

		// IPv4 address verification
		if (add2test.family() == PF_INET)
		{
			//std::string str[4];
			//str[2] = it->mask.getHostname();
			//str[1] = it->host.getHostname();
			//str[0] = add2test.getHostname();
			add2test &= it->mask;
			//str[3] = add2test.getHostname();

			if (add2test == it->host)
				return true;
		}
	}

	return false;
}
bool DenyList::getNode(node_t* pNode,DWORD pos)
{
	if((pos >=  0)&&(pos < _nodes.size()))
	{
		*pNode = _nodes.at(pos);
		return true;
	}
	return false;

}
