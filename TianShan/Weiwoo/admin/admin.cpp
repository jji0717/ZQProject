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
// $Log: /ZQProjs/TianShan/Weiwoo/admin/admin.cpp $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
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

WeiwooAdminConsole::WeiwooAdminConsole(const char* endpointBind)
{
	int i =0;
	_communicator = Ice::initialize(i, NULL);

	printf("Opening local endpoint at: %s\n", endpointBind);
	_adapter = _communicator->createObjectAdapterWithEndpoints("WeiwooAdmin", endpointBind);
//	_app = new DummyApp(*this);

	_adapter->activate();
}

void WeiwooAdminConsole::usage()
{
	printf("Usage: WeiwooAdmin -b <bind endpoint> [-e <endpoint>] [-s <eventchannel endpoint>]\n");
	printf("       WeiwooAdmin -h\n");
	printf("Weiwoo administrative tool.\n");
	printf("options:\n");
	printf("\t-e   the endpoint to connect to Weiwoo server\n");
	printf("\t-s   the endpoint to connect to Event Channel\n");
	printf("\t-b   the local endpoint to bind, where accept connections from others\n");
	printf("\t-h   display this help\n");
	printf("\nConsole commands:\n");
	printf("  help                       - display this help\n");
	printf("  connect <endpoint>         - connect a Weiwoo server\n");
	printf("  subscribe <endpoint>       - subscribe Weiwoo events from a IceStorm server\n");
	printf("  list {site|app}            - list all defined sites or applications\n");
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

#define WEIWOOADMIN_TRY	try {
#define WEIWOOADMIN_CATCH  } catch(const ::TianShanIce::BaseException& be) {error("::TianShanIce::Exception(): %s", be.message.c_str());} \
				catch(...) { error("unknown exception caught");	}

const char* WeiwooAdminConsole::getPrompt()
{
    if (!_commands.empty() || !isatty(fileno(yyin)))
		return NULL;

    if(_continue)
    {
		_continue = false;
		return "(cont)> ";
    }
    else return "Weiwoo> ";
}

void WeiwooAdminConsole::connect(const Args& args)
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
	
	WEIWOOADMIN_TRY;
	_bizPrx = TianShanIce::Site::SiteAdminPrx::checkedCast(_communicator->stringToProxy(std::string(SERVICE_NAME_BusinessRouter ":") + endpoint));
	WEIWOOADMIN_CATCH;
	
	if (!_bizPrx)
		error("failed to connect to BusinessAdmin at \"%s\"\n", endpoint);
	
//	WEIWOOADMIN_TRY;
//	_sessPrx = TianShanIce::SRM::SessionAdminPrx::checkedCast(_communicator->stringToProxy(std::string(SERVICE_NAME_SessionManager ":") + endpoint));
//	WEIWOOADMIN_CATCH;

//	if (!_sessPrx)
//		error("failed to connect to SessionAdmin at \"%s\"\n", endpoint.c_str());
}

void WeiwooAdminConsole::listSites()
{
	ASSET_CONNECTION(_bizPrx);

	WEIWOOADMIN_TRY;

	::TianShanIce::Site::VirtualSites collection = _bizPrx->listSites();
	
	printf("Name                        Description\n");
	printf("--------------------------  ----------------------------------\n");
	for (::TianShanIce::Site::VirtualSites::iterator it = collection.begin(); it < collection.end(); it++)
		printf("%-26s  %s\n", it->name.c_str(), it->desc.c_str());
	printf("    %d site(s) found\n", collection.size());

	WEIWOOADMIN_CATCH;
}

void WeiwooAdminConsole::updateSite(const Args& args)
{
	ASSET_CONNECTION(_bizPrx);
	
	WEIWOOADMIN_TRY;
	Args::const_iterator it = args.begin();
	if (it < args.end())
		it ++;

	std::string desc;

	for (; it !=args.end(); it++)
		desc += *it + " ";

	if (desc.size() >0)
		desc = desc.substr(0, desc.size()-1);
	
	_bizPrx->updateSite(args[0], desc);
	_bizPrx->updateSiteResourceLimited(args[0],24000,6);
	WEIWOOADMIN_CATCH;
}

void WeiwooAdminConsole::removeSite(const Args& args)
{
	ASSET_CONNECTION(_bizPrx);
	
	WEIWOOADMIN_TRY;

	if (!_bizPrx->removeSite(args[0]))
		error("failed to remove the specified site");

	WEIWOOADMIN_CATCH;
}

void WeiwooAdminConsole::showSite(const Args& args)
{
	ASSET_CONNECTION(_bizPrx);
	
	WEIWOOADMIN_TRY;

	::TianShanIce::Site::VirtualSites collection = _bizPrx->listSites();
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
	::TianShanIce::Site::AppMounts mounts =  _bizPrx->listMounts(it->name);
	for (::TianShanIce::Site::AppMounts::iterator ait = mounts.begin(); ait != mounts.end(); ait++)
	{
		::TianShanIce::Site::AppMountPrx mount = *ait;
		if (!mount)
			continue;

		printf("\tpath[%s] -> app[%s]\n", mount->getMountedPath().c_str(), mount->getAppName().c_str());
		cApp++;
	}

	printf("  %d properties %d applications found on the site\n", cProp, cApp);

	WEIWOOADMIN_CATCH;
}

void WeiwooAdminConsole::setSiteProp(const Args& siteName)
{
	ASSET_CONNECTION(_bizPrx);

	WEIWOOADMIN_TRY;

	::TianShanIce::Properties props = _bizPrx->getSiteProperties(siteName[0]);
	
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
	
	_bizPrx->setSiteProperties(siteName[0], props);

	WEIWOOADMIN_CATCH;
}

// about applications
void WeiwooAdminConsole::listApps()
{
	ASSET_CONNECTION(_bizPrx);

	WEIWOOADMIN_TRY;

	::TianShanIce::Site::AppInfos collection = _bizPrx->listApplications();
	
	printf("App       Endpoint                    Description\n");
	printf("--------  --------------------------  -------------------------\n");
	for (::TianShanIce::Site::AppInfos::iterator it = collection.begin(); it < collection.end(); it++)
		printf("%-8s  %-26s  %s\n", it->name.c_str(), it->endpoint.c_str(), it->desc.c_str());
	printf("    %d application(s) found\n", collection.size());

	WEIWOOADMIN_CATCH;
}

void WeiwooAdminConsole::updateApp(const Args& args)
{
	ASSET_CONNECTION(_bizPrx);

	WEIWOOADMIN_TRY;
	char buf[256];
	std::string endpoint, desc;

	readInput(buf, sizeof(buf)-1, "endpoint: ", false);
	endpoint = buf;

	readInput(buf, sizeof(buf)-1, "description: ");
	desc = buf;

	_bizPrx->updateApplication(args[0], endpoint, desc);

	WEIWOOADMIN_CATCH;
}

void WeiwooAdminConsole::removeApp(const Args& args)
{
	ASSET_CONNECTION(_bizPrx);
	
	WEIWOOADMIN_TRY;
	if (!_bizPrx->removeApplication(args[0]))
		error("failed to remove the specified application");
	WEIWOOADMIN_CATCH;
}

// about mount
void WeiwooAdminConsole::mount(const Args& args)
{
	ASSET_CONNECTION(_bizPrx);
	string sitename = args[0], mountpath, appname;

	WEIWOOADMIN_TRY;
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

	if (!_bizPrx->mountApplication(sitename, mountpath, appname))
		error("failed to mount the application \"%s\" at path \"%s\" on site \"%s\"", appname.c_str(), mountpath.c_str(), sitename.c_str());
	WEIWOOADMIN_CATCH;
}

void WeiwooAdminConsole::unmount(const Args& args)
{
	ASSET_CONNECTION(_bizPrx);
	string sitename = args[0], mountpath;

	WEIWOOADMIN_TRY;
	char buf[256];
	if (args.size()<2)
	{
		readInput(buf, sizeof(buf)-1, "mount path: ", false);
		mountpath = buf;
	}
	else mountpath = args[1];

	if (!_bizPrx->unmountApplication(sitename, mountpath))
		error("failed to unmount the application at path \"%s\" on site \"%s\"", mountpath.c_str(), sitename.c_str());
	WEIWOOADMIN_CATCH;
}

class SessionEventSinkI : public ::TianShanIce::SRM::SessionEventSink
{
public:
	SessionEventSinkI(WeiwooAdminConsole& console) : _console(console) {}

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

	WeiwooAdminConsole& _console;
};

void WeiwooAdminConsole::subscribe(const Args& args)
{
    if(args.empty())
    {
		error("'subscribe' requires the endpoint to the IceStorm service");
		return;
    }
	
	// build up the endpoint string
	std::string endpoint;
	for (Args::const_iterator it = args.begin(); it !=args.end(); it++)
		endpoint += *it + " ";

	WEIWOOADMIN_TRY;
	
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
	
	WEIWOOADMIN_CATCH;

	if (!_sub)
		error("failed to subscribe at \"%s\"\n", endpoint.c_str());
}
