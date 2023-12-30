// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: PathAdminConsole.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/admin/PathAdmin.cpp $
// 
// 3     1/12/16 9:08a Dejian.fei
// 
// 2     4/18/11 5:26p Fei.huang
// + migrated to linux
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 18    09-09-16 16:39 Xiaohui.chai
// 
// 17    08-12-22 15:50 Yixin.tian
// 
// 16    08-09-27 13:55 Yixin.tian
// 
// 15    08-03-28 17:35 Yixin.tian
// dummyprint function add a parameter because TianShanDefines has changed
// 
// 14    08-01-03 15:47 Hui.shao
// 
// 13    07-09-30 13:48 Yixin.tian
// 
// 12    07-09-30 12:11 Yixin.tian
// 
// 11    07-09-30 11:07 Hui.shao
// declared commands "dump" and "import" for TianYiXin to implement
// 
// 11    07-09-24 15:21 Hui.shao
// 
// 10    07-04-20 15:14 Hongquan.zhang
// 
// 9     07-01-15 10:09 Hongquan.zhang
// 
// 8     06-12-20 11:25 Hongquan.zhang
// 
// 7     06-12-14 14:32 Hui.shao
// 
// 6     10/16/06 3:29p Hui.shao
// 
// 5     9/21/06 4:36p Hui.shao
// batch checkin 20060921
// 
// 4     06-09-19 19:01 Hui.shao
// 
// 3     06-07-31 18:53 Hui.shao
// 
// 2     06-07-03 16:16 Hui.shao
// 
// 1     06-07-03 16:08 Hui.shao
// 
// 1     06-06-30 14:21 Hui.shao
// the console to admin paths
// ===========================================================================

#include "PathAdmin.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <algorithm>
#include "SystemUtils.h"

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

#ifdef YYDEBUG
extern int yydebug;
#endif

using namespace std;

extern FILE* yyin;

// PathAdminConsole* gPathAdmin = NULL;

PathAdminConsole::PathAdminConsole()
{
	int i =0;
	_ic = Ice::initialize(i, NULL);
}

void PathAdminConsole::usage()
{
    cout <<
        "help                                        Print this message.\n"
        "exit, quit                                  Exit this program.\n"
        "connect <endpoint to PathManager>           connect to the PathManager.\n"
        "list {servicegroup|storage|streamer|ticket} List the objects\n"
        "update {servicegroup|storage|streamer} <id> Update the object\n"
        "remove {servicegroup|storage|streamer} <id> Remove the object\n"
        "link {storage|streamer}                     Link the objects\n"
        "relink {storage|streamer} <StreamerNetId> <LinkId>  Modify or unlink the link.\n"
        "show streamer <netId>                       Display the details of the objects\n"
        "dump <filename to write>                    Export the settings into an external XML file\n"
        //        "import [-o] <filename to read>              Import the settings from an external XML file\n"
        //        "                                            -o if need to cleanup and overwrite the current\n";
        "\n";
}

#define ASSET_CONNECTION if (!_adminPrx) \
{ error("no PathManager has been currently connected, run 'connect' first"); return; }

#define TRY 	try	{
#define CATCH 	} catch(const ::TianShanIce::BaseException& e) { error("!exception %s: %s", e.ice_name().c_str(), e.message.c_str()); } \
				catch(const ::Ice::Exception& e) { error("!exception %s", e.ice_name().c_str()); }


void PathAdminConsole::connect(const Args& args)
{
    if(args.empty())
    {
		error("'connect' requires the endpoint to the PathManager");
		return;
    }
	
	// build up the endpoint string
	std::string endpoint = "PathManager:";
	for (Args::const_iterator it = args.begin(); it !=args.end(); it++)
		endpoint += *it + " ";
	
	TRY;
	_adminPrx = TianShanIce::Transport::PathAdminPrx::checkedCast(_ic->stringToProxy(endpoint));
	CATCH;
	
	if (!_adminPrx)
		error("failed to connect to %s\n", endpoint.c_str());
}

void PathAdminConsole::listServiceGroups()
{
	ASSET_CONNECTION;
	
	TRY;
	
	::TianShanIce::Transport::ServiceGroups sgs = _adminPrx->listServiceGroups();
	
	printf("ID    Description\n");
	printf("----  ----------------------\n");
	for (::TianShanIce::Transport::ServiceGroups::iterator it = sgs.begin(); it < sgs.end(); it++)
		printf("%4d  %s\n", it->id, it->desc.c_str());
	printf("    %lu ServiceGroup(s) found\n", sgs.size());

	CATCH;
}

void PathAdminConsole::listStorages()
{
	ASSET_CONNECTION;
	
	TRY;

	::TianShanIce::Transport::Storages collection = _adminPrx->listStorages();
	
	printf("NetId       Type           Endpoint                        Description\n");
	printf("----------  -------------  ------------------------------  -------------------\n");
	for (::TianShanIce::Transport::Storages::iterator it = collection.begin(); it < collection.end(); it++)
		printf("%-10s  %-13s  %-30s  %s\n", it->netId.c_str(), it->type.c_str(), it->ifep.c_str(), it->desc.c_str());
	printf("    %lu Storage(s) found\n", collection.size());

	CATCH;
}

void PathAdminConsole::listStreamers()
{
	ASSET_CONNECTION;
	
	TRY;

	::TianShanIce::Transport::Streamers collection = _adminPrx->listStreamers();
	
	printf("NetId       Type           Endpoint                        Description\n");
	printf("----------  -------------  ------------------------------  -------------------\n");
	for (::TianShanIce::Transport::Streamers::iterator it = collection.begin(); it < collection.end(); it++)
		printf("%-10s  %-13s  %-30s  %s\n", it->netId.c_str(), it->type.c_str(), it->ifep.c_str(), it->desc.c_str());
	printf("    %lu Streamer(s) found\n", collection.size());

	CATCH;
}

void PathAdminConsole::listTickets()
{
	ASSET_CONNECTION;
	
	TRY;

	::TianShanIce::Transport::PathTickets collection = _adminPrx->listTickets();
	
	printf("Ticket                               State Cost  Lease/ms Streamer\n");
	printf("------------------------------------ ----- ----- -------- -----------\n");
	for (::TianShanIce::Transport::PathTickets::iterator it = collection.begin(); it < collection.end(); it++)
	{
		std::string streamerId = "n/a";
		try {
			streamerId = (*it)->getStreamLink()->getStreamerId();
		}
		catch(...) {}
	
//		printf("%36s  %-14s  %5d  %s\n", (*it)->getIdent().name.c_str(), ::ZQTianShan::ObjStateStr((*it)->getState()), (*it)->getCost(), streamerId.c_str());
		printf("%36s %5d %5d %8d %s\n", (*it)->getIdent().name.c_str(), (*it)->getState(), (*it)->getCost(), (*it)->getLeaseLeft(), streamerId.c_str());
	}
	printf("    %lu Ticket(s) found\n", collection.size());

	CATCH;
}

void PathAdminConsole::listStorageLinks()
{
/*
ASSET_CONNECTION;

  ::TianShanIce::Transport::ServiceGroups sgs = _adminPrx->listServiceGroups();
  
	printf("GroupID    Description\n");
	for (::TianShanIce::Transport::ServiceGroups::iterator it = sgs.begin(); it < sgs.end(); it++)
	{
	printf("%8d   %s\n", it->id, it->desc.c_str());
	}
	printf("    %d ServiceGroup(s) found\n", sgs.size());
	*/
}

void PathAdminConsole::listStreamLinks()
{
/*
ASSET_CONNECTION;

  ::TianShanIce::Transport::ServiceGroups sgs = _adminPrx->listServiceGroups();
  
	printf("GroupID    Description\n");
	for (::TianShanIce::Transport::ServiceGroups::iterator it = sgs.begin(); it < sgs.end(); it++)
	{
	printf("%8d   %s\n", it->id, it->desc.c_str());
	}
	printf("    %d ServiceGroup(s) found\n", sgs.size());
	*/
}


void PathAdminConsole::updateServiceGroup(const Args& args)
{
	ASSET_CONNECTION;
	
	char buf[256] = "";
	std::string desc;

	readInput(buf, sizeof(buf)-1, "desc: ", true);
	desc = buf;

	TRY;

	_adminPrx->updateServiceGroup(atoi(args[0].c_str()), desc);

	CATCH;
}

void PathAdminConsole::updateStorage(const Args& args)
{
	ASSET_CONNECTION;
	
	char buf[256] = "";

	readInput(buf, sizeof(buf)-1, "proxy: ", false);
	std::string proxy = buf;

	readInput(buf, sizeof(buf)-1, "type:[content] ", false);
	std::string type = buf;

	readInput(buf, sizeof(buf)-1, "desc: ", false);
	std::string desc = buf;

	TRY;

	_adminPrx->updateStorage(args[0], type, proxy, desc);

	CATCH;
}

void PathAdminConsole::updateStreamer(const Args& args)
{
	ASSET_CONNECTION;
	
	char buf[256] = "";

	readInput(buf, sizeof(buf)-1, "proxy: ", false);
	std::string proxy = buf;

	readInput(buf, sizeof(buf)-1, "type:[SeaChange.IpEdge] ", false);
	std::string type = buf;

	readInput(buf, sizeof(buf)-1, "desc: ", false);
	std::string desc = buf;

	TRY;
		_adminPrx->updateStreamer(args[0], type, proxy, desc);
	CATCH;
}

void PathAdminConsole::removeServiceGroup(const Args& args)
{
	TRY;
	_adminPrx->removeServiceGroup(atoi(args[0].c_str()));
	CATCH;
}

void PathAdminConsole::removeStorage(const Args& args)
{
	TRY;
	_adminPrx->removeStorage(args[0]);
	CATCH;
}

void PathAdminConsole::removeStreamer(const Args& args)
{
	TRY;
	_adminPrx->removeStreamer(args[0]);
	CATCH;
}

void PathAdminConsole::linkStorage(void)
{
	char buf[256] = "";

	TRY;
	::TianShanIce::StrValues types = _adminPrx->listSupportedStorageLinkTypes();
	printf("Supported types: ");
	for (::TianShanIce::StrValues::iterator it2= types.begin(); it2<types.end(); it2++)
		printf("%s; ", it2->c_str());
	printf("\n");

	readInput(buf, sizeof(buf)-1, "StorageLink type: ", false);
	std::string type = buf;

	::TianShanIce::PDSchema schema = _adminPrx->getStorageLinkSchema(type);

	readInput(buf, sizeof(buf)-1, "Storage NetId: ", false);
	std::string storeNetId = buf;
	readInput(buf, sizeof(buf)-1, "Streamer NetId: ", false);
	std::string streamerNetId = buf;

	::TianShanIce::ValueMap linkPd;

	for (::TianShanIce::PDSchema::iterator it = schema.begin(); it < schema.end(); it++)
	{
		//if (readInput(buf, sizeof(buf)-1, (it->keyname + ": ").c_str(), it->optional) <=0 )
		if (readInput(buf, sizeof(buf)-1, (it->keyname + ": ").c_str(), it->optional2) <=0 )
		{
			linkPd[it->keyname] = it->defaultvalue;
			continue;
		}

		::TianShanIce::Variant  val;
		val.bRange = false;
		val.type   = it->defaultvalue.type;

		switch(val.type)
		{
			case ::TianShanIce::vtInts:		val.ints.push_back(atoi(buf)); break;
			case ::TianShanIce::vtLongs:	val.lints.push_back(atol(buf)); break;
			case ::TianShanIce::vtStrings:	val.strs.push_back(buf); break;
			default: break;
		}

		linkPd[it->keyname] = val;
	}

	::TianShanIce::Transport::StorageLinkPrx link = _adminPrx->linkStorage(storeNetId, streamerNetId, type, linkPd);
	printf("StorageLink[%s] added\n", link->getIdent().name.c_str());

	CATCH;
}

void AddNumber()
{
	
}
bool CheckPrivateData(const std::string strKey,int type, char *psrc,::TianShanIce::Variant *pvar);
bool GetPrivateDataStr(char *dest,::TianShanIce::Variant *pvar);
char* Delblank(char *psrc);
void PathAdminConsole::linkStreamer(void)
{
	char buf[256] = "";

	TRY;
	::TianShanIce::StrValues types = _adminPrx->listSupportedStreamLinkTypes();
	printf("Supported types: ");
	for (::TianShanIce::StrValues::iterator it2= types.begin(); it2<types.end(); it2++)
		printf("%s; ", it2->c_str());
	printf("\n");

	readInput(buf, sizeof(buf)-1, "StreamLink type: ", false);
	std::string type = buf;

	::TianShanIce::PDSchema schema = _adminPrx->getStreamLinkSchema(type);

	readInput(buf, sizeof(buf)-1, "Streamer NetId: ", false);
	std::string streamerNetId = buf;
	readInput(buf, sizeof(buf)-1, "ServiceGroup Id: ", false);
	int svcgrpId = atoi(buf);

	::TianShanIce::ValueMap linkPd;

	for (::TianShanIce::PDSchema::iterator it = schema.begin(); it < schema.end(); it++)
	{
		//if (readInput(buf, sizeof(buf)-1, (it->keyname + ": ").c_str(), it->optional) <=0 )
		if (readInput(buf, sizeof(buf)-1, (it->keyname + ": ").c_str(), it->optional2) <=0 )
		{
			linkPd[it->keyname] = it->defaultvalue;
			continue;
		}

		::TianShanIce::Variant  val;
		val.bRange = false;
		val.type   = it->defaultvalue.type;
		
		switch(val.type)
		{
			case ::TianShanIce::vtInts:		CheckPrivateData("",val.type,buf,&val); break;
			case ::TianShanIce::vtLongs:	CheckPrivateData("",val.type,buf,&val); break;
			case ::TianShanIce::vtStrings:	val.strs.push_back(buf); break;
			default: break;
		}

		linkPd[it->keyname] = val;
	}

	::TianShanIce::Transport::StreamLinkPrx link = _adminPrx->linkStreamer(svcgrpId, streamerNetId, type, linkPd);
	printf("StreamLink[%s] added\n", link->getIdent().name.c_str());

//	_adminPrx->listStreamLinksByStreamer()

	CATCH;
}

void PathAdminConsole::relinkStorage(const Args& args)
{
	ASSET_CONNECTION;

    if(args.size() != 2)
    {
        error("Must provide streamer netid and linkid.");
        return;
    }
	
	TRY;

	std::string ident = args[1];
	std::transform(ident.begin(), ident.end(), ident.begin(), ::tolower);
	
	::TianShanIce::Transport::StorageLinks collection = _adminPrx->listStorageLinksByStreamer(args[0]);
	::TianShanIce::Transport::StorageLinks::iterator lit;

	for (lit = collection.begin(); lit < collection.end(); lit++)
	{
		try {
			std::string linkIdent = (*lit)->getIdent().name;
			std::transform(linkIdent.begin(), linkIdent.end(), linkIdent.begin(), ::tolower);
			
			if (0 == linkIdent.compare(ident))
				break;
		} catch (...) {};
	}

	if (collection.end() == lit)
	{
		error("failed to find storage link [%s]", ident.c_str());
		return;
	}

	char buf[256] = "";
	::TianShanIce::Transport::StorageLinkExPrx link = ::TianShanIce::Transport::StorageLinkExPrx::checkedCast(*lit);
	readInput(buf, sizeof(buf)-1, "Destroy or Modify? [D] ", true);
	if ('D' == buf[0] || 'd' == buf[0])
	{
		link->destroy();
		printf("StorageLink[%s] destroyed\n", ident.c_str());
		return;
	}

	::TianShanIce::PDSchema schema = _adminPrx->getStorageLinkSchema(link->getType());
	::TianShanIce::ValueMap linkPd;

	for (::TianShanIce::PDSchema::iterator it = schema.begin(); it < schema.end(); it++)
	{
		//if (readInput(buf, sizeof(buf)-1, (it->keyname + ": ").c_str(), it->optional) <=0 )
		if (readInput(buf, sizeof(buf)-1, (it->keyname + ": ").c_str(), it->optional2) <=0 )
		{
			linkPd[it->keyname] = it->defaultvalue;
			continue;
		}

		::TianShanIce::Variant  val;
		val.bRange = false;
		val.type   = it->defaultvalue.type;
		
		switch(val.type)
		{
        case ::TianShanIce::vtInts:		CheckPrivateData("",val.type,buf,&val); break;
        case ::TianShanIce::vtLongs:	CheckPrivateData("",val.type,buf,&val); break;
        case ::TianShanIce::vtStrings:	val.strs.push_back(buf); break;
	default: break;
		}

		linkPd[it->keyname] = val;
	}

	link->updatePrivateData(linkPd);
	printf("StorageLink[%s] modified\n", ident.c_str());
	
	CATCH;
}


void PathAdminConsole::relinkStreamer(const Args& args)
{
	ASSET_CONNECTION;

    if(args.size() != 2)
    {
        error("Must provide streamer netid and linkid.");
        return;
    }
	
	TRY;

	std::string ident = args[1];
	std::transform(ident.begin(), ident.end(), ident.begin(), ::tolower);
	
	::TianShanIce::Transport::StreamLinks collection = _adminPrx->listStreamLinksByStreamer(args[0]);
	::TianShanIce::Transport::StreamLinks::iterator lit;

	for (lit = collection.begin(); lit < collection.end(); lit++)
	{
		try {
			std::string linkIdent = (*lit)->getIdent().name;
			std::transform(linkIdent.begin(), linkIdent.end(), linkIdent.begin(), ::tolower);
			
			if (0 == linkIdent.compare(ident))
				break;
		} catch (...) {};
	}

	if (collection.end() == lit)
	{
		error("failed to find stream link [%s]", ident.c_str());
		return;
	}

	char buf[256] = "";
	::TianShanIce::Transport::StreamLinkExPrx link = ::TianShanIce::Transport::StreamLinkExPrx::checkedCast(*lit);
	readInput(buf, sizeof(buf)-1, "Destroy or Modify? [D] ", true);
	if ('D' == buf[0] || 'd' == buf[0])
	{
		link->destroy();
		printf("StreamLink[%s] destroyed\n", ident.c_str());
		return;
	}

	::TianShanIce::PDSchema schema = _adminPrx->getStreamLinkSchema(link->getType());
	::TianShanIce::ValueMap linkPd;

	for (::TianShanIce::PDSchema::iterator it = schema.begin(); it < schema.end(); it++)
	{
		//if (readInput(buf, sizeof(buf)-1, (it->keyname + ": ").c_str(), it->optional) <=0 )
		if (readInput(buf, sizeof(buf)-1, (it->keyname + ": ").c_str(), it->optional2) <=0 )
		{
			linkPd[it->keyname] = it->defaultvalue;
			continue;
		}

		::TianShanIce::Variant  val;
		val.bRange = false;
		val.type   = it->defaultvalue.type;
		
		switch(val.type)
		{
			case ::TianShanIce::vtInts:		CheckPrivateData("",val.type,buf,&val); break;
			case ::TianShanIce::vtLongs:	CheckPrivateData("",val.type,buf,&val); break;
			case ::TianShanIce::vtStrings:	val.strs.push_back(buf); break;
			default: break;
		}

		linkPd[it->keyname] = val;
	}

	link->updatePrivateData(linkPd);
	printf("StreamLink[%s] modified\n", ident.c_str());
	
	CATCH;
}

static void dummyprint(const char* line,void* pVoid = NULL)
{
	printf("%s\n", line);
}

void PathAdminConsole::showStreamer(const Args& args)
{
	TRY;
	::TianShanIce::Transport::Streamers streamers = _adminPrx->listStreamers();
	::TianShanIce::Transport::Streamer theStreamer;

	::TianShanIce::Transport::Streamers::iterator strmIt;
	for (strmIt = streamers.begin(); strmIt < streamers.end(); strmIt ++)
	{
		if (0 == args[0].compare(strmIt->netId))
		{
			theStreamer = *strmIt;
			break;
		}
	}

	if (strmIt >= streamers.end())
	{
		error("failed to find streamer[%s]", args[0].c_str());
		return;
	}

	streamers.clear();

	printf("[Streamer]: NetId: %s;\n      Type: %s;\n      Proxy: \"%s\";\n      Desc: %s\n", theStreamer.netId.c_str(), theStreamer.type.c_str(), theStreamer.ifep.c_str(), theStreamer.desc.c_str());
	ZQTianShan::dumpValueMap(theStreamer.privateData, "           ", dummyprint);
	
	{
		::TianShanIce::Transport::StorageLinks links = _adminPrx->listStorageLinksByStreamer(theStreamer.netId);
		printf("  +StorageLinks (%lu):\n", links.size());
		for (::TianShanIce::Transport::StorageLinks::iterator it = links.begin(); it < links.end(); it++)
		{
			::TianShanIce::Transport::StorageLinkPrx link = *it;
			printf("     [%s]\n        Type: %s;\n        Storage: %s\n", link->getIdent().name.c_str(), link->getType().c_str(), link->getStorageId().c_str());
			TianShanIce::ValueMap linkPd = link->getPrivateData();
			ZQTianShan::dumpValueMap(linkPd, "        ", dummyprint);
		}
	}

	{
		::TianShanIce::Transport::StreamLinks links = _adminPrx->listStreamLinksByStreamer(theStreamer.netId);
		printf("  +StreamLinks (%lu):\n", links.size());
		for (::TianShanIce::Transport::StreamLinks::iterator it = links.begin(); it < links.end(); it++)
		{
			::TianShanIce::Transport::StreamLinkPrx link = *it;
			printf("     [%s]\n        Type: %s;\n        ServiceGroup: %d\n", link->getIdent().name.c_str(), link->getType().c_str(), link->getServiceGroupId());
			TianShanIce::ValueMap linkPd = link->getPrivateData();
			ZQTianShan::dumpValueMap(linkPd, "        ", dummyprint);
		}
	}

	CATCH;
}

void PathAdminConsole::dump(const Args& args)
{
	TRY;
	std::string strXml = _adminPrx->dumpXml();
	FILE* f = fopen(args[0].c_str(), "w");
	if(f)
	{
		size_t bytes = fwrite(strXml.c_str(), 1, strXml.length(), f);
		if(bytes <= strXml.length() && ferror(f))
		{
			printf("Dump data failure,WriteFile() function not succeed,error code is %d\n",SYS::getLastErr());
		}
		fclose(f);
	}
	else
		printf("dump date to %s failure\n",args[0].c_str());

	CATCH;

}

void PathAdminConsole::import(const Args& args)
{
	TRY;
	if(args[0].compare("-o") == 0 && args.size() > 1)
	{
		_adminPrx->importXml(args[1],true);
	}
	else
		_adminPrx->importXml(args[0],false);

	CATCH;
}


const char* PathAdminConsole::getPrompt()
{
    if (!_commands.empty() || !isatty(fileno(yyin)))
		return NULL;

    if(_continue)
    {
		_continue = false;
		return "(cont)> ";
    }
    else return "Path> ";
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool GetPrivateDataStr(char *dest,::TianShanIce::Variant *pvar)
{
	std::string strtemp = "";
	char temp[20];
	bool flag = false;
    switch(pvar->type)
	{
	case ::TianShanIce::vtStrings:
		{
			::TianShanIce::StrValues::iterator strpos = pvar->strs.begin();
			while(strpos != pvar->strs.end())
			{
				if(flag)
				{
					strtemp += ";";
				}
				strtemp += *strpos; 	
				strpos++;
				flag = true;
			}
			strcpy(dest,strtemp.c_str());
			return true;
		}
	case ::TianShanIce::vtLongs:
		{
			::TianShanIce::LValues::iterator Lpos = pvar->lints.begin();
			if(pvar->bRange == false)
			{
				while(Lpos != pvar->lints.end())
				{
					if(flag)
					{
						strtemp += ";";
						
					}
					strtemp += itoa(*Lpos, temp, 10); 
					Lpos++;
					flag = true;
				}
			}
			else
			{
				if(Lpos != pvar->lints.end())
				{
					strtemp += itoa(*Lpos, temp, 10);
					Lpos++;
					strtemp += "  ~  ";
					strtemp += itoa(*Lpos, temp, 10);
				}
				else 
					strtemp = "";
			
			}
			strcpy(dest,strtemp.c_str());
			return true;
		}
	case ::TianShanIce::vtInts:
		{			
			::TianShanIce::IValues::iterator Ipos = pvar->ints.begin();
			if(pvar->bRange == false)
			{
				while(Ipos != pvar->ints.end())
				{
					if(flag)
					{
						strtemp += ";";
					}
					strtemp += itoa(*Ipos, temp, 10); 
					Ipos++;
					flag = true;
				}
			}
			else
			{
				if(Ipos != pvar->ints.end())
				{
				strtemp += itoa(*Ipos, temp, 10);
				Ipos++;
				strtemp += "  ~  ";
				strtemp += itoa(*Ipos, temp, 10);
				}
				else 
					strtemp = "";
			}
			strcpy(dest,strtemp.c_str());
			return true;
		}
	case ::TianShanIce::vtBin:
	default:
		return false;
	}
   return false;
}
///Delete all of front Blank and end Blank in a string
///@parme[in] pstr the string which is delete blank
///@return    if the string only contain Blank or NULL ,return NULL;
///              else return Delete Blank String
char* Delblank(char *psrc)//去掉一个字符串前后的所有空格。
{
	char* pfirst, *plast;
	if(*psrc == '\0')
		return NULL;
	plast = psrc + strlen(psrc) - 1;
	
	while(*psrc == ' '&& *psrc != '\0')
	{
		psrc++;
	}
	while(*plast ==' ')
	{
		plast--;
	}
	pfirst = psrc;
	*(plast + 1) = '\0';
	
	if(*pfirst == '\0')
		return NULL;
	
	return pfirst;
	
}

///Judges a character string is not the pure digital string
///@parme psrc the Judges character string
///@return  if the string is pure digital string ,return TRUE;
///          else return false
bool IsInt(char *psrc)//判断一个字符串是不是纯数字串。
{
	int i = strlen(psrc);
	if(i >= 2 && *psrc  == '0' )
		return false;
	while(*psrc >= '0' && *psrc <= '9')
	{
		psrc++;
	}
	
	if(*psrc != '\0')
		return false;
	
	return true;
}

///judges a string is  only contain digital 、blank and specify character
///@parme[in] ch specify character
///@parme[in] psrc the judges string
///@return  if the string only contain digital blank and specify character, return TRUE
///            else  return false
bool IsString(char ch, char *psrc)//判断一个字符串中是否只包含了数字空格和指定的字符。
{
	while(*psrc >= '0' && *psrc <= '9' || *psrc == ' ' || *psrc == ch)
	{
		psrc++;
	}
	
	if(*psrc != '\0')
		return false;
	
	return true;
}

///judges a string is contain double ';' or the first character is ';'
///@parme[in] psrc the judges string;
///@return  if the string contain double';' or the first character is';',return false;
///         else return TRUE
bool DoubleChar(char *psrc)
{
	char *pstr = psrc;
	int i = strlen(psrc);
	
	if(i == 1 && *psrc == ';')
		return false;
	
	for(; *pstr != '\0'; pstr++)
	{
		if(*pstr == *(pstr +1))
		{
			if(*pstr == ';')
				return false;
		}
	}
	return true;
}

///Check the string  is effect or not
///@param[in] strKey   VariantMap key
///@param[in]type  Variant Type
///@param[in] psrc the string is checked 
///@param[out] pvar receive the Variant
///@return	if the string is effect, return TRUE
///          else return false
bool CheckPrivateData(const std::string strKey,int type, char *psrc,::TianShanIce::Variant *pvar)
{
	char sepStr[] = ";";
	char sepIntTrue[] = "~";
	char *token, *pstr, *ptemp;
	switch(type)
	{
	case ::TianShanIce::vtStrings:
		pvar->type = TianShanIce::vtStrings;
		pvar->bRange = false;
		
		if(!DoubleChar(psrc))
			return false;
		
		if(*psrc == ';')
			return false;
		token = strtok(psrc,sepStr );
		while( token != NULL)
		{
			pvar->strs.push_back(token);
			token = strtok(NULL, sepStr);
		}
		break;
	case ::TianShanIce::vtLongs:
		pvar->type = TianShanIce::vtLongs;
		if(IsString(';',psrc))//判断是不是只有数字空格和';'
		{
			pvar->bRange = false;
			if(!DoubleChar(psrc))
				return false;
			if(*psrc == ';')
				return false;
			
			token = strtok(psrc,sepStr );
			while( token != NULL)
			{
				pstr =  Delblank(token);// " ; ;"
				
				if(!pstr)
					return false;
				
				if(!IsInt(pstr))
					return false;
				pvar->lints.push_back(atoi(pstr));
				token = strtok(NULL,sepStr );
			}
		}
		else 
			if(IsString('~',psrc))//判断是不是只有数字空格和'-'
			{
				pvar->bRange = true;
				if(*psrc == '-')
					return false;
				
				token = strtok(psrc,sepIntTrue );
				
				while( token != NULL)
				{
					pstr =  Delblank(token);
					
					if(!pstr)
						return false;
					
					if(!IsInt(pstr))
						return false;
					pvar->ints.push_back(atoi(pstr));
					token = strtok(NULL,sepIntTrue );
				}
			}
			
			else
				return false;
			break;
	case ::TianShanIce::vtInts:
		
		pvar->type = TianShanIce::vtInts;
		
		if(IsString(';',psrc))//判断是不是只有数字空格和';'
		{
			pvar->bRange = false;
			if(!DoubleChar(psrc))
				return false;
			if(*psrc == ';')
				return false;
			
			token = strtok(psrc,sepStr );
			while( token != NULL)
			{
				pstr =  Delblank(token);// " ; ;"
				
				if(!pstr)
					return false;
				
				if(!IsInt(pstr))
					return false;
				pvar->ints.push_back(atoi(pstr));
				token = strtok(NULL,sepStr );
			}
		}
		else 
			if(IsString('~',psrc))//判断是不是只有数字空格和'-'
			{
				int cout = 0;
				pvar->bRange = true;
                for(ptemp = psrc; *ptemp != '\0'; ptemp++)
				{
					if(*ptemp == '~')
						cout++;
				}
				if(cout != 1)
					return false;
				if(*psrc == '~')
					return false;
				
				token = strtok(psrc,sepIntTrue );
				
				while( token != NULL)
				{
					pstr =  Delblank(token);
					
					if(!pstr)
						return false;
					
					if(!IsInt(pstr))
						return false;
					pvar->ints.push_back(atoi(pstr));
					token = strtok(NULL,sepIntTrue );
				}
			}
			
			else
				return false;
			break;
	case ::TianShanIce::vtBin:
		return false;
	default:
		break;
 }
 return true;
}
