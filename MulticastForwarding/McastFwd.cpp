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
// Ident : $Id: McastFwd.cpp,v 1.5 2004/07/29 05:13:27 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : impl the main service class McastFwd
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/McastFwd.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 13    04-11-19 15:43 Hui.shao
// 
// 12    04-11-18 16:26 Hui.shao
// new XML configuration
// 
// 11    04-11-08 18:03 Hui.shao
// switched listener to sniffer
// 
// 10    04-10-10 18:09 Hui.shao
// accept "anyaddress" to bind due to the ipv6 problems
// 
// 9     04-09-09 17:45 Kaliven.lee
// 
// 8     04-09-09 10:33 Kaliven.lee
// make some change so that service class can inherit from mcastfwd
// 
// 7     04-08-22 16:37 Kaliven.lee
// Revision 1.5  2004/07/29 05:13:27  shao
// log msg on denylist
//
// Revision 1.4  2004/07/22 06:17:48  shao
// adjusted some method/paramter names, and added comments
//
// Revision 1.3  2004/07/21 07:10:57  shao
// global logger
//
// Revision 1.2  2004/07/20 13:30:19  shao
// 2nd stage impl
//
// Revision 1.1  2004/07/07 11:42:22  shao
// created
//
// ===========================================================================
#include "McastFwdconf.h"
#include "McastFwd.h"

// -----------------------------
// class McastFwd
// -----------------------------


McastFwd::McastFwd()
: Conversation(NULL)
{
}

McastFwd::~McastFwd()
{
	for (conversations_t::iterator it = _conversations.begin(); it<_conversations.end(); it++)
	{
		if ((*it) !=NULL)
			delete (*it);
		_conversations.erase(it);
	}
}

bool McastFwd::initialize(ZQ::common::IPreference* preference, bool checkPrefName /*=true*/, int tunnelListenPort /*= DEFAULT_TUNNEL_LISTERN_PORT*/)
{
	_tunnelListenPort = tunnelListenPort;

	_pTheSvr = NULL; // _pTheSvr always equals to NULL to avoid redirecting
	if (preference == NULL)
		return false;

	log(ZQ::common::Log::L_DEBUG, LOGFMT("processing preference"));
	char buf[1024];

	// default settings
	ZQ::common::IPreference* defaultPref = preference->firstChild("Default");
	if (defaultPref != NULL)
	{	
		log(ZQ::common::Log::L_DEBUG, LOGFMT("start default settings"));
		
	// the tunnel listener
		ZQ::common::IPreference* prefTunnelListener = defaultPref->firstChild("TunnelListener");
		bool bTunnelListenerInited =false;
		if (prefTunnelListener != NULL)
		{	
			log(ZQ::common::Log::L_DEBUG, LOGFMT("openning the tunnel listener to accept incoming connections"));
			
			// read the port number
			_tunnelListenPort = atoi(prefTunnelListener->get("localPort", buf, "0"));
			if (_tunnelListenPort<=0)
				_tunnelListenPort = DEFAULT_TUNNEL_LISTERN_PORT;

			// the tunnel server bind address pair
			ZQ::common::FtHostAddress bindAddr(true);

			for (ZQ::common::IPreference* prefAddr = prefTunnelListener->firstChild("LocalAddress");
				 prefAddr !=NULL;
				 prefAddr = prefTunnelListener->nextChild())
			{
				bindAddr += prefAddr->get("address", buf, "");
				prefAddr->free();
			}

			prefTunnelListener->free();
		
			if (bindAddr.getAddressCount() <= 0)
			{
				log(ZQ::common::Log::L_DEBUG, LOGFMT("no valid interface to initialize tunnel server"));
			}
			else
			{
				// initialize the tunnel server
				if (gTunnelManager.listen(bindAddr, _tunnelListenPort))
				{
					log(ZQ::common::Log::L_INFO, LOGFMT("opened tunnel server on %s,%s:%d"), bindAddr.getHostAddress(0), bindAddr.getHostAddress(1), _tunnelListenPort);
					bTunnelListenerInited =true;
				}
			}
		}

		if (!bTunnelListenerInited)
			log(ZQ::common::Log::L_WARNING, LOGFMT("no tunnel server initialized"));

		// process <LocalSendAddress>
		bringup_McastSender(defaultPref);

		// process <MulticastListener>
		ZQ::common::IPreference* prefMCL = defaultPref->firstChild("MulticastListener");
		if (prefMCL !=NULL)
		{
			log(ZQ::common::Log::L_DEBUG, LOGFMT("reading the default listen interfaces"));
			for(ZQ::common::IPreference* addrpref= prefMCL->firstChild("LocalAddress");
				addrpref;
				addrpref=prefMCL->nextChild())
			{
				addrpref->get("address", buf, "");
				ZQ::common::InetHostAddress addr;
				if (addr.setAddress(buf) && (addr.isAnyAddress() || addr == gLocalAddrs)) // test if it is local
				{
					ZQ::common::Guard< ZQ::common::Mutex > guard(_listenAddrsLock);
					_listenAddrs.push_back(addr);
					log(ZQ::common::Log::L_INFO, LOGFMT("added default listen interface %s"), addr.getHostAddress());
				}
				else
					log(ZQ::common::Log::L_INFO, LOGFMT("illegal default listen interface %s"), buf);

				addrpref->free();
			}

			prefMCL->free();
		}

		// process <DenyList>
		ZQ::common::IPreference* prefDL = defaultPref->firstChild("DenyList");
		if (prefDL !=NULL)
		{
			log(ZQ::common::Log::L_INFO, LOGFMT("applying default deny list"));
			bringup_DenyList(prefDL);
			prefDL->free();
		}

		// process <Tunnels> to create default tunnels
		ZQ::common::IPreference* prefTunnel = defaultPref->firstChild("Tunnels");
		if (prefTunnel !=NULL)
		{
			log(ZQ::common::Log::L_DEBUG, LOGFMT("establishing default tunnels"));
			bringup_Tunnels(prefTunnel);
			prefTunnel->free();
		}

		defaultPref->free();
		log(ZQ::common::Log::L_DEBUG, LOGFMT("end of the default settings"));

	} // end of default settings

	// process <Conversation /> elements
	log(ZQ::common::Log::L_DEBUG, LOGFMT("initializing conversations"));
	for(ZQ::common::IPreference* cvstPref= preference->firstChild("Conversation");
		cvstPref;
		cvstPref=preference->nextChild())
	{
		Conversation* cvst = new Conversation(this);
		ZQ::common::Guard< ZQ::common::Mutex > guard(_conversationsLock);
		if (cvst->initialize(cvstPref) && cvst->isValid())
			_conversations.push_back(cvst);
		else
			delete cvst;

		cvstPref->free();
	}

	log(ZQ::common::Log::L_DEBUG, LOGFMT("finished processing preference"));

	if (_conversations.size() <=0)
	{
		log(ZQ::common::Log::L_ERROR, LOGFMT("no valid conversation has been initialized"));
		return false;
	}

	return true;
}

bool McastFwd::isValid()
{
	return (_conversations.size() >0 );
}

/*
int McastFwd::OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress& source, int sport, ZQ::common::InetMcastAddress* group, int gport)
{
	_pTheSvr = NULL; // _pTheSvr always equals to NULL to avoid redirecting
	return Conversation::OnCapturedData(data, datalen, source, sport, group, gport);
}
*/

bool McastFwd::start()
{
	_pTheSvr = NULL; // _pTheSvr always equals to NULL to avoid redirecting
	bool succ = false;
	
/*
// start default listeners
	if (_listeners.size() >0)
		if (Conversation::start())
			succ = true;
*/
	// start each sub conversation now
	for (conversations_t::iterator it = _conversations.begin(); it<_conversations.end(); it++)
	{
		if ((*it) !=NULL &&  (*it)->start())
			succ = true;
	}

	_theSniffer.start();

	return succ;
}

bool McastFwd::stop()
{
	log(ZQ::common::Log::L_ERROR, LOGFMT(""));
	bool succ = false;

	// stop the sniffer
	_theSniffer.stop();

	// stop the tunnel server
	gTunnelManager.close();

	// stop each sub conversation now
	for (conversations_t::iterator it = _conversations.begin(); it<_conversations.end(); it++)
	{
		if ((*it) !=NULL &&  (*it)->stop())
			succ = true;
	}

	return succ;
}
