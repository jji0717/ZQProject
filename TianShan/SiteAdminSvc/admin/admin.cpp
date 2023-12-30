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
// Ident : $Id: admin.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/SiteAdminSvc/admin/admin.cpp $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 6     08-01-22 11:56 Build
// 
// 5     08-01-22 11:47 Hui.shao
// 
// 4     07-12-13 18:26 Hui.shao
// 
// 3     07-12-13 17:02 Hui.shao
// 
// 2     07-12-10 18:53 Hui.shao
// 
// 12    07-04-19 17:50 Hongquan.zhang
// 
// 11    06-08-24 19:23 Hui.shao
// 
// 10    06-08-16 17:26 Hui.shao
// 
// 9     06-07-24 18:34 Hui.shao
// 
// 8     06-07-21 12:49 Hui.shao
// added command subscribe
// 
// 7     06-07-14 14:54 Hui.shao
// init impl on session record operations
// 
// 6     06-07-13 13:48 Hui.shao
// 
// 5     06-07-10 15:28 Hui.shao
// 
// 4     06-07-10 11:33 Hui.shao
// 
// 3     06-07-06 20:05 Hui.shao
// 
// 2     06-07-05 19:53 Hui.shao
// ===========================================================================

#include "../../common/TianShanDefines.h"
#include "admin.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

#ifdef YYDEBUG
extern int yydebug;
#endif

using namespace std;

extern FILE* yyin;

SAConsole::SAConsole(const char* endpointBind)
{
	int i =0;
	_communicator = Ice::initialize(i, NULL);

	printf("Opening local endpoint at: %s\n", endpointBind);
	_adapter = _communicator->createObjectAdapterWithEndpoints("sac", endpointBind);
//	_app = new DummyApp(*this);

	_adapter->activate();
}

void SAConsole::usage()
{
	printf("Usage: sac -b <bind endpoint> [-e <endpoint>] [-s <eventchannel endpoint>]\n");
	printf("       sac -h\n");
	printf("Site administrative tool.\n");
	printf("options:\n");
	printf("\t-e   the endpoint to connect to Weiwoo server\n");
	printf("\t-s   the endpoint to connect to Event Channel\n");
	printf("\t-b   the local endpoint to bind, where accept connections from others\n");
	printf("\t-h   display this help\n");
	printf("\nConsole commands:\n");
	printf("  help                       - display this help\n");
	printf("  connect <endpoint>         - connect a SiteAdmin server\n");
	printf("  subscribe <endpoint>       - subscribe SiteAdmin events from a IceStorm server\n");
	printf("  list {site|app}            - list all defined sites or applications\n");
	printf("  list txn {*|sitename} [*|app] - list all the transactions by site or applications\n");
	printf("  remove {site|app}          - list a definition of a site or application\n");
	printf("  add site <name> <desc>     - add a new site with given name and description\n");
	printf("  update site <name> <desc>  - update a site with given name and description\n");
	printf("  set site <name>            - set the properties of a site\n");
	printf("  show site <name>           - show a site and its application mount-ages\n");
	printf("  add app <name>             - add a new site with given name\n");
	printf("  update app <name>          - update a site with given name\n");
	printf("  mount <site> <path> <app>  - mount the application <app> onto <site>/<path>\n");
	printf("\n");
}

#define ASSET_CONNECTION(_PROXY) if (!_PROXY) \
{ error("no service has been currently connected, run 'connect' first"); return; }

#define SAC_TRY	try {
#define SAC_CATCH  } catch(const ::TianShanIce::BaseException& be) { error("caught %s: %s", be.ice_name().c_str(), be.message.c_str());} \
	            catch(const ::Ice::Exception& e) { error("caught %s", e.ice_name().c_str()); } \
				catch(...) { error("unknown exception caught");	}

const char* SAConsole::getPrompt()
{
    if (!_commands.empty() || !isatty(fileno(yyin)))
		return NULL;

    if(_continue)
    {
		_continue = false;
		return "(cont)> ";
    }
    else return "sac> ";
}

void SAConsole::connect(const Args& args)
{
    if(args.empty())
    {
		error("'connect' requires the endpoint to the Weiwoo service");
		return;
    }
	
	// build up the endpoint string
	std::string endpoint;
	for (Args::const_iterator it = args.begin(); it !=args.end(); it++)
		endpoint += *it + " ";
	
	SAC_TRY;
	_sasPrx = TianShanIce::Site::SiteAdminPrx::checkedCast(_communicator->stringToProxy(std::string(SERVICE_NAME_BusinessRouter ":") + endpoint));
	SAC_CATCH;
	
	if (!_sasPrx)
		error("failed to connect to BusinessAdmin at \"%s\"\n", endpoint);
	
//	SAC_TRY;
//	_sessPrx = TianShanIce::SRM::SessionAdminPrx::checkedCast(_communicator->stringToProxy(std::string(SERVICE_NAME_SessionManager ":") + endpoint));
//	SAC_CATCH;

//	if (!_sessPrx)
//		error("failed to connect to SessionAdmin at \"%s\"\n", endpoint.c_str());
}

void SAConsole::listSites()
{
	ASSET_CONNECTION(_sasPrx);

	SAC_TRY;

	::TianShanIce::Site::VirtualSites collection = _sasPrx->listSites();
	
	printf("Name                        Description\n");
	printf("--------------------------  ----------------------------------\n");
	for (::TianShanIce::Site::VirtualSites::iterator it = collection.begin(); it < collection.end(); it++)
		printf("%-26s  %s\n", it->name.c_str(), it->desc.c_str());
	printf("    %d site(s) found\n", collection.size());

	SAC_CATCH;
}

void SAConsole::updateSite(const Args& args)
{
	ASSET_CONNECTION(_sasPrx);
	
	SAC_TRY;
	Args::const_iterator it = args.begin();
	if (it < args.end())
		it ++;

	std::string desc;

	for (; it !=args.end(); it++)
		desc += *it + " ";

	if (desc.size() >0)
		desc = desc.substr(0, desc.size()-1);
	
	_sasPrx->updateSite(args[0], desc);
	_sasPrx->updateSiteResourceLimited(args[0],24000,6);
	SAC_CATCH;
}

void SAConsole::removeSite(const Args& args)
{
	ASSET_CONNECTION(_sasPrx);
	
	SAC_TRY;

	if (!_sasPrx->removeSite(args[0]))
		error("failed to remove the specified site");

	SAC_CATCH;
}

void SAConsole::showSite(const Args& args)
{
	ASSET_CONNECTION(_sasPrx);
	
	SAC_TRY;

	::TianShanIce::Site::VirtualSites collection = _sasPrx->listSites();
	::TianShanIce::Site::VirtualSites::iterator it;
	for (it = collection.begin(); it < collection.end(); it++)
	{
		if (args[0] == it->name)
			break;
	}

	if (it >= collection.end())
	{
		error("site \"%s\" not found\n", args[0].c_str());
		return;
	}

	printf("Site [%s] %s\n", it->name.c_str(), it->desc.c_str());
	printf("--------------------------------\n");

	int cProp =0, cApp=0;
	printf("Properties:\n");
	for (::TianShanIce::Properties::iterator pit = it->properties.begin(); pit != it->properties.end(); pit++, cProp++)
		printf("\t[%s] = \"%s\"\n", pit->first.c_str(), pit->second.c_str());
	
	printf("AppInfos:\n");
	::TianShanIce::Site::AppMounts mounts =  _sasPrx->listMounts(it->name);
	for (::TianShanIce::Site::AppMounts::iterator ait = mounts.begin(); ait != mounts.end(); ait++)
	{
		::TianShanIce::Site::AppMountPrx mount = *ait;
		if (!mount)
			continue;

		printf("\tpath[%s] -> app[%s]\n", mount->getMountedPath().c_str(), mount->getAppName().c_str());
		cApp++;
	}

	printf("  %d properties %d applications found on the site\n", cProp, cApp);

	SAC_CATCH;
}

void SAConsole::setSiteProp(const Args& siteName)
{
	ASSET_CONNECTION(_sasPrx);

	SAC_TRY;

	::TianShanIce::Properties props = _sasPrx->getSiteProperties(siteName[0]);
	
	char buf[256];
	std::string var;

	printf("Hint: empty property to quit entering, empty value to remove the property\n");

	while (true)
	{
		if (readInput(buf, sizeof(buf)-1, "property: ", true) <=0)
			break;
		var = buf;

		if (readInput(buf, sizeof(buf)-1, "value: ", true) <=0)
		{
			props.erase(var);
			continue;
		}

		props[var] = buf;
	}
	
	_sasPrx->setSiteProperties(siteName[0], props);

	SAC_CATCH;
}

// about applications
void SAConsole::listApps()
{
	ASSET_CONNECTION(_sasPrx);

	SAC_TRY;

	::TianShanIce::Site::AppInfos collection = _sasPrx->listApplications();
	
	printf("App       Endpoint                    Description\n");
	printf("--------  --------------------------  -------------------------\n");
	for (::TianShanIce::Site::AppInfos::iterator it = collection.begin(); it < collection.end(); it++)
		printf("%-8s  %-26s  %s\n", it->name.c_str(), it->endpoint.c_str(), it->desc.c_str());
	printf("    %d application(s) found\n", collection.size());

	SAC_CATCH;
}

void SAConsole::updateApp(const Args& args)
{
	ASSET_CONNECTION(_sasPrx);

	SAC_TRY;
	char buf[256];
	std::string endpoint, desc;

	readInput(buf, sizeof(buf)-1, "endpoint: ", false);
	endpoint = buf;

	readInput(buf, sizeof(buf)-1, "description: ");
	desc = buf;

	_sasPrx->updateApplication(args[0], endpoint, desc);

	SAC_CATCH;
}

void SAConsole::removeApp(const Args& args)
{
	ASSET_CONNECTION(_sasPrx);
	
	SAC_TRY;
	if (!_sasPrx->removeApplication(args[0]))
		error("failed to remove the specified application");
	SAC_CATCH;
}

// about mount
void SAConsole::mount(const Args& args)
{
	ASSET_CONNECTION(_sasPrx);
	string sitename = args[0], mountpath, appname;

	SAC_TRY;
	char buf[256];
	if (args.size()<2)
	{
		readInput(buf, sizeof(buf)-1, "mount path: ", false);
		mountpath = buf;
	}
	else mountpath = args[1];

	if (args.size()<3)
	{
		readInput(buf, sizeof(buf)-1, "application name: ", false);
		appname = buf;
	}
	else appname = args[2];

	if (!_sasPrx->mountApplication(sitename, mountpath, appname))
		error("failed to mount the application \"%s\" at path \"%s\" on site \"%s\"", appname.c_str(), mountpath.c_str(), sitename.c_str());
	SAC_CATCH;
}

void SAConsole::unmount(const Args& args)
{
	ASSET_CONNECTION(_sasPrx);
	string sitename = args[0], mountpath;

	SAC_TRY;
	char buf[256];
	if (args.size()<2)
	{
		readInput(buf, sizeof(buf)-1, "mount path: ", false);
		mountpath = buf;
	}
	else mountpath = args[1];

	if (!_sasPrx->unmountApplication(sitename, mountpath))
		error("failed to unmount the application at path \"%s\" on site \"%s\"", mountpath.c_str(), sitename.c_str());
	SAC_CATCH;
}

class SessionEventSinkI : public ::TianShanIce::SRM::SessionEventSink
{
public:
	SessionEventSinkI(SAConsole& console) : _console(console) {}

protected:

    virtual void OnNewSession(const ::std::string& sessId, const ::std::string& proxy, const ::Ice::Current& c)
    {
		char buf[1024];
		sprintf(buf, "OnNewSession() sessId=%s; proxy=%s", sessId.c_str(), proxy.c_str());
		_console.spoolLine(buf);
    }

	virtual void OnDestroySession(const ::std::string& sessId, const ::Ice::Current& c)
    {
		char buf[1024];
		sprintf(buf, "OnDestroySession() sessId=%s", sessId.c_str());
		_console.spoolLine(buf);
    }

    virtual void OnStateChanged(const ::std::string& sessId, const ::std::string& proxy, ::TianShanIce::State prevState, ::TianShanIce::State crntState, const ::Ice::Current& c)
	{
		char buf[1024];
		sprintf(buf, "OnStateChanged() sessId=%s; state %d->%d", sessId.c_str(), prevState, crntState);
		_console.spoolLine(buf);
	}

    virtual void ping(::Ice::Long timestamp, const ::Ice::Current& c)
	{
	}

	SAConsole& _console;
};

void SAConsole::subscribe(const Args& args)
{
#ifdef WITH_ICESTORM
    if(args.empty())
    {
		error("'subscribe' requires the endpoint to the IceStorm service");
		return;
    }
	
	// build up the endpoint string
	std::string endpoint;
	for (Args::const_iterator it = args.begin(); it !=args.end(); it++)
		endpoint += *it + " ";

	SAC_TRY;
	
	if (_sub)
		_sub = NULL;

	if (_adapter)
	{
		_sub = new TianShanIce::Events::EventChannelImpl(_adapter, endpoint.c_str(), true);
		::TianShanIce::SRM::SessionEventSinkPtr sink = new SessionEventSinkI(*this);
		::TianShanIce::Properties qos;
		_sub->sink(sink, qos);
	}

	enableSpool();
	
	SAC_CATCH;

	if (!_sub)
		error("failed to subscribe at \"%s\"\n", endpoint.c_str());
#endif // WITH_ICESTORM
}

void SAConsole::listTxn(const Args& args)
{
// 	["amd", "ami"] TxnInfos listLiveTxn(string siteName, string appMount, TianShanIce::StrValues paramNames)
// 		throws ServerError;

	ASSET_CONNECTION(_sasPrx);
	std::string bySite, byApp;
	
	SAC_TRY;
	if (args.size()>0)
	{
		bySite = args[0];
	}

	if (args.size()>1)
	{
		byApp = args[1];
	}

	::TianShanIce::StrValues params;
	params.push_back(SYS_PROP(state));
	params.push_back(SYS_PROP(siteName));
	params.push_back(SYS_PROP(path));
	params.push_back(SYS_PROP(state));

	params.push_back(SYS_PROP(ContentStore));
	params.push_back(SYS_PROP(Streamer));
	params.push_back(SYS_PROP(serviceGroupID));
	params.push_back(SYS_PROP(streamId));
	params.push_back(SYS_PROP(bandwidth));

	::TianShanIce::Site::TxnInfos txnInfos = _sasPrx->listLiveTxn(bySite, byApp, params);

#define RPT_FMT "%-24s %-10s %-10s %-16s %-20s %-20s %-6s %-10s\n"
#define RPT_FIELD(_F) it->params[_F].c_str()

	printf(RPT_FMT, "sessId", "siteName", "path", "state", "contentStore", "streamer", "svcGrp", "bandwidth", "streamId");
	printf(RPT_FMT, "======", "========", "====", "=====", "============", "========", "======", "=========", "========");

	for (::TianShanIce::Site::TxnInfos::iterator it = txnInfos.begin(); it < txnInfos.end(); it++)
	{
		printf(RPT_FMT, it->sessId.c_str(), RPT_FIELD(SYS_PROP(siteName)), RPT_FIELD(SYS_PROP(path)), RPT_FIELD(SYS_PROP(state)), 
			RPT_FIELD(SYS_PROP(ContentStore)), RPT_FIELD(SYS_PROP(Streamer)), RPT_FIELD(SYS_PROP(serviceGroupID)), 
			RPT_FIELD(SYS_PROP(bandwidth)), RPT_FIELD(SYS_PROP(streamId)));
	}

	printf("    %d live transaction(s) found\n", txnInfos.size());
	SAC_CATCH;
}

void SAConsole::trace(const Args& args)
{
	//	["ami", "amd"] void trace(string sessId, string category, string eventCode, string eventMsg)
	//		throws NoSuchTxn, ServerError;
	ASSET_CONNECTION(_sasPrx);
	
	SAC_TRY;
	if (args.size()<4)
	{
		error("wrong argument");
		return;
	}

	std::string sessId = args[0], category = args[1], eventCode = args[2], eventMsg = args[3];

	for (int i=4; i< args.size(); i++)
		eventMsg += std::string(" ") + args[i];
	
	_sasPrx->trace(sessId, category, eventCode, eventMsg);

	SAC_CATCH;
}

void SAConsole::dumpTxnXml(const Args& args)
{
	//["amd"] string dumpLiveTxn(string sessId, string beginFormat, string traceFormat, string endFormat)
	//	throws NoSuchTxn, ServerError;

	SAC_TRY;
	ASSET_CONNECTION(_sasPrx);
	string result =
	_sasPrx->dumpLiveTxn(args[0], "<txn sessId=\"$sys.sessId\" start=\"$sys.stampResolved\" end=\"$sys.stampStopped\">\n",
		                 "\t<event code=\"$event.category:$event.code\">$event.msg</event>\n",
	                     "</txn>\n");
	printf(result.c_str());

	SAC_CATCH;
}

//////////////////////////////////////////////////////////////////////////
// main entry
//////////////////////////////////////////////////////////////////////////

#include "getopt.h"

extern "C"
{
#include <time.h>
#include <stdio.h>
}

SAConsole gAdmin;

int main(int argc, char* argv[])
{
	int ch;
	while((ch = getopt(argc, argv, "he:s:b:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			gAdmin.usage();
			exit(0);
			
		case 'e':
			{
				Args args;
				args.push_back(optarg);
				gAdmin.connect(args);
			}
			break;
			
		case 's':
			{
				Args args;
				args.push_back(optarg);
				gAdmin.subscribe(args);
			}
			break;
			
		default:
			printf("Error: unknown option %c specified\n", ch);
			exit(1);
		}
	}
	
	gAdmin.parse(stdin, 1);
	return 0;
}


