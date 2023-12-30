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
// Ident : $Id: Conversation.h,v 1.5 2004/07/29 05:12:44 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : define the conversation
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/Conversation.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 4     04-11-08 18:03 Hui.shao
// switched listener to sniffer
// 
// 3     04-09-28 14:16 Hui.shao
// 
// 2     04-08-19 11:52 Kaliven.lee
// add friend
// Revision 1.5  2004/07/29 05:12:44  shao
// moved preference process from class denylist
//
// Revision 1.4  2004/07/22 09:05:52  shao
// impl direct re-multicast conversation
//
// Revision 1.3  2004/07/22 06:17:48  shao
// adjusted some method/paramter names, and added comments
//
// Revision 1.2  2004/07/20 13:30:13  shao
// 2nd stage impl
//
// Revision 1.1  2004/07/07 11:42:13  shao
// created
//
// ===========================================================================

#ifndef	__ZQ_Conversation_H__
#define	__ZQ_Conversation_H__

#include "McastFwdConf.h"
#include "McastSniffer.h"
#include "DenyList.h"
#include "Log.h"
#include "Tunnel.h"
#include "IPreference.h"

/*
	<Conversation>
        <Socket address="My_V6_MulticastGroup" port="7000"/>

        <LocalListenAddress>
            <Socket address="3f::22"/>
            <Socket address="LocalV6IP.company.com"/>
        </LocalListenAddress>

        <LocalSendAddress>
            <Socket address="3f::22" port="6000"/>
            <Socket address="LocalV6IP.company.com" port="6000"/>
        </LocalSendAddress>

        <RemoteServer>
            <Connection>
                <AddrsLocal>
                    <Address address="192.168.80.138"/>
                    <Address address="192.168.0.8"/>
                </AddrsLocal>
                <AddrsRemote>
                    <Socket address="192.168.0.138" port="1000"/>
                    <Socket address="192.168.80.8" port="1000"/>
                </AddrsRemote>
            </Connection>
        </RemoteServer>

		<DenyList>
            <Socket address="3f::22" port="6000"/>
            <Socket address="MyV6IP.company.com" port="6000" family="???"/>
        </DenyList>
    </Conversation>
*/

// -----------------------------
// class Conversation
// -----------------------------
/// define one multicast group/port to monitor, it at least have one listener interface
/// it also can have its private sender and tunnel connections. when the senders and tunnels
/// are not provided, it uses those defined in the server
class Conversation : public McastSubscriber
{
	friend class McastFwd;
	friend class TunnelConnection;
	friend class MCastFwdServ;

public:

	/// Constructor.
	/// @param theServer   point to the server instance, NULL if it is an orphan
	Conversation(McastFwd* theServer);

	/// Destructor.
	~Conversation();

	/// Initial the conversation with a preference
	/// @param preference      point to a preference instance which define this coversation
	/// @param checkPrefName   false if ignore the top preference name
	/// @return                true if successful
	virtual bool initialize(ZQ::common::IPreference* preference, bool checkPrefName=true);

	/// Start this conversation, the convesation must be initialized and valid
	/// @return                true if successful
	virtual bool start();

	/// Stop this conversation, the conversation can not be start once stopped
	/// @return                true if successful
	virtual bool stop();
	
	/// Check if the conversation is initialized successfully
	/// @return                true if valid
	virtual bool isValid();

	/// get the server instance
	/// @return                point to the server instance, NULL if this is an orphan
	virtual McastFwd* getServer() { return _pTheSvr; }

	/// re-multicast the message to the specified group/port thru the sender interfaces
	/// @param message         message to send
	/// @param len             length of the message
	/// @param group           the target multicast group
	/// @param port            the target multicast port
	/// @param redirect        true redirect to the server's sender to multicast if not
	///                        handled by the private senders
	/// @param pTConn		   point to which tunnel connection that the data was forwarded from
	/// @return                byte of the message that has been sent
	virtual int  mcastData(const void* message, const int len, const ZQ::common::InetMcastAddress& group, const int gport, bool redirect=true, const TunnelConnection* pTConn=NULL);

protected:

	/// Event fired when the listener interface captured a message on the conversation group/port
	/// inherit from McastSubscriber
	/// @param McastMsgInfo    captured message informatin
	/// @return                byte of the message that has been processed
	virtual int OnMcastMessage(const McastMsgInfo& MsgInfo);
//	virtual int  OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress& source, int sport, ZQ::common::InetMcastAddress* group =NULL, int gport =-1);

	/// Event fired when the tunnel received a message from the peer
	/// @param pTConn		   point to which tunnel connection that the data was forwarded from
	/// @param message         message to send
	/// @param len             length of the message
	/// @param source          the source machine where the message was sent from
	/// @param sport           the source port that the message was sent
	/// @param group           the target multicast group
	/// @param gport           the target multicast port
	/// @return                byte of the message that has been processed
	virtual void OnFowardedData(const TunnelConnection* pTConn, const void *data, const int datalen, ZQ::common::InetHostAddress& source, int sport, ZQ::common::InetMcastAddress& group, const int gport);

	/// Check if the source IP/port is denied
	/// @param message         message to send
	/// @param len             length of the message
	/// @param source          the source machine where the message was sent from
	/// @param sport           the source port that the message was sent
	/// @param group           the target multicast group
	/// @param gport           the target multicast port
	/// @return                byte of the message that has been processed
	virtual bool isDenied(const ZQ::common::InetHostAddress& source, int sport);

	// init conversation components based on the preference
	virtual bool bringup_Tunnels(ZQ::common::IPreference* pref);
	virtual bool bringup_McastListener(ZQ::common::IPreference* pref);
	virtual bool bringup_McastSender(ZQ::common::IPreference* pref);
	virtual bool bringup_DenyList(ZQ::common::IPreference* preference);

	/// establish a single tunnel connection to the remote service
	/// @param local        local address to bind
	/// @param remote       remote address to connect to
	/// @param port         remote port to connect to
	/// @param timeout      connection must be established in a certain timeout
	/// @return             point to a TunnelConnection, NULL if failed
	TunnelConnection* establishTunnel(const ZQ::common::FtHostAddress& local, const ZQ::common::FtHostAddress& remote, const int port,  timeout_t timeout);;

protected:

	std::string _bindaddrs;

//	typedef std::vector < std::string > listeners_t;
//	listeners_t _listeners;
//	ZQ::common::Mutex _listenersLock;
	int _listenerCount;


	typedef std::vector < ZQ::common::UDPMulticast* > remcasters_t;
	remcasters_t _remcasters;
	ZQ::common::Mutex _remcastersLock;

	typedef std::vector < TunnelConnection* > tunnels_t;
	tunnels_t _tunnels;
	ZQ::common::Mutex _tunnelsLock;

	bool _bValid;

	ZQ::common::InetMcastAddress _group;
	int _port;

	McastFwd* _pTheSvr;

	DenyList _localDenylist;
	static DenyList _senderList;
	std::string _establishTimeStr;

	static int _lastID;
	int _id;
};

#endif // __ZQ_Conversation_H__