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
// Ident : $Id: McastFwdConf.cpp,v 1.8 2004/08/09 10:08:56 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : generate McastFwd extension configuration file
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/McastFwd/McastFwdConf.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 2     04-11-18 19:19 Hui.shao
// limit to enter up to 2 ip for tunnellistener
// ===========================================================================

#include "InetAddr.h"
#include "UDPSocket.h"
#include "XMLPreference.h"

#include "getopt.h"
extern "C"
{
#include <stdio.h>
}

#include <vector>

ZQ::common::ComInitializer comi;
ZQ::common::XMLPrefDoc doc(comi);

void usage()
{
	printf("Usage: McastFwdConf -f <filename>\n");
	printf("       McastFwdConf -h\n");
	printf("generates the extension configuration file for McastFwd service.\n");
	printf("options:\n");
	printf("\t-f   specify the configuration file name\n");
	printf("\t-h   display this help\n");
}

const char* readln(char* line, const char* prompt=NULL);
void InitLocalIPs();
const char* SelectLocalIP(const char* purpose = NULL, const bool bIPv6 =false);
bool BuildConversationPref(ZQ::common::IPreference*pConversation, bool isDefault=false);

typedef std::vector < std::string > strs_t;
typedef std::vector < int > index_t;
static strs_t LocalIPv4Addrs, LocalIPv6Addrs;
static bool bAddrStrsInited =false;


int main(int argc, char* argv[])
{
	char buffer[1024];

	if (argc <2)
	{
		usage();
		exit(0);
	}

	std::string filename;

	ZQ::common::IPreference* confPref =NULL;

	char ch;
	while((ch = getopt(argc, argv, "hf:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			exit(0);

		case 'f':
			filename = optarg;
			// initial the configuration file
			if (filename.empty() || !doc.open(filename.c_str(), XMLDOC_CREATE)
				|| (confPref = doc.newElement("Config")) ==NULL)
			{
				printf("failed to create configuration %s\n", filename.c_str());
				exit(1);
			}
			break;
		}
	}

	// begin the <Default> settings
	printf("The XML-formatted extension configuration for McastFwd is built with a\n"
		"default setting and multiple conversation settings. The filename can be\n"
		"specified in registry:\n"
		"HKEY_LOCAL_MACHINE\\SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\McastFwd\\ExtConfiguration\n\n"
		"This program will guide you start with the default setting and then the\n"
		"conversations.\n\n");

	printf("######################\n");
	printf("## default settings ##\n");
	printf("######################\n");
	ZQ::common::IPreference* pDefault = doc.newElement("Default");

	{
		// example: <TunnelListener localPort="1000">
		//            <LocalAddress address="127.0.0.1"/>
		//            <LocalAddress address="10.3.0.135"/>
		//          </TunnelListener>
		ZQ::common::IPreference* pTunnelListener = doc.newElement("TunnelListener");

		printf("McastFwd can accept incoming tunnel connections from other McastFwd.\n"
			"To make this feature enabled, McastFwd must open a tunnel server\n"
			"listener which requires a port number and local IP addresses to bind.\n");

		while (atoi(readln(buffer, "Enter the TunnelListener port number: "))<=0);
		pTunnelListener->set("localPort", buffer);

		bool bMoreAddr = true;
		int i=0;
		while (bMoreAddr && i<2)
		{
			ZQ::common::IPreference* pAddr = doc.newElement("LocalAddress");
			pAddr->set("address", SelectLocalIP("for TunnelListener"));

			pTunnelListener->addNextChild(pAddr);
			pAddr->free();
			i++;

			bMoreAddr = (tolower(*readln(buffer, "More address to bind TunnelListener [Y/N]: ")) =='y');
		}

		pDefault->addNextChild(pTunnelListener);
		pTunnelListener->free();
	}

	BuildConversationPref(pDefault, true);

	// end of the default settings
	confPref->addNextChild(pDefault);
	pDefault->free();
	printf("---- Default setting completed ----\n\n\n");

	bool bMoreConv  =true;

	printf("######################\n");
	printf("##   Conversations  ##\n");
	printf("######################\n");
	printf("A conversation is a combination of the destination multicast IP address\n"
		"and port number, you must provide the information to specify.\n"
		"Additionally, one conversation can has its own bind IP address to listener\n"
		"and its own sender address. It can also have its own tunnel connections to\n"
		"forward the message to the specified destination McastFwd instance. The\n"
		"conversation will use the default listener and sender interface if the\n"
		"private settings are not provided.\n");

	do
	{
		// <Conversation groupAddr="225.12.12.12" groupPort="6001" />
		ZQ::common::IPreference* pConv = doc.newElement("Conversation");
		std::string ip = readln(buffer, "Enter the multicast group address: ");
		pConv->set("groupAddr",buffer);
		int port=0;
		while ((port = atoi(readln(buffer, "Enter the multicast port number: ")))<=0);
		pConv->set("groupPort", buffer);

		if ((tolower(*readln(buffer, "Do you have any private setting for this conversation [Y/N]: ")) =='y'))
			BuildConversationPref(pConv, false);

		confPref->addNextChild(pConv);
		pConv->free();

		printf("---- conversation [%s]:%d setting completed ----\n\n", ip.c_str(), port);
		bMoreConv = (tolower(*readln(buffer, "Configurate more conversation [Y/N]: ")) =='y');

	} while (bMoreConv);


	doc.set_root(confPref);
	doc.save();
	confPref->free();

	printf("You must be sure the registry:\n"
		"HKEY_LOCAL_MACHINE\\SOFTWARE\\SeaChange\\ITV\\CurrentVersion\\McastFwd\\ExtConfiguration\n\n"
		"is point to the configuration file, then restart the McastFwd service to make active\n"
		"turn the loglevel as DEBUG and check the log messages to verify if the\n"
		"configuration is valid\n");

	return 0;

}

bool BuildConversationPref(ZQ::common::IPreference*pConversation, bool isDefault)
{
	char buffer[1024];
	if (pConversation == NULL)
		return false;

	const char* prompt =isDefault ? "Configure default bind address to sniff mcast traffic [Y/N]: "
		: "Configure private bind address to sniff mcast traffic [Y/N]: ";
	if (tolower(*readln(buffer, prompt)) =='y')
	{
		// example: <MulticastListener>
		//            <LocalAddress address="127.0.0.1"/>
		//            <LocalAddress address="10.3.0.135"/>
		//          </MulticastListener>
		ZQ::common::IPreference* pPref = doc.newElement("MulticastListener");

		bool bMoreAddr = true;
		while (bMoreAddr)
		{
			ZQ::common::IPreference* pAddr = doc.newElement("LocalAddress");
			pAddr->set("address", SelectLocalIP("for sniffing"));

			pPref->addNextChild(pAddr);
			pAddr->free();

			bMoreAddr = (tolower(*readln(buffer, "More bind address to sniff [Y/N]: ")) =='y');
		}

		pConversation->addNextChild(pPref);
		pPref->free();
	}

	prompt =isDefault ? "Configure default bind address to re-mcast forwarded traffic [Y/N]: "
		: "Configure private re-mcast address [Y/N]: ";
	if (tolower(*readln(buffer, prompt)) =='y')
	{
		// example: <MulticastSender localPort="1000">
		//            <LocalAddress address="192.168.80.8" />
		//          </MulticastSender>
		ZQ::common::IPreference* pPref = doc.newElement("MulticastSender");

		prompt =isDefault ? "Enter the default send port number: "
			: "Enter the send port number: ";
		while (atoi(readln(buffer, prompt))<=0);
		pPref->set("localPort", buffer);

		bool bMoreAddr = true;
		while (bMoreAddr)
		{
			ZQ::common::IPreference* pAddr = doc.newElement("LocalAddress");
			pAddr->set("address", SelectLocalIP("for re-mcasting"));

			pPref->addNextChild(pAddr);
			pAddr->free();

			bMoreAddr = (tolower(*readln(buffer, "More bind address to re-mcast [Y/N]: ")) =='y');
		}

		pConversation->addNextChild(pPref);
		pPref->free();
	}

	prompt =isDefault ? "Configure default deny list to filter mcast traffic [Y/N]: "
		: "Configure private deny list to filter mcast traffic [Y/N]: ";
	if (tolower(*readln(buffer, prompt)) =='y')
	{
		// example: <DenyList>
		//            <Source address="192.168.*.178" port ="6000"/>
		//			  <Source address="192.168.8.178/24" />
		//			  <Source address="192.168.8.178" />
		//          </DenyList>
		ZQ::common::IPreference* pPref = doc.newElement("DenyList");

		bool bMoreAddr = true;
		while (bMoreAddr)
		{
			ZQ::common::IPreference* pAddr = doc.newElement("Source");
			pAddr->set("address", readln(buffer, "Enter the source address: "));

			while(*readln(buffer, "Enter the source port number, 0 if any: ") =='\0');
			if (atoi(buffer) >0)
				pAddr->set("port", buffer);

			pPref->addNextChild(pAddr);
			pAddr->free();

			bMoreAddr = (tolower(*readln(buffer, "More source to deny [Y/N]: ")) =='y');
		}

		pConversation->addNextChild(pPref);
		pPref->free();
	}

	prompt =isDefault ? "Configure default tunnel connections [Y/N]: "
		: "Configure private tunnel connections [Y/N]: ";
	if (tolower(*readln(buffer, prompt)) =='y')
	{
		// example: <Tunnels>
		//            <Connection remotePort="1000" >
		//                <LocalAddress address="192.168.80.8"/>
		//                <LocalAddress address="192.168.12.130"/>
		//                <RemoteAddress address="192.168.80.178"/>
		//                <RemoteAddress address="192.168.12.178"/>
		//            </Connection>
		//          </Tunnels>
		ZQ::common::IPreference* pPref = doc.newElement("Tunnels");

		bool bMoreConn  =true;

		do
		{
			ZQ::common::IPreference* pConn = doc.newElement("Connection");

			while (atoi(readln(buffer, "Enter the remote tunnel server port number: "))<=0);
			pConn->set("remotePort", buffer);

			bool bMoreAddr = true;
			int i = 0;
			while (bMoreAddr)
			{
				ZQ::common::IPreference* pAddr = doc.newElement("RemoteAddress");
				pAddr->set("address", readln(buffer, "Enter the remote address: "));

				pConn->addNextChild(pAddr);
				pAddr->free();
				if(++i>=2)
					break;

				bMoreAddr = (tolower(*readln(buffer, "More backup remote address for the connection [Y/N]: ")) =='y');
			}

			if (i>1)
				printf("[Warning] you must be sure both remote addresses are on the same machine\n");

			bMoreAddr = true;
			i = 0;
			while (bMoreAddr)
			{
				ZQ::common::IPreference* pAddr = doc.newElement("LocalAddress");
				pAddr->set("address", SelectLocalIP());

				pConn->addNextChild(pAddr);
				pAddr->free();
				if(++i>=2)
					break;

				bMoreAddr = (tolower(*readln(buffer, "More backup local address for the connection [Y/N]: ")) =='y');
			}

			pPref->addNextChild(pConn);
			pConn->free();

			bMoreConn = (tolower(*readln(buffer, "More default tunnel connection [Y/N]:")) =='y');

		} while (bMoreConn);

		pConversation->addNextChild(pPref);
		pPref->free();
	}

	return true;
}

const char* SelectLocalIP(const char* purpose, const bool bIPv6)
{
	InitLocalIPs();

	strs_t& addrstrs = LocalIPv4Addrs;
	if (bIPv6)
		addrstrs = LocalIPv6Addrs;

	int c = addrstrs.size();

	printf("local address:\n");
	for (int i =0; i < c; i ++)
		printf("%2d) %s\n", i+1, addrstrs[i].c_str());

	int choice = 0;
	printf("Select a local address %s: ", (purpose?purpose:""));

	while (choice<1 || choice>c)
	{
		char chs[128];
		choice = atoi(readln(chs));
	}

	return addrstrs[choice-1].c_str();
}

const char* readln(char* line, const char* prompt)
{
	if (prompt) printf(prompt);

#   define BLANKS "\t \r\n"
	if (line == NULL)
		return NULL;

	*line = '\0';
	for (char*p = line; *p!='\n' && *p!='\r'; p++)
	{
		*p = getchar();
		if (*p=='\r' || *p=='\n') p--;
	}

	*p=0x00;

	return line;
}

void InitLocalIPs()
{
	if (!bAddrStrsInited)
	{
		static ZQ::common::UDPSocket so;
		ZQ::common::InetHostAddress LocalAddresses
			= ZQ::common::InetHostAddress::getLocalAddress();

		for (int c = LocalAddresses.getAddressCount()-1; c >=0; c--)
		{
			std::string str = LocalAddresses.getHostAddress(c);

			switch(LocalAddresses.family(c))
			{
			case PF_INET:
				if (str.compare(0, sizeof("127.")-1, "127."))
					LocalIPv4Addrs.push_back(str);
				break;

			case PF_INET6:
				if (str.compare(0, sizeof("00:")-1, "00.") && str.compare(0, sizeof("::")-1, "::"))
					LocalIPv6Addrs.push_back(str);
				break;
			}
		}
		bAddrStrsInited = true;
	}
}
