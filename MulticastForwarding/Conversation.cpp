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
// Ident : $Id: Conversation.cpp,v 1.7 2004/07/29 06:26:30 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : impl the conversation
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/Conversation.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 17    04-11-19 15:43 Hui.shao
// 
// 16    04-11-18 16:25 Hui.shao
// new XML configuration
// 
// 15    04-11-08 18:03 Hui.shao
// switched listener to sniffer
// 
// 14    04-10-10 18:08 Hui.shao
// accept "anyaddress" for binding due to ipv6 problem
// 
// 13    04-09-15 16:43 Kaliven.lee
// add stamp for packet lost test
// 
// 12    04-09-15 16:42 Kaliven.lee
// 
// 11    04-09-15 14:54 Hui.shao
// fix the wrong denied source address in log message
// 
// 10    04-09-09 10:30 Kaliven.lee
// 
// 9     04-08-26 11:44 Kaliven.lee
// cast hostaddress to char*
// 
// 8     04-08-26 9:53 Kaliven.lee
// fixed wrong tag name
// 
// 7     04-08-22 14:35 Jian.shen
// 
// 6     04-08-20 14:29 Kaliven.lee
// 
// 5     04-08-19 11:52 Kaliven.lee
// add friend
// 
// 4     04-08-18 18:07 Kaliven.lee
// Revision 1.7  2004/07/29 06:26:30  shao
// no message
//
// Revision 1.6  2004/07/29 05:12:44  shao
// moved preference process from class denylist
//
// Revision 1.5  2004/07/22 09:05:52  shao
// impl direct re-multicast conversation
//
// Revision 1.4  2004/07/22 06:17:48  shao
// adjusted some method/paramter names, and added comments
//
// Revision 1.3  2004/07/21 07:10:57  shao
// global logger
//
// Revision 1.2  2004/07/20 13:30:13  shao
// 2nd stage impl
//
// Revision 1.1  2004/07/07 11:42:13  shao
// created
//
// ===========================================================================
#include "McastFwdConf.h"
#include "Conversation.h"
#include "McastFwd.h"

// -----------------------------
// class Conversation
// -----------------------------

#define CVSTLOGFMT(_X) LOGFMT("conversation[%s:%d] " _X ), _group.getHostAddress(), _port

DenyList Conversation::_senderList;
int Conversation::_lastID =0;

extern bool gbPeerStampFlag ;

Conversation::Conversation(McastFwd* theServer /*=NULL*/)
:_pTheSvr(theServer), _bValid(false), _port(0), _listenerCount(0)
{
	_id = _lastID++;
}

Conversation::~Conversation()
{
	stop();
}

bool Conversation::stop()
{
	log(ZQ::common::Log::L_DEBUG, CVSTLOGFMT("stop"));

	 // for the listeners that are blocked on recv(), send a dumy message to wake them up
#define DUMY_MESSAGE "bye"
	mcastData(DUMY_MESSAGE, sizeof (DUMY_MESSAGE)-1, _group, _port);

	// clean up the resenders
	{
		ZQ::common::Guard< ZQ::common::Mutex > guard(_remcastersLock);
		for (remcasters_t::iterator it= _remcasters.begin(); it < _remcasters.end(); it++)
			delete (*it);
		_remcasters.clear();
	}

/*
// clean up the listeners
	{
		ZQ::common::Guard< ZQ::common::Mutex > guard(_listenersLock);
		for (listeners_t::iterator it= _listeners.begin(); it < _listeners.end(); it++)
		{
			// ScThreadPool doesn't support terminate thread
			(*it)->quit(); // TODO: more process required to clean up the thread
			delete (*it);
		}
		_listeners.clear();
	}
*/
	return true;
}

bool Conversation::start()
{
	if (_port<=0)
	{
		log(ZQ::common::Log::L_ERROR, LOGFMT("unable to start conversation: unknown multicast group/port"));
		return false;
	}

	if (_listenerCount <=0)
	{
		log(ZQ::common::Log::L_ERROR, LOGFMT("unable to start conversation: no valid listener has been specified"), _group.getHostAddress(), _port);
		return false;
	}

//	log(ZQ::common::Log::L_DEBUG, CVSTLOGFMT("start"));
//	// now start each listener
//	for (listeners_t::iterator it= _listeners.begin(); it < _listeners.end(); it++)
//		(*it)->start();
		
	return true;
}

bool Conversation::isDenied(const ZQ::common::InetHostAddress& source, int sport)
{
	// valid the source against local sender interfaces
	if (_senderList.match(source, sport))
	{
#ifdef _DEBUG
		std::string addrstr = source.getHostAddress();
		log(ZQ::common::Log::L_DEBUG, CVSTLOGFMT("%s:%d is a sender interface, denied"), addrstr.c_str(), sport);
#endif
		return true;
	}

	// valid the source against local denylist
	if (_localDenylist.match(source, sport))
		return true;

	// valid the source against the default denylist
	if (_pTheSvr != NULL)
		return _pTheSvr->isDenied(source, sport);

	return false;
}

int Conversation::mcastData(const void* message, const int len, const ZQ::common::InetMcastAddress& group, const int gport, bool redirect/*=true*/, const TunnelConnection* pTConn/*=NULL*/)
{
	if (message == NULL || len <0)
	{
		log(ZQ::common::Log::L_WARNING, CVSTLOGFMT("NULL message to resend, ignored"));
		return false;
	}

	if (_remcasters.size() <=0)
	{
		// no local resender specified
		if (redirect && _pTheSvr!=NULL)
			return _pTheSvr->mcastData(message, len, group, gport, false, pTConn);

		log(ZQ::common::Log::L_DEBUG, CVSTLOGFMT("no valid sender to mcastData the forward traffic"));
		return 0;
	}

	// re-multicast thru local resenders
	int ret = 0;
	for (remcasters_t::iterator it = _remcasters.begin(); it < _remcasters.end(); it ++)
	{

		ZQ::common::tpport_t port;
		log(ZQ::common::Log::L_DEBUG, CVSTLOGFMT("send via %s:%d"), (*it)->getLocal(&port).getHostAddress(), port);


		(*it)->setGroup(group, gport);
		(*it)->setTimeToLive(1);
		ret = (*it)->send(message, len);
//		log(ZQ::common::Log::L_DEBUG,CVSTLOGFMT("send via %s:%d"), (*it)->getLocal(&port).getHostAddress(), port);
	}

	return ret;
}

bool Conversation::initialize(ZQ::common::IPreference* preference, bool checkPrefName/*=false*/)
{
	if (_bValid)
	{
		log(ZQ::common::Log::L_ERROR, CVSTLOGFMT("conversation has already been initialized, reject to load preference again"));
		return false;
	}

	char buf[1024];
	// check if match the preference name
	if (preference ==NULL || (checkPrefName && strcmp("Conversation", preference->name(buf))!=0))
	{
		log(ZQ::common::Log::L_ERROR, LOGFMT("illegal conversation preference"));
		return false;
	}

	// read the group/port of this conversation
	_port =0;
	preference->get("groupAddr", buf, "");
	if (strlen(buf)>0)
	{
		try
		{
			_group.setAddress(buf);
			_port = atoi(preference->get("groupPort", buf, "0"));
		}
		catch(ZQ::common::Exception e)
		{
			log(ZQ::common::Log::L_WARNING, LOGFMT("illegal conversation preference: group %s:%d %s"), buf, _port, e.getString());
			_port = 0;
		}
	}

	if (_port <=0)
	{
		log(ZQ::common::Log::L_ERROR, LOGFMT("conversation has no valid mulitcast group/port to listen"));
		return false;
	}
	
	log(ZQ::common::Log::L_INFO, CVSTLOGFMT("initialiazing"));

	// process <MulticastSender>
	bringup_McastSender(preference);

	// process <MulticastListener>
	bringup_McastListener(preference);

	// initiliaze the listeners based on default configuration
	if (_pTheSvr != NULL && _listenerCount<=0)
	{
		int c = _pTheSvr->defaultListenAddressCount();
		for (int i =0; i< c; i++)
		{
			ZQ::common::InetHostAddress bindaddr = _pTheSvr->defaultListenAddress(i);
/*			McastListener* listener = new McastListener(_group, _port, bindaddr, this);
			if (listener == NULL)
				continue;

			ZQ::common::Guard< ZQ::common::Mutex > guard(_listenersLock);
			_listeners.push_back(listener);
*/
			if(!_pTheSvr->_theSniffer.open(_group, _port, bindaddr, this))
				continue;

			_bindaddrs += std::string((_listenerCount++==0)?"":",") + bindaddr.getHostAddress();

			log(ZQ::common::Log::L_INFO, CVSTLOGFMT("listening on default interface %s"), bindaddr.getHostAddress());
		}
	}

	// process <DenyList>
	ZQ::common::IPreference* ldlPref = preference->firstChild("DenyList");
	if (ldlPref !=NULL)
	{
		log(ZQ::common::Log::L_INFO, CVSTLOGFMT("applying private deny list"));
		bringup_DenyList(ldlPref);
		ldlPref->free();
	}

	// process <Tunnels> to create private tunnels
	ZQ::common::IPreference* tunnelPref = preference->firstChild("Tunnels");
	if (tunnelPref !=NULL)
	{
		log(ZQ::common::Log::L_INFO, CVSTLOGFMT("bring up private tunnels"));
		bringup_Tunnels(tunnelPref);
		tunnelPref->free();
	}

	_bValid = (_listenerCount >0);

	if(_bValid)
		log(ZQ::common::Log::L_INFO, CVSTLOGFMT("successfuly initialized"));
	else
		log(ZQ::common::Log::L_ERROR, CVSTLOGFMT("failed to initialize"));

	return _bValid;
}

bool Conversation::bringup_Tunnels(ZQ::common::IPreference* pref)
{
	if (pref ==NULL)
		return false;

	for(ZQ::common::IPreference* connpref= pref->firstChild("Connection");
		connpref != NULL;
		connpref = pref->nextChild())
	{
		char buf[512];
		int remotePort = atoi(connpref->get("remotePort", buf, "0"));
		
		if (remotePort <=0) 
			remotePort = DEFAULT_TUNNEL_LISTERN_PORT;

		// the address pair
		ZQ::common::FtHostAddress localAddr(true), remoteAddr;

		// the local address pair to bind
		{
			ZQ::common::IPreference* prefAddr= connpref->firstChild("LocalAddress");
			if (prefAddr != NULL)
			{
				localAddr += prefAddr->get("address", buf, "");
				prefAddr->free();
			}

			prefAddr= connpref->nextChild();
			if (prefAddr != NULL)
			{
				localAddr += prefAddr->get("address", buf, "");
				prefAddr->free();
			}
		}

		// remote address pair
		{
			ZQ::common::IPreference* prefAddr= connpref->firstChild("RemoteAddress");
			if (prefAddr != NULL)
			{
				remoteAddr += prefAddr->get("address", buf, "");
				prefAddr->free();
			}

			// the backup remote address
			prefAddr= connpref->nextChild();
			if (prefAddr != NULL)
			{
				remoteAddr += prefAddr->get("address", buf, "");
				prefAddr->free();
			}
		}

		connpref->free();

		// try to create the tunnel
		log(ZQ::common::Log::L_DEBUG, CVSTLOGFMT("creating tunnel %s => %s:%d"), (const char*)localAddr, (const char*)remoteAddr, remotePort);
		TunnelConnection* conn = establishTunnel(localAddr, remoteAddr, remotePort,  DEFAULT_TIMEOUT);
		if(conn ==NULL)
		{
			log(ZQ::common::Log::L_ERROR, CVSTLOGFMT("failed to establish tunnel %s => %s:%d"), (const char*)localAddr,(const char*)remoteAddr, remotePort);
			continue;
		}

		log(ZQ::common::Log::L_INFO, CVSTLOGFMT("created tunnel %s => %s:%d; active %s=>%s"), (const char*) localAddr,(const char*) remoteAddr, remotePort, conn->getLocalAddr(), conn->getRemoteAddr());

		// scan if this connection is already in the list
		for (tunnels_t::iterator it = _tunnels.begin(); it< _tunnels.end() && (*it) != conn; it++);
		
		if (it == _tunnels.end()) // new connection
			 _tunnels.push_back(conn);

	} // conn loop

	return true;
}

bool Conversation::bringup_McastListener(ZQ::common::IPreference* pref)
{
	if (pref ==NULL || _port <=0 || _pTheSvr ==NULL)
		return false;

	ZQ::common::IPreference* McastListenerPref = pref->firstChild("MulticastListener");
	if (McastListenerPref ==NULL)
		return false;

	for(ZQ::common::IPreference* addrpref= McastListenerPref->firstChild("LocalAddress");
		addrpref;
		addrpref=McastListenerPref->nextChild())
	{
		char buf[512];
		addrpref->get("address", buf, "");
		ZQ::common::InetHostAddress addr;
		if (addr.setAddress(buf) && (addr.isAnyAddress() || addr == gLocalAddrs)) // test if it is local
		{
/*
			McastListener* listener = new McastListener(_group, _port, addr, this);
			ZQ::common::Guard< ZQ::common::Mutex > guard(_listenersLock);
			_listeners.push_back(listener);
			openedListener ++;
*/
			if (_pTheSvr->_theSniffer.open(_group, _port, addr, this))
				_bindaddrs += std::string((_listenerCount++==0)?"":",") + addr.getHostAddress();

			log(ZQ::common::Log::L_INFO, CVSTLOGFMT("listening on interface %s"), buf);
		}

		addrpref->free();
	}

	McastListenerPref->free();

	return (_listenerCount>0);
}

bool Conversation::bringup_McastSender(ZQ::common::IPreference* pref)
{
	if (pref ==NULL)
		return false;

	int openedSender =0;

	ZQ::common::IPreference* McastSenderPref = pref->firstChild("MulticastSender");
	if (McastSenderPref ==NULL)
		return false;

	char buf[512];
	int port = atoi(McastSenderPref->get("localPort", buf, "0"));
	if (port <=0)
		port = DEFAULT_REMCAST_PORT;

	for(ZQ::common::IPreference* addrpref = McastSenderPref->firstChild("LocalAddress");
		addrpref;
		addrpref = McastSenderPref->nextChild())
	{
//		int port = atoi(sockpref->get("port", buf, "0"));
//		if (port <=0)
//			port = DEFAULT_REMCAST_PORT;

		addrpref->get("address", buf, "");
		
		log(ZQ::common::Log::L_DEBUG, CVSTLOGFMT("opening a resender on %s:%d"), buf, port);
		ZQ::common::UDPMulticast* sender = new ZQ::common::UDPMulticast(buf, port);
		if (sender!=NULL && sender->isActive())
		{
			ZQ::common::Guard< ZQ::common::Mutex > guard(_remcastersLock);
//			sender->setGroup(_group, _port);
//			sender->setTimeToLive(1);
			_remcasters.push_back(sender);
			openedSender++;
			log(ZQ::common::Log::L_INFO, CVSTLOGFMT("opened a resender on %s:%d"), buf, port);
			//added the interface in the sender deny list
			_senderList.add(buf, port);
		}
		else
		{
			if (sender!=NULL)
				delete sender;
			sender = NULL;
			log(ZQ::common::Log::L_ERROR, CVSTLOGFMT("failed to open a mcast sender on %s:%d"), buf, port);
		}

		addrpref->free();
	}

	McastSenderPref->free();

	return (openedSender>0);
}

bool Conversation::bringup_DenyList(ZQ::common::IPreference* preference)
{
	if (preference ==NULL)
		return false;

	char buf[1024];

	// load the <Source /> elements
	for(ZQ::common::IPreference* srcpref= preference->firstChild("Source");
		srcpref;
		srcpref=preference->nextChild())
	{
		int port = atoi(srcpref->get("port", buf, "0"));
        srcpref->get("address", buf, "");

		if (strlen(buf)>0)
		{
			log(ZQ::common::Log::L_DEBUG, CVSTLOGFMT("adding %s:%d into deny list"), buf, port);
			if (_localDenylist.add(buf, port))
				log(ZQ::common::Log::L_INFO, CVSTLOGFMT("added %s:%d into deny list"), buf, port);
		}

		srcpref->free();
	}

	return true;
}

bool Conversation::isValid()
{
	return (_listenerCount >0);
}

int Conversation::OnMcastMessage(const McastMsgInfo& MsgInfo)
// int Conversation::OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress& source, int sport, ZQ::common::InetMcastAddress* group, int gport)
{
	std::string shoststr = MsgInfo.source.getHostAddress(), groupstr = MsgInfo.group.getHostAddress();
#ifdef _DEBUG
	log(ZQ::common::Log::L_DEBUG, CVSTLOGFMT("captured a package on %s:%d from %s:%d"), groupstr.c_str(), MsgInfo.gport, shoststr.c_str(), MsgInfo.sport);
#endif
	// validate with the denylist
	if (isDenied(MsgInfo.source, MsgInfo.sport))
	{
		log(ZQ::common::Log::L_INFO, CVSTLOGFMT("message from %s:%d is denied"), shoststr.c_str(), MsgInfo.sport);
		return 0;
	}

	if(gbPeerStampFlag)
	{
		char* buf = (char*)MsgInfo.data;
		buf[0] = MCASTFWD_PEER_STAMP;
	}
	// if tunnels is empty but the resender is not empty, go thru the local resenders
	if (_pTheSvr != NULL && _tunnels.size()<=0 && _remcasters.size()>0)
	{
		log(ZQ::common::Log::L_INFO, CVSTLOGFMT("direct re-multicast thru private senders"));
		return mcastData(MsgInfo.data, MsgInfo.datalen, MsgInfo.group, MsgInfo.gport);
	}

	//TODO: if this is the server, adopt all the orphans
	if (_pTheSvr == NULL)
	{
	}
	
	// if has private tunnels then forward the data thru the private tunnels
	if (_tunnels.size()>0)
	{
		log(ZQ::common::Log::L_INFO, CVSTLOGFMT("forward thru %s tunnels"), (_pTheSvr != NULL ? "private": "default"));
		int i=0;
		for(tunnels_t::iterator it = _tunnels.begin(); it<_tunnels.end(); it++)
		{
			if ((*it)!=NULL)
			{
				(*it)->forwardData(MsgInfo.data, MsgInfo.datalen, MsgInfo.source, MsgInfo.sport, MsgInfo.group, MsgInfo.gport);
				i++;
			}
		}
		return i;
	}

	// if doesn't have local tunnels then call the server OnCapturedData();
	if (_pTheSvr != NULL)
		return _pTheSvr->OnMcastMessage(MsgInfo);
		
	return 0;
}

void Conversation::OnFowardedData(const TunnelConnection* pTConn, const void *data, const int datalen, ZQ::common::InetHostAddress& source, int sport, ZQ::common::InetMcastAddress& group, const int gport)
{
	log(ZQ::common::Log::L_DEBUG, CVSTLOGFMT("conversation %d mcastData to group: %s:%d"), _id,group.getHostAddress(), gport);
#ifdef _DEBUG
	// dump the message to screen
	char *msg= (char*) data;
	for (int i =0; i< datalen; i++, msg++)
		printf("%c", isprint(*msg) ? *msg : '.');
	printf("\n");
#endif // _DEBUG

	mcastData(data, datalen, group, gport, true, pTConn);
}

TunnelConnection* Conversation::establishTunnel(const ZQ::common::FtHostAddress& local, const ZQ::common::FtHostAddress& remote, const int port,  timeout_t timeout)
{
	TunnelConnection* conn = gTunnelManager.connect(local, remote, port, timeout);

	if (conn==NULL)
		return NULL;

#ifndef ALLOW_TUNNEL_SELF
	if(conn->_localid == conn->_peerid)
	{
		//give up this connection
		conn->close();
		delete conn;

		log(ZQ::common::Log::L_ERROR, CVSTLOGFMT("not allow to connected to self, drop the connection"));
		return (conn = NULL);
	}
#endif

	// make sure it is a unique connection
	// scan the established connections for matched _peerid
	int tunnelnum = gTunnelManager.size();
	int i=0;
	for (i=0; i<tunnelnum; i++)
	{
		TunnelConnection* oldConn = (TunnelConnection*) gTunnelManager[i];

		if (oldConn==NULL || oldConn==conn || conn->_peerid != oldConn->_peerid)
			continue;

		// found one tunnel to the same machine
		if (oldConn->_status == TunnelConnection::ACTIVE)
		{
			log(ZQ::common::Log::L_WARNING, CVSTLOGFMT("one tunnel to the same machine has already been established and active, close current connection"));
			//give up this connection
			conn->close();
			delete conn;

			// use the already active connection
			conn = oldConn;
		}
#ifdef _DEBUG
		else if(conn->_localid < conn->_peerid)
#else
		else if(conn->_localid.compare(conn->_peerid) <=0)
#endif
		{
			// the peer of this connection has the sin to create the connection
			log(ZQ::common::Log::L_WARNING, CVSTLOGFMT("the other side has the sin to connect, close current connection"));
			//give up this connection
			conn->close();
			delete conn;

			// use the connection established by the peer
			conn = oldConn;
		}
	}

	if (conn!=NULL)
		conn->addOwner(this);

	return conn;
}
