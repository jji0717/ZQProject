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
// Ident : $Id: SentryPages.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Sentry/SentryPages.cpp $
// 
// 11    3/16/16 10:53a Ketao.zhang
// 
// 10    1/11/16 6:08p Dejian.fei
// 
// 9     5/08/14 4:25p Hui.shao
// source/log format
// 
// 8     5/08/14 3:12p Hui.shao
// 
// 7     5/08/14 2:34p Hui.shao
// 
// 6     5/07/14 6:00p Hui.shao
// bug#19016 only allow local Sentry to import/export Transport XML
// 
// 5     5/07/14 5:51p Hui.shao
// 
// 4     7/10/13 1:35p Build
// 
// 3     7/09/13 12:20p Build
// 
// 2     10-11-18 19:51 Xiaohui.chai
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 60    10-09-14 17:03 Fei.huang
// * fix bug 13163, use listService to find service name instead of
// looking for service binary
// 
// 59    10-01-12 17:41 Fei.huang
// * fix mem leak for strdup, crash on free
// 
// 58    10-01-06 18:17 Fei.huang
// * fix leak in page rendering
// 
// 57    09-12-17 18:25 Fei.huang
// * fix: don't compare service name in buffer in case of readlink error
// (remove dup entries)
// 
// 56    09-10-16 15:05 Xiaohui.chai
// Neighbors page:
// Rename column 'Proxy' to 'Endpoint' of table 'TianShan Modules'
// 
// 55    09-09-10 19:59 Xiaohui.chai
// 
// 54    09-08-07 19:00 Xiaohui.chai
// move the header/footer files to the folder 'layout'
// 
// 53    09-07-21 11:16 Fei.huang
// + search for running services
// 
// 52    09-07-06 17:17 Fei.huang
// 
// 51    09-07-06 17:16 Fei.huang
// * linux port
// 
// 50    09-05-18 17:07 Xiaohui.chai
// new implementment
// 
// 49    08-12-26 18:08 Xiaohui.chai
// add interface for IHttpResponse
// 
// 48    08-12-05 17:43 Hui.shao
// 
// 47    08-12-05 17:28 Hui.shao
// 
// 46    08-12-05 17:22 Hui.shao
// modified the table of modules
// 
// 45    08-09-04 14:16 Xiaohui.chai
// 
// 44    08-08-20 14:54 Xiaohui.chai
// 
// 43    08-07-14 19:33 Xiaohui.chai
// Added proxy page.
// 
// 42    08-06-25 17:18 Xiaohui.chai
// 
// 41    08-06-25 15:25 Xiaohui.chai
// 
// 40    08-06-20 14:25 Xiaohui.chai
// 
// 39    08-06-20 10:43 Xiaohui.chai
// 
// 38    08-06-19 17:10 Xiaohui.chai
// 
// 37    08-06-10 17:18 Xiaohui.chai
// 
// 36    08-06-10 11:32 Xiaohui.chai
// supported parameter all for NavPage
// 
// 35    08-03-25 11:44 Xiaohui.chai
// 
// 34    08-03-10 13:54 Xiaohui.chai
// added a refresh link to navpage
// 
// 33    08-02-28 13:20 Xiaohui.chai
// 
// 32    08-02-27 16:54 Xiaohui.chai
// changed neighbor map layout
// 
// 31    08-02-27 11:54 Xiaohui.chai
// change neighbor map layout
// 
// 30    08-02-22 12:06 Xiaohui.chai
// 
// 29    08-02-22 11:22 Xiaohui.chai
// web page display changed
// 
// 28    08-02-21 11:46 Xiaohui.chai
// 
// 27    08-02-20 16:27 Xiaohui.chai
// 
// 26    08-02-19 17:28 Xiaohui.chai
// 
// 25    08-02-19 15:53 Xiaohui.chai
// Show the active service name instead of interface name in the left
// panel.
// 
// 24    08-01-22 18:00 Xiaohui.chai
// 
// 23    08-01-21 16:34 Xiaohui.chai
// 
// 22    08-01-14 16:04 Xiaohui.chai
// interface changed:
// rename IHttpResponser to IHttpResponse
// rename IHttpRequestCtx::Responser to IHttpRequestCtx::Response
// add GetMethodType() to IHttpRequestCtx
// 
// 21    07-12-26 14:48 Xiaohui.chai
// 
// 20    07-12-25 14:04 Xiaohui.chai
// 
// 19    07-12-21 15:22 Xiaohui.chai
// 
// 18    07-12-21 14:36 Xiaohui.chai
// move transport map page to AdminCtrl_web.dll
// 
// 17    07-12-14 11:13 Xiaohui.chai
// 
// 16    07-11-23 14:37 Xiaohui.chai
// added site pages
// 
// 15    07-11-13 14:38 Xiaohui.chai
// 
// 14    07-11-09 12:20 Xiaohui.chai
// 
// 13    07-11-07 15:54 Xiaohui.chai
// 
// 12    07-11-07 14:47 Xiaohui.chai
// 
// 11    07-11-07 13:50 Xiaohui.chai
// 
// 10    07-11-06 16:18 Xiaohui.chai
// 
// 9     07-11-06 11:50 Yixin.tian
// 
// 8     07-11-05 16:05 Xiaohui.chai
// add neighbors page, neighbors map page, local interfaces page
// 
// 7     07-10-23 16:02 Xiaohui.chai
// add local processes page
// 
// 6     07-10-19 18:18 Xiaohui.chai
// 
// 5     07-10-16 15:13 Xiaohui.chai
// 
// 4     07-07-19 17:29 Hongquan.zhang
// 
// 3     07-06-07 12:31 Hui.shao
// refresh service information
// 
// 2     07-06-04 14:46 Hui.shao
// separated html pages from env
// ===========================================================================

#include <boost/thread.hpp>
#include <fstream>
#include "SentryPages.h"
#include "SentryEnv.h"
#include "SentryImpl.h"
#include "HtmlTempl.h"
#include "LayoutConfig.h"
#include <FileSystemOp.h>

#ifdef ZQ_OS_MSWIN
#include "ProcessPerfMonitor.h"
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

//#ifdef _DEBUG
#pragma comment(lib, "Advapi32")
//#else
//#endif

typedef std::vector<std::string> SVCSET;
extern void listService(SVCSET* set = 0);

namespace ZQTianShan {
namespace Sentry {

// -----------------------------
// class SentryPages
// -----------------------------
///
SentryPages::SentryPages(SentryEnv& env, const char* htmldir)
:_env(env)
{
	setHomeDir(htmldir);
	_navNeighbor.name = "TianShan Neighborhood";
	_navSite.name = "The TianShan Site";
}

const char* SentryPages::setHomeDir(const char* htmldir)
{
	if (NULL == htmldir || strlen(htmldir) <= 0)
		_htmldir = _env._programRootPath + "webctrl" FNSEPS;
	else _htmldir = htmldir;

	if (_htmldir[_htmldir.length() - 1] != FNSEPC)
		_htmldir += FNSEPS;

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(SentryPages, "setHomeDir: %s"), _htmldir.c_str());
	return _htmldir.c_str();
}

void SentryPages::prepareLayoutInfo()
{
	std::string layoutDir = _htmldir + "layout" + FNSEPS;
	FS::createDirectory(layoutDir, true);
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "prepare services' layout in: %s"), layoutDir.c_str());
	LayoutConfig cfg(_env);
	if (!cfg.load(_env._strDllConfig.c_str()))
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "fail to load layout config file.[configfile = %s]"), _env._strDllConfig.c_str());
		return;
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "prepare .hdr file in: %s"), layoutDir.c_str());
	cfg.GenerateLocalHeaderFiles(layoutDir.c_str());
	cfg.GenerateSiteHeaderFiles(layoutDir.c_str());
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, ".hdr file prepared"));

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "prepare footer.ftr in: %s"), layoutDir.c_str());
	std::ofstream footer((layoutDir + "footer.ftr").c_str());
	//ZQTianShan::HtmlTempl::HtmlFooter_MainPage(footer, _env._selfInfo.name.c_str(), (_env._selfInfo.adminRootUrl + "localhost.sysinvoke").c_str());
	ZQTianShan::HtmlTempl::HtmlFooter_MainPage_trivial(footer);
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "footer.ftr prepared"));

	// generate a default header
	{
		ZQTianShan::HtmlTempl::Tab tab;
		tab.name = "TianShan";
		std::ofstream defaultHdr((layoutDir + "default.hdr").c_str());
		::ZQTianShan::HtmlTempl::HtmlHeader_MainPage(defaultHdr, "TianShan", &tab, 1);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "default.hdr prepared"));
	}

	cfg.GetLocalNavNode(_navLocal);
	cfg.GetSiteNavNode(_navSite);

	// fixup nav page config
	{
		// local
		if (_navLocal.displayname.empty())
			_navLocal.displayname = _navLocal.name;
		std::vector < struct _NavNode >::iterator it_nav;
		for (it_nav = _navLocal.children.begin(); it_nav != _navLocal.children.end(); ++it_nav)
		{
			if (it_nav->displayname.empty())
				it_nav->displayname = it_nav->name;
		}

		// site
		if (_navSite.displayname.empty())
			_navSite.displayname = _navSite.name;
		for (it_nav = _navSite.children.begin(); it_nav != _navSite.children.end(); ++it_nav)
		{
			if (it_nav->displayname.empty())
				it_nav->displayname = it_nav->name;
		}
	}
}

SentryPages::NavNode SentryPages::getLocalActiveEntries()
{
	NavNode local;

	local.name = _navLocal.name;
	local.displayname = _navLocal.displayname;
	local.href = _navLocal.href;

#ifdef ZQ_OS_MSWIN

	//get the active service
	SC_HANDLE schSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager)
	{
		local.children.reserve(_navLocal.children.size());
		std::vector< NavNode >::const_iterator cit_svc;
		for (cit_svc = _navLocal.children.begin(); cit_svc != _navLocal.children.end(); ++cit_svc)
		{
			// acquire service handle
			SC_HANDLE schSvc = ::OpenService(schSCManager, cit_svc->name.c_str(), SC_MANAGER_ALL_ACCESS);
			if (schSvc)
			{
				bool bServiceActive = false;
				SERVICE_STATUS svcstat = { 0 };
				if (::QueryServiceStatus(schSvc, &svcstat))
				{
					// check service state
					switch (svcstat.dwCurrentState)
					{
					case SERVICE_STOP_PENDING:
					case SERVICE_RUNNING:
					case SERVICE_CONTINUE_PENDING:
					case SERVICE_PAUSE_PENDING:
					case SERVICE_PAUSED:
						bServiceActive = true;
						break;
					default:
						bServiceActive = false;
						break;
					}
				}

				if (bServiceActive)
					local.children.push_back(*cit_svc);

				::CloseServiceHandle(schSvc);
			}
			else
			{
				DWORD errcode = GetLastError();
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SentryPages, "OpenService() failed. service is %s, error code is %u"), cit_svc->name.c_str(), errcode);
			}
		}

		::CloseServiceHandle(schSCManager);
	}
	else
	{
		DWORD errcode = GetLastError();
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "OpenSCManager() failed, error code is %u"), errcode);
	}

#else
	SVCSET set;
	listService(&set);

	std::vector< NavNode >::const_iterator cit_svc;
	for (cit_svc = _navLocal.children.begin(); cit_svc != _navLocal.children.end(); ++cit_svc) {
		for (SVCSET::const_iterator running_svc = set.begin(); running_svc != set.end(); ++running_svc) {
			if (!strcasecmp(running_svc->c_str(), cit_svc->name.c_str())) {
				local.children.push_back(*cit_svc);
			}
		}
	}
#endif
	return local;
}

//////////////////////////////////////////////////////////////////////////
// local pages
#define LAYOUT_VAR_TEMPLATE     "#template"
void SentryPages::localHostPage(HttpRequestCtx *pHttpRequestCtx)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "request localhost page."));
	IHttpResponse &out = pHttpRequestCtx->Response();
	const char* tmpl = "LOCAL_LOCALHOST"; // fixed template name
	pushHeader(out, tmpl);

	out << "<table class='listTable'>";
	out << "<tr><th align=right>Sentry NodeId:&nbsp;&nbsp;&nbsp;</th><td>" << _env._selfInfo.id << "</td></tr>";
	out << "<tr><th align=right>Sentry Proxy:&nbsp;&nbsp;&nbsp;</th><td>" << _env._selfInfo.sentrysvcPrx << "</td></tr>";
	out << "<tr><th align=right>Admin RootURL:&nbsp;&nbsp;&nbsp;</th><td>" << _env._selfInfo.adminRootUrl << "</td></tr>";
	out << "<tr><th align=right>Group:&nbsp;&nbsp;&nbsp;</th><td>" << _env._groupAddr.getHostAddress() << ":" << _env._groupPort << "</td></tr>";
	out << "<tr style='background-color:black'><td colspan=2 style='padding:0'></td></tr>";
	out << "<tr><th align=right>Hostname:&nbsp;&nbsp;&nbsp;</th><td>" << _env._selfInfo.name << "</td></tr>";
	out << "<tr><th align=right>Processor:&nbsp;&nbsp;&nbsp;</th><td>" << _env._selfInfo.cpu << "</td></tr>";
	out << "<tr><td align=right>     freq:&nbsp;&nbsp;&nbsp;</th><td>" << _env._selfInfo.cpuClockMHz << "MHz</td></tr>";
	out << "<tr><td align=right>    count:&nbsp;&nbsp;&nbsp;</th><td>" << _env._selfInfo.cpuCount << "</td></tr>";
	out << "<tr><th align=right>Operating System:&nbsp;&nbsp;&nbsp;</th><td>" << _env._selfInfo.os << "</td></tr>";
	out << "<tr><th align=right>Physical Memory:&nbsp;&nbsp;&nbsp;</th><td>" << _env._selfInfo.memTotalPhys / 1024 / 1024 << "MB</td></tr>";
	out << "<tr><th align=right>Virtual Memory:&nbsp;&nbsp;&nbsp;</th><td>" << _env._selfInfo.memTotalVirtual / 1024 / 1024 << "MB</td></tr>";
	out << "<tr><th align=right>Startup:&nbsp;&nbsp;&nbsp;</th><td>";

	char dtbuf[32];
	if (TimeToUTC(_env._selfInfo.osStartup, dtbuf, sizeof(dtbuf)))
		out << dtbuf;

	out << "</td></tr>";
	out << "</table>";

	pushFooter(out, tmpl);
}

typedef struct _NtSvcDesc
{
	std::string name, status, displayname;
	int processId;
	int tsAppPid; // valid for active TianShan service
	bool isActiveTsService;
	std::string TsIf;
} NtSvcDesc;

typedef std::vector<NtSvcDesc> NtSvcDescs;
#define SERVICECTL_SVCNAME  "svcctl#name"
#define SERVICECTL_CTLTYPE  "svcctl#type"
void SentryPages::localServicesPage(HttpRequestCtx *pHttpRequestCtx)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "localServicesPage() refreshing local services page"));

#ifdef ZQ_OS_MSWIN
	IHttpResponse &out = pHttpRequestCtx->Response();
	const char* tmpl = pHttpRequestCtx->GetRequestVar(LAYOUT_VAR_TEMPLATE);
	if (NULL == tmpl)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "localServicesPage() template name missed"));
		pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
		return;
	}
	pushHeader(out, tmpl);

	//service control start or stop
	const char* pSvcName = pHttpRequestCtx->GetRequestVar(SERVICECTL_SVCNAME);
	const char* pStart = pHttpRequestCtx->GetRequestVar(SERVICECTL_CTLTYPE);
	if (pSvcName != NULL && pStart != NULL)//control service
	{
		char chCmd[256] = { 0 };
		if (*pStart == '1')
		{
			sprintf(chCmd, "net start \"%s\"", pSvcName);
			if (system(chCmd) != 0)
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "localServicesPage() start service[%s] failed"), pSvcName);
		}
		else if (*pStart == '0')
		{
			sprintf(chCmd, "net stop \"%s\"", pSvcName);
			if (system(chCmd) != 0)
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "localServicesPage() stop service[%s] failed"), pSvcName);
		}
		else
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "localServicesPage() unknown service control command"));
	}

	typedef std::map<int, int> PIDMap;
	PIDMap tsSvc2App;
	{
		ProcessPerfMonitor perfmon(_env._log);
		ProcessPerfData::State procState;
		perfmon.Snapshot().swap(procState);

		ZQ::common::MutexGuard gd(_env._lockLocalProcesses);
		SentryEnv::Processes::const_iterator cit_tsProcess;
		for (cit_tsProcess = _env._localProcesses.begin(); cit_tsProcess != _env._localProcesses.end(); ++cit_tsProcess)
		{
			std::vector< ProcessPerfData::RawData >::const_iterator cit_pd;
			for (cit_pd = procState.processesRawData.begin(); cit_pd != procState.processesRawData.end(); ++cit_pd)
			{
				if (cit_tsProcess->processId == (int)cit_pd->processId)
					tsSvc2App[cit_pd->parentPid] = cit_tsProcess->processId;
			}
		}
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "localServicesPage() enumerating local services"));
	NtSvcDescs NtSvcs;
	{
		ENUM_SERVICE_STATUS_PROCESS tbEnumServicesStatus[64];
		int iBytesNeeded, iNEntriesRead, iRet = ERROR_MORE_DATA;
		SC_HANDLE schSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

		if (NULL == schSCManager)
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "OpenSCManager() failed"));
		else
		{
			DWORD dwResumeHandle = 0;

			// Process groups of ServiceStatus blocks
			while (iRet == ERROR_MORE_DATA)
			{
				if (!::EnumServicesStatusEx(schSCManager, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_ACTIVE + SERVICE_INACTIVE,
					(LPBYTE)tbEnumServicesStatus, sizeof(tbEnumServicesStatus),
					(LPDWORD)&iBytesNeeded, (LPDWORD)&iNEntriesRead, (LPDWORD)&dwResumeHandle, NULL))
				{
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "EnumServicesStatus() failed"));
					iRet = ::GetLastError();
					if (iRet != ERROR_MORE_DATA)
						break;
				}
				else iRet = 0;

				// Process body of ServiceStatus blocks within current group
				{
					for (int i = 0; i < iNEntriesRead; i++)
					{
						NtSvcDesc svcdesc;
						svcdesc.name = tbEnumServicesStatus[i].lpServiceName;
						svcdesc.displayname = tbEnumServicesStatus[i].lpDisplayName;
						svcdesc.processId = -1;
						svcdesc.isActiveTsService = false;

						switch (tbEnumServicesStatus[i].ServiceStatusProcess.dwCurrentState)
						{
						case SERVICE_STOPPED:
							svcdesc.status = "Stopped";
							break;
						case SERVICE_START_PENDING:
							svcdesc.status = "Start Pending";
							break;
						case SERVICE_STOP_PENDING:
							svcdesc.status = "Stop Pending";
							svcdesc.processId = tbEnumServicesStatus[i].ServiceStatusProcess.dwProcessId;
							break;
						case SERVICE_RUNNING:
							svcdesc.status = "Running";
							svcdesc.processId = tbEnumServicesStatus[i].ServiceStatusProcess.dwProcessId;
							break;
						case SERVICE_CONTINUE_PENDING:
							svcdesc.status = "Continue Pending";
							svcdesc.processId = tbEnumServicesStatus[i].ServiceStatusProcess.dwProcessId;
							break;
						case SERVICE_PAUSE_PENDING:
							svcdesc.status = "Pause Pending";
							svcdesc.processId = tbEnumServicesStatus[i].ServiceStatusProcess.dwProcessId;
							break;
						case SERVICE_PAUSED:
							svcdesc.status = "Paused";
							svcdesc.processId = tbEnumServicesStatus[i].ServiceStatusProcess.dwProcessId;
							break;
						default:
							svcdesc.status = "UNKNOWN";
						}

						//mark TianShan service
						if (svcdesc.processId > 0)
						{
							PIDMap::iterator it_pid = tsSvc2App.find(svcdesc.processId);
							if (it_pid != tsSvc2App.end())
							{
								envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "localServicesPage() found TianShan service[%s]"), svcdesc.name.c_str());
								svcdesc.isActiveTsService = true;
								svcdesc.tsAppPid = it_pid->second;
							}
						}

						NtSvcs.push_back(svcdesc);
					}
				} // for

			} // while

			::CloseServiceHandle(schSCManager);
		} // if schSCManager
	}


	{
		// ctrl form
		out << "<form id=\"svcctrl-form\" method=\"get\" action=\"" << pHttpRequestCtx->GetUri() << "\">";
		out << "<input type=\"hidden\" name=\"" LAYOUT_VAR_TEMPLATE "\" value=\"" << tmpl << "\">";
		out << "<input id=\"ctrl-type\" type=\"hidden\" name=\"" SERVICECTL_CTLTYPE "\">";
		out << "<input id=\"svc-name\" type=\"hidden\" name=\"" SERVICECTL_SVCNAME "\">";
		out << "</form>";
		out << "<script type=\"text/javascript\">"
			<< "function startSvc(svcname){"
			<< "document.getElementById(\"ctrl-type\").value=\"1\";"
			<< "document.getElementById(\"svc-name\").value=svcname;"
			<< "document.getElementById(\"svcctrl-form\").submit();"
			<< "}\n";
		out << "function stopSvc(svcname){"
			<< "document.getElementById(\"ctrl-type\").value=\"0\";"
			<< "document.getElementById(\"svc-name\").value=svcname;"
			<< "document.getElementById(\"svcctrl-form\").submit();"
			<< "}</script>";
		// Running TianShan services Table
		out << "<h2>Active TianShan Components</h2><table class='listTable'>"
			<< "<tr><th>Ctrl</th><th>Service</th><th>Status</th><th>ProcessId</th><th>Description</th><th>Interface</th></tr>\n";
		out << "<colgroup><col span='3'><col style='text-align:right'></colgroup>";

		ZQ::common::MutexGuard gd(_env._lockLocalAdapters);
		for (NtSvcDescs::iterator it = NtSvcs.begin(); it < NtSvcs.end(); it++)
		{
			if (!it->isActiveTsService)
				continue;

			out << "<tr><td><span class=\"ctrl lnk\" onclick=\"stopSvc('" << it->name << "')\">&lt;</span></td>";
			out << "<td>" << it->name
				<< "</td><td>" << it->status
				<< "</td><td>" << it->processId
				<< "</td><td>" << it->displayname
				<< "</td><td>";
			// interfaces
			std::string ifs;
			for (SentryEnv::Adapters::iterator ait = _env._localAdapters.begin(); ait < _env._localAdapters.end(); ait++)
			{
				if (it->tsAppPid == ait->processId)
				{
					for (TianShanIce::StrValues::iterator sit = ait->interfaces.begin(); sit < ait->interfaces.end(); sit++)
						ifs += *sit + ",";
				}
			}
			if (!ifs.empty())
			{
				ifs.erase(ifs.size() - 1); // remove the last ','<< ifs 
				out << ifs;
			}
			else
			{
				out << "N/A";
			}
			out << "</td></tr>";
		}
		out << "</table><p>\n";
	}

	// all services Table
	out << "<hr><p><h2>All Services</h2><table class='listTable' style='text-align:left;'>"
		<< "<tr><th>Ctrl</th><th>Service</th><th>Status</th><th>ProcessId</th><th>Detail</th></tr>\n";
	out << "<colgroup><col span='3'><col style='text-align:right'></colgroup>";

	for (NtSvcDescs::iterator it = NtSvcs.begin(); it < NtSvcs.end(); it++)
	{
		char buf[1024];
		if (it->processId == -1)
			out << "<tr><td><span class=\"ctrl lnk\" onclick=\"startSvc('" << it->name << "')\">4</span></td>";
		else
			out << "<tr><td><span class=\"ctrl lnk\" onclick=\"stopSvc('" << it->name << "')\">&lt;</span></td>";

		sprintf(buf, "<td>%s</td><td>%s</td><td>%d</td><td>%s</td></tr>\n", it->name.c_str(), it->status.c_str(), it->processId, it->displayname.c_str());
		out << buf;
	}
	out << "</table><p>\n";

	pushFooter(out, tmpl);
#endif
}

void SentryPages::localProcessesPage(HttpRequestCtx *pHttpRequestCtx)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "localProcessesPage() refreshing local processe list"));

#ifdef ZQ_OS_MSWIN
	IHttpResponse &out = pHttpRequestCtx->Response();
	const char* tmpl = pHttpRequestCtx->GetRequestVar(LAYOUT_VAR_TEMPLATE);
	if (NULL == tmpl)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "localProcessesPage() template name missed"));
		pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
		return;
	}

	pushHeader(out, tmpl);

	ProcessPerfMonitor perfmon(_env._log);
	perfmon.CollectPerfData();
	Sleep(500);
	perfmon.CollectPerfData();
	ProcessesPD pds;
	perfmon.GetPerfData(pds);

	//display
	out << "<div>";
	out << "<h2>Running Processes</h2>";
	out << "<table class='listTable'>";
	out << "<colgroup><col class=\"col-desc\" style=\"text-align:left\"><col span='6' style='text-align:right'></colgroup>";
	out << "<tr class='heading'><th>Image Name</th><th>PID</th><th>CPU</th><th>Memory Usage</th><th>Virtual Memory Size</th><th>Handle Count</th><th>Thread Count</th></tr>";
	for (size_t i = 0; i < pds.size(); ++i)
	{
		out << "<tr>";
		out << "<td>" << pds[i].imageA << "</td>";
		out << "<td>" << pds[i].processId << "</td>";
		out << "<td>" << pds[i].cpuUsagePercent << "</td>";
		out << "<td>" << (DWORD)(pds[i].memUsageByte.QuadPart / 1024) << " KB</td>";
		out << "<td>" << (DWORD)(pds[i].vmemSizeByte.QuadPart / 1024) << " KB</td>";
		out << "<td>" << pds[i].handleCount << "</td>";
		out << "<td>" << pds[i].threadCount << "</td>";
		out << "</tr>";
	}

	out << "</table>";
	out << "</div>";
	pushFooter(out, tmpl);
#endif
}

void SentryPages::localInterfacesPage(HttpRequestCtx *pHttpRequestCtx)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "localInterfacesPage() requested"));

	IHttpResponse &out = pHttpRequestCtx->Response();
	const char* tmpl = pHttpRequestCtx->GetRequestVar(LAYOUT_VAR_TEMPLATE);
	if (NULL == tmpl)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "template name missed during the request."));
		pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
		return;
	}

	pushHeader(out, tmpl);

	out << "<div><h2>Active Local Modules</h2>";
	out << "<table class='listTable'>";
	out << "<tr class='heading'><th>Interface</th><th>Adatper</th><th>Endpoint</th><th>PID</th><th>Activated</th></tr>";

	std::set< int > localActiveProcesses;
	{
		// get all active process
		ZQ::common::MutexGuard gd(_env._lockLocalProcesses);

		::Ice::Long stampExp = now() - _env._timeout * 2000;
		SentryEnv::Processes::const_iterator cit_process;
		for (cit_process = _env._localProcesses.begin(); cit_process != _env._localProcesses.end(); ++cit_process)
		{
			if (stampExp <= cit_process->lastHeartbeat)
				localActiveProcesses.insert(cit_process->processId);
		}
	}

	int nIfCount = 0;
	{
		// list all active interface
		ZQ::common::MutexGuard gd(_env._lockLocalAdapters);

		SentryEnv::Adapters::const_iterator cit_adptr;
		for (cit_adptr = _env._localAdapters.begin(); cit_adptr != _env._localAdapters.end(); ++cit_adptr)
		{
			if (localActiveProcesses.end() == localActiveProcesses.find(cit_adptr->processId))
				continue;

			// active adapter
			::TianShanIce::StrValues::const_iterator cit_if;
			char tempbuf[80];
			for (cit_if = cit_adptr->interfaces.begin(); cit_if != cit_adptr->interfaces.end(); ++cit_if)
			{
				out << "<tr>"
					<< "<td>" << (*cit_if) << "</td>"
					<< "<td>" << cit_adptr->adapterId << "</td>"
					<< "<td>" << cit_adptr->endpoint << "</td>"
					<< "<td>" << cit_adptr->processId << "</td>"
					<< "<td>" << ZQTianShan::TimeToUTC(cit_adptr->lastChange, tempbuf, sizeof(tempbuf)-2) << "</td>"
					<< "</tr>";

				++nIfCount;
			}
		}
	}

	out << "</table>";
	out << "<h2>Total:  " << nIfCount << "</h2>";
	out << "</div>";

	pushFooter(out, tmpl);
}

//////////////////////////////////////////////////////////////////////////
// neighbors pages
typedef struct _FatNodeInfo
{
	SentryEnv::NodeInfo baseNodeInfo;
	::ZqSentryIce::ServiceInfos services;
} FatNodeInfo;

typedef std::map<std::string, FatNodeInfo> FatNodeMap;

#define PAGE_LINK_STYLE     "linkstyle"

void SentryPages::neighborsPage(HttpRequestCtx *pHttpRequestCtx)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "neighborsPage() requested"));

	IHttpResponse &out = pHttpRequestCtx->Response();
	const char* tmpl = pHttpRequestCtx->GetRequestVar(LAYOUT_VAR_TEMPLATE);
	if (NULL == tmpl)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "neighborsPage() template name missed in request."));
		pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
		return;
	}

	std::string externalLinkInstruction;
	{
		// construct instruction of external-link's style, only support new-window now
		const char* linkStyle = pHttpRequestCtx->GetRequestVar(PAGE_LINK_STYLE);
		if (linkStyle && '\0' != linkStyle)
			externalLinkInstruction = std::string("target='") + linkStyle + "' ";
	}

	pushHeader(out, tmpl);

	// step 1. gather fat node info with service info included
	FatNodeMap fatNodeMap;
	{
		::Ice::Long stampExp = now() - _env._timeout * 2000;

		ZQ::common::MutexGuard gd(_env._lockNeighbors);
		for (SentryEnv::NodeMap::iterator it = _env._neighbors.begin(); it != _env._neighbors.end(); it++)
		{
			if (it->second.lastHeartbeat < stampExp)
				continue;
			FatNodeInfo fatNodeInfo;
			fatNodeInfo.baseNodeInfo = it->second;
			fatNodeInfo.services.clear();
			fatNodeMap.insert(FatNodeMap::value_type(fatNodeInfo.baseNodeInfo.id, fatNodeInfo));
		}
	}

	{
		ZQ::common::MutexGuard gd(_env._lockRemoteServices);
		for (SentryEnv::RemoteServices::iterator it = _env._remoteServices.begin(); it < _env._remoteServices.end(); it++)
		{
			if (fatNodeMap.end() == fatNodeMap.find(it->nodeid))
				continue;

			fatNodeMap[it->nodeid].services.push_back(it->baseInfo);
		}
	}

	if (!fatNodeMap.empty())
	{
		FatNodeMap::iterator it;
		out << "<div><h2>Participant Machines</h2>";
		out << "<table class='listTable'><tr class='heading'><th>Hostname</th><th>NodeId</th><th>rootURL</th><th>Processor</th><th>Memory</th><th>OS</th><th>Startup</th></tr>";
		for (it = fatNodeMap.begin(); it != fatNodeMap.end(); it++)
		{
			FatNodeInfo& nodeinfo = it->second;
			out << "<tr>"
				<< "<td><a " << externalLinkInstruction << "href=\"" << nodeinfo.baseNodeInfo.adminRootUrl << "\">"
				<< nodeinfo.baseNodeInfo.name << "</a></td>"
				<< "<td>" << nodeinfo.baseNodeInfo.id << "</td>"
				<< "<td>" << nodeinfo.baseNodeInfo.adminRootUrl << "</td>"
				<< "<td>" << nodeinfo.baseNodeInfo.cpu << "  " << nodeinfo.baseNodeInfo.cpuClockMHz << "MHz x" << nodeinfo.baseNodeInfo.cpuCount << "</td>"
				<< "<td>" << nodeinfo.baseNodeInfo.memTotalPhys / 1024 / 1024 << "MB / " << nodeinfo.baseNodeInfo.memTotalVirtual / 1024 / 1024 << "MB</td>"
				<< "<td>" << nodeinfo.baseNodeInfo.os << "</td><td>";

			char dtbuf[32];
			if (TimeToUTC(nodeinfo.baseNodeInfo.osStartup, dtbuf, sizeof(dtbuf)))
				out << dtbuf;

			out << "</td></tr>";
		}

		out << "</table><h2>TianShan Modules</h2>";
		out << "<table class='listTable'><tr class='heading'><th>Hostname</th><th>Interfaces</th><th>Endpoint</th></tr>";
		out << "<colgroup><col class='heading'></colgroup>";

		for (it = fatNodeMap.begin(); it != fatNodeMap.end(); it++)
		{
			FatNodeInfo& nodeinfo = it->second;
			if (nodeinfo.services.empty())
				continue;

			::ZqSentryIce::ServiceInfos::iterator sit = nodeinfo.services.begin();
			out << "<tr><td rowspan=" << (uint32)nodeinfo.services.size() << ">" << nodeinfo.baseNodeInfo.name << "</td>";
			out << "<td>" << sit->name << "</td><td>" << sit->proxystr << "</td></tr>";

			++sit;
			for (; sit < nodeinfo.services.end(); sit++)
			{
				out << "<tr><td>" << sit->name << "</td><td>" << sit->proxystr << "</td></tr>";
			}
		}

		out << "</table></div>";
	}

	pushFooter(out, tmpl);
}

void SentryPages::neighborsMapPage(HttpRequestCtx *pHttpRequestCtx)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "neighborsMapPage() requested"));

	IHttpResponse &out = pHttpRequestCtx->Response();
	const char* tmpl = pHttpRequestCtx->GetRequestVar(LAYOUT_VAR_TEMPLATE);
	if (NULL == tmpl)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "neighborsMapPage() template name missed"));
		pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
		return;
	}

	pushHeader(out, tmpl);

	//gather fat node info with service info included
	FatNodeMap fatNodeMap;
	{
		::Ice::Long stampExp = now() - _env._timeout * 2000;

		ZQ::common::MutexGuard gd(_env._lockNeighbors);
		for (SentryEnv::NodeMap::iterator it = _env._neighbors.begin(); it != _env._neighbors.end(); it++)
		{
			if (it->second.lastHeartbeat < stampExp)
				continue;
			FatNodeInfo fatNodeInfo;
			fatNodeInfo.baseNodeInfo = it->second;
			fatNodeInfo.services.clear();
			fatNodeMap.insert(FatNodeMap::value_type(fatNodeInfo.baseNodeInfo.id, fatNodeInfo));
		}
	}

	{
		ZQ::common::MutexGuard gd(_env._lockRemoteServices);
		for (SentryEnv::RemoteServices::iterator it = _env._remoteServices.begin(); it < _env._remoteServices.end(); it++)
		{
			if (fatNodeMap.end() == fatNodeMap.find(it->nodeid))
				continue;

			fatNodeMap[it->nodeid].services.push_back(it->baseInfo);
		}
	}

	// generate neighbor map page
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "neighborsMapPage() preparing neighborhood map"));
	{
		std::ofstream dotfile((_htmldir + "neighbor1.dot").c_str());
		dotfile << "digraph G {\n size=\"6,100\";\n node[shape=plaintext, fontsize=8, fontname=Arial]; rankdir=LR; ranksep=1.5;\n";
		dotfile << "   subgraph selfnode {\n";
		dotfile << "      n" << _env._selfInfo.id
			<< " [shape=plaintext, URL=\"localhost.sysinvoke\","
			<< " label=<<table border='0' cellborder='0' cellpadding='0' cellspacing='0'><tr><td align='left' valign='top'>"
			<< _env._selfInfo.name << "<br align='left'/>";

		{
			ZQ::common::MutexGuard gd(_env._lockLocalAdapters);
			for (SentryEnv::Adapters::iterator it = _env._localAdapters.begin(); it < _env._localAdapters.end(); it++)
			{
				for (::TianShanIce::StrValues::iterator snit = it->interfaces.begin(); snit < it->interfaces.end(); snit++)
				{
					if ((*snit).empty())
						continue;
					dotfile << " - " << *snit << "<br align='left'/>";
				}
			}
		}

		dotfile << "</td><td port='icon' align='left' valign='top'><img src='"
			<< _htmldir << "black_server.png'/></td></tr></table>>];\n}\n";

		dotfile << "subgraph tsnodes {\n";

		FatNodeMap::iterator it;
		for (it = fatNodeMap.begin(); it != fatNodeMap.end(); it++)
		{
			FatNodeInfo& nodeinfo = it->second;
			if (0 == nodeinfo.baseNodeInfo.id.compare(_env._selfInfo.id))
				continue;

			dotfile << "      n" << nodeinfo.baseNodeInfo.id
				<< " [shape=plaintext, URL=\"" << nodeinfo.baseNodeInfo.adminRootUrl << "\""
				<< ",label=<<table border='0' cellborder='0' cellpadding='0' cellspacing='0'><tr><td align='left' valign='top' port='icon' rowspan='2'><img src='"
				<< _htmldir << "server.png'/></td><td align='left' valign='top'>"
				<< nodeinfo.baseNodeInfo.name << "<br align='left'/>";
			for (::ZqSentryIce::ServiceInfos::iterator sit = nodeinfo.services.begin(); sit < nodeinfo.services.end(); sit++)
				dotfile << " - " << sit->name << "<br align='left'/>";

			dotfile << "</td></tr></table>>];";
		}
		dotfile << "}\n";

		dotfile << "subgraph links {\n edge[style=dotted,weight=100,arrowhead=both,fontname=Arial,fontsize=10];";
		for (it = fatNodeMap.begin(); it != fatNodeMap.end(); it++)
		{
			FatNodeInfo& nodeinfo = it->second;
			if (0 == nodeinfo.baseNodeInfo.id.compare(_env._selfInfo.id))
				continue;
			dotfile << "          n" << _env._selfInfo.id << ":icon -> n" << nodeinfo.baseNodeInfo.id << ":icon;";
			// 			FatNodeMap::iterator j =it;
			// 			for (j++; j != fatNodeMap.end(); j++)
			// 				dotfile << "          n" << nodeinfo.baseNodeInfo.id << " -> n" << j->second.baseNodeInfo.id << ";";
		};
		dotfile << "}\n\n}";
	}

	std::string cmd = std::string("dot -Tpng \"") + _htmldir + "neighbor1.dot\" -o\"" + _htmldir + "neighbor.png\"";
	if (0 != ::system(cmd.c_str()))
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "error occured when execute command: %s"), cmd.c_str());

	cmd = std::string("dot -Tcmap \"") + _htmldir + "neighbor1.dot\" -o\"" + _htmldir + "neighbor.map\"";
	if (0 != ::system(cmd.c_str()))
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "neighborsMapPage() error occured when execute command: %s"), cmd.c_str());

	//
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "neighborsMapPage() importing map content: %s"), (_htmldir + "neighbor.map").c_str());
		out << "<IMG SRC='neighbor.png' usemap='#neighbor__map' border=0><map name='neighbor__map'>";
		std::ifstream mapfile((_htmldir + "neighbor.map").c_str());
		std::string mapstr;
		while (mapfile.good())
		{
			char ch = 0;
			mapfile.get(ch);
			mapstr.push_back(ch);
		}

		std::string externalLinkInstruction;
		{
			// construct instruction of external-link's style, only support new-window now
			const char* linkStyle = pHttpRequestCtx->GetRequestVar(PAGE_LINK_STYLE);
			if (linkStyle && '\0' != linkStyle)
				externalLinkInstruction = std::string("target='") + linkStyle + "' ";
		}

		if (!externalLinkInstruction.empty())
		{
			// modify the html code
			// ignore the local area, always the first
			std::string::size_type pos = mapstr.find(">");
			while ((pos = mapstr.find("<area ", pos)) != std::string::npos)
			{
				pos += 6; // strlen("<area "), got the insertion position
				mapstr.insert(pos, externalLinkInstruction);
			}
		}

		out << mapstr << "\n</map>";
	}

	pushFooter(out, tmpl);
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// site pages
#define SITEPAGE_VAR_SVCNAME "site.svc#name"

void SentryPages::globalServicePage(HttpRequestCtx *pHttpRequestCtx)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "globalServicePage() requested"));

	//
	IHttpResponse &out = pHttpRequestCtx->Response();
	const char* tmpl = pHttpRequestCtx->GetRequestVar(LAYOUT_VAR_TEMPLATE);
	if (NULL == tmpl)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "globalServicePage() template name missed"));
		pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
		return;
	}

	const char* svcname = pHttpRequestCtx->GetRequestVar(SITEPAGE_VAR_SVCNAME);
	if (NULL == svcname)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "globalServicePage() service name missed"));
		pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
		return;
	}

	pushHeader(out, tmpl);
	out << "<div><h2>Global Interface : " << svcname << "</h2>";
	out << "<table class='listTable'><tr class='heading'><th>NodeId</th><th>Proxy String</th></tr>";
	//service on this node
	{
		bool bFoundSvc = false;
		ZQ::common::MutexGuard gd(_env._lockLocalAdapters);
		for (SentryEnv::Adapters::const_iterator cit_adptr = _env._localAdapters.begin(); cit_adptr != _env._localAdapters.end(); ++cit_adptr)
		{
			for (::TianShanIce::StrValues::const_iterator cit_if = cit_adptr->interfaces.begin(); cit_if != cit_adptr->interfaces.end(); ++cit_if)
			{
				if (0 == cit_if->compare(svcname))
				{
					out << "<td>" << _env._selfInfo.id << "</td>"
						<< "<td>" << svcname << ":" << cit_adptr->endpoint << "</td></tr>";
					bFoundSvc = true;
					break;
				}
			}
			if (bFoundSvc)
				break;
		}
	}

	//services on remote nodes
	{
		ZQ::common::MutexGuard gd(_env._lockRemoteServices);
		for (SentryEnv::RemoteServices::const_iterator cit_rmtsvc = _env._remoteServices.begin(); cit_rmtsvc != _env._remoteServices.end(); ++cit_rmtsvc)
		{
			if (0 == cit_rmtsvc->baseInfo.name.compare(svcname))
			{
				out << "<td>" << cit_rmtsvc->nodeid << "</td>"
					<< "<td>" << cit_rmtsvc->baseInfo.proxystr << "</td></tr>";
			}
		}
	}
	out << "</table></div>";

	pushFooter(out, tmpl);
}

//////////////////////////////////////////////////////////////////////////
#define __AUX_NavDirBegin(out, name, URL, id) {\
	out << "<p class='nav_dir' onmouseover='mouseGoesOver(this)' onmouseout='mouseGoesOut(this)' "\
	<< "onclick=\"toggleFolder('" << id << "', this)\">"\
	<< "<span class='nav'><a onclick='markActive(this)' href=\""\
	<< URL << "\" target=\"basefrm\">"\
	<< name << "</a></span>"\
	<< "<img src='images/tree/arrow_up.gif' alt='o'/></p>"; \
	out << "<div id=\"" << id << "\" style=\"display: block;\">"; \
}

#define __AUX_NavDirEnd(out){ out << "</div>"; }
#define __AUX_NavItem(out, name, URL) {\
	out << "<p class='nav_item' onmouseover='mouseGoesOver(this)' onmouseout='mouseGoesOut(this)'>"\
	<< "<span class='nav'><a onclick='markActive(this)' href=\""\
	<< URL << "\" target=\"basefrm\">"\
	<< name << "</a></span></p>"; \
}

void SentryPages::navPage(HttpRequestCtx *pHttpRequestCtx)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "navPage() requested"));

	IHttpResponse &nav = pHttpRequestCtx->Response();
	//html head
	nav << "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n"
		<< "<head>\n"
		<< "<meta http-equiv=\"Content-Type\" content=\"text/xhtml;charset=iso-8859-1\" />\n"
		<< "<meta http-equiv=\"Content-Style-Type\" content=\"text/css\" />\n"
		<< "<meta http-equiv=\"Content-Language\" content=\"en\" />\n"
		<< "<link rel=\"stylesheet\" href=\"tsweb.css\">\n"
		<< "<title>TreeView</title>\n"
		<< "<script language=\"JavaScript\" src=\"nav.js\"> </script>\n"
		<< "</head>\n";

	const char* caredMachines = pHttpRequestCtx->GetRequestVar("all");
	//html body
	nav << "<body class=\"ftvtree coolscrollbar\">\n"
		<< "<div class=\"directory scrollPanel\" style='background-color:#ffffff;'>\n"  //div +1

#ifdef ZQ_OS_MSWIN
#pragma message(__MSGLOC__ "The title of the top level folder should be separated from the source")
#endif // ZQ_OS_MSWIN

		<< "<p class='nav_dir nav_top' onmouseover='mouseGoesOver(this)' onmouseout='mouseGoesOut(this)'>"
		<< "<span class='nav'><a href='nav.sysinvoke";
	if (caredMachines)
	{
		nav << "?all=" << caredMachines << "'>Streaming Service</a>";
	}
	else
	{
		nav << "'>ZQ TianShan</a>";
	}

	nav << "</span><img src='images/tree/arrow_left.gif' alt='o'/></p>\n"
		<< "<div style=\"display: block;\">"; //div +2

	if (caredMachines) // all server included and use the value as the sorting sequence
	{
#ifdef ZQ_OS_MSWIN
#pragma message(__MSGLOC__ "sort the machines base on the type")
#endif // ZQ_OS_MSWIN

		typedef std::multimap<std::string, std::string> NeighborsInfo; // type to proxy multimap
		NeighborsInfo neighbors;
		// copy a list of neighbors machine
		{
			ZQ::common::MutexGuard gd(_env._lockNeighbors);
			for (SentryEnv::NodeMap::iterator it = _env._neighbors.begin(); it != _env._neighbors.end(); it++)
			{
				std::string type = it->second.type;
				// case insensitive compare
				std::transform(type.begin(), type.end(), type.begin(), toupper);;
				neighbors.insert(std::make_pair(type, it->second.sentrysvcPrx));
			}
		}

		// the info of this machine
		std::string selfType = _env._selfInfo.type;
		std::transform(selfType.begin(), selfType.end(), selfType.begin(), toupper);
		bool selfProcessed = false;

		int idxMach = 0; // global machine index

		// sort the machines base on the type
		std::vector<std::string> caredMachinesV;
		ZQ::common::stringHelper::SplitString(caredMachines, caredMachinesV, " ");
		caredMachinesV.push_back(""); // placeholder
		for (std::vector<std::string>::iterator itType = caredMachinesV.begin(); itType != caredMachinesV.end(); ++itType)
		{
			std::transform(itType->begin(), itType->end(), itType->begin(), toupper); // case insensitive

			if ((!selfProcessed) && (selfType == (*itType) || itType->empty()))
			{ // insert the info of this machine
				__AUX_NavDirBegin(nav, _navLocal.displayname, _navLocal.href, "thismachine");

				//get the active service
				NavNode localActiveEntries = getLocalActiveEntries();
				std::vector< NavNode > &localActiveServices = localActiveEntries.children;
				for (size_t i = 0; i < localActiveServices.size(); ++i)
				{
					__AUX_NavItem(nav, localActiveServices[i].displayname, localActiveServices[i].href);
				}
				__AUX_NavDirEnd(nav);

				selfProcessed = true; // mark processed
			}

			// get the machines of this type
			std::pair<NeighborsInfo::iterator, NeighborsInfo::iterator> machRange; // range of current processing machines
			if (itType->empty()) // reach the placeholder
			{
				machRange.first = neighbors.begin();
				machRange.second = neighbors.end();
			}
			else
			{
				machRange = neighbors.equal_range(*itType);
			}

			for (NeighborsInfo::iterator itMach = machRange.first; itMach != machRange.second; ++itMach)
			{
				if (itMach->second.empty())
				{
					envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SentryPages, "No proxy string for machine [%s]. The machine's info is not available now."), itMach->first.c_str());
					continue;
				}

				try {
					ZqSentryIce::SentryServicePrx other =
						::ZqSentryIce::SentryServicePrx::uncheckedCast(_env._communicator->stringToProxy(itMach->second));
					ZqSentryIce::WebView mach = other->getWebView();

					std::string id = "machine";
					{
						char buf[10] = { 0 };
						id += itoa(++idxMach, buf, 10);
					}

					__AUX_NavDirBegin(nav, mach.base.name, mach.base.URL, id);
					for (int iSvc = 0; iSvc < (int)mach.services.size(); ++iSvc)
					{
						ZqSentryIce::WebEntry &svc = mach.services[iSvc];
						__AUX_NavItem(nav, svc.name, svc.URL);
					}

					__AUX_NavDirEnd(nav);
				}
				catch (const Ice::Exception &e)
				{
					envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SentryPages, "Caught [%s] during query web view from [%s]."), e.ice_name().c_str(), itMach->second.c_str());
				}
			} // end for NeighborsInfo::iterator

			neighbors.erase(machRange.first, machRange.second); // remove the machines of this type
		} // end for caredMachinesV::it
	}
	else //local
	{
		__AUX_NavDirBegin(nav, _navLocal.displayname + "(localhost)", _navLocal.href, "localfolder");

		//get the active service
		NavNode localActiveEntries = getLocalActiveEntries();
		std::vector< NavNode > &localActiveServices = localActiveEntries.children;

		bool bLocalWeiwoo = false;
		if (!localActiveServices.empty())
		{
			for (size_t i = 0; i < localActiveServices.size(); ++i)
			{
				// envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "navPage() local aliveSvc[%s] [%s]"), localActiveServices[i].name.c_str(), localActiveServices[i].displayname.c_str());
				__AUX_NavItem(nav, localActiveServices[i].displayname, localActiveServices[i].href);
				std::string svcname = localActiveServices[i].name;
				std::transform(svcname.begin(), svcname.end(), svcname.begin(), tolower);
				if (std::string::npos != svcname.find("weiwoo")) // local Weiwoo
				{
					// TODO: verify if the site-wide SessionMgr is the local Weiwoo instance
					bLocalWeiwoo = true;
				}
			}
		}
		__AUX_NavDirEnd(nav);

#ifndef ZQ_OS_MSWIN
		// bug#19016 only allow local Sentry to import/export Transport XML, the flag is passed to plugin AdminCtrl_web via env-var
		::setenv("sentry_localWeiwoo", bLocalWeiwoo ? "1" : "0", 1);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "navPage() setenv() sentry_localWeiwoo=%s"), bLocalWeiwoo ? "1" : "0");
#endif // ZQ_OS_MSWIN
	} // end of local

	// site
	{
		__AUX_NavDirBegin(nav, _navSite.displayname, _navSite.href, "sitefolder");
		if (!_navSite.children.empty())
		{
			for (size_t i = 0; i < _navSite.children.size(); ++i)
			{
				__AUX_NavItem(nav, _navSite.children[i].displayname, _navSite.children[i].href);
			}
		}
		__AUX_NavDirEnd(nav);
	}

	nav << "</div>"; //div -2
	nav << "</div>"; //div -1
	nav << "</body></html>";
}

void SentryPages::pushHeader(IHttpResponse& out, const char* templatename)
{
	if (NULL == templatename)
	{
		// change to default header
		templatename = "default";
	}

	std::string hdrpath = _htmldir + "layout" + FNSEPS + templatename + ".hdr";
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "pushHeader() loading header file[%s]"), hdrpath.c_str());

	std::ifstream header(hdrpath.c_str());

	char ch;
	while (header.get(ch))
	{
		out << ch;
	}
}

void SentryPages::pushFooter(IHttpResponse& out, const char* templatename)
{
	//ignore the template name
	std::string ftrpath = _htmldir + "layout" + FNSEPS + "footer.ftr";
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "pushHeader() loading footer file[%s]"), ftrpath.c_str());

	std::ifstream footer(ftrpath.c_str());

	char ch;
	while (footer.get(ch))
	{
		out << ch;
	}
}

SentryPages::Page SentryPages::_pageTbl[] =
{
	{ "nav", &SentryPages::navPage },
	{ "localhost", &SentryPages::localHostPage },
	{ "services", &SentryPages::localServicesPage },
	{ "processes", &SentryPages::localProcessesPage },
	{ "interfaces", &SentryPages::localInterfacesPage },
	{ "neighbors", &SentryPages::neighborsPage },
	{ "neighborsmap", &SentryPages::neighborsMapPage },
	{ "globalsvc", &SentryPages::globalServicePage },
	{ "proxy", &SentryPages::proxyPage } // implemented in WebProxy.cpp
};

void SentryPages::SystemPage(HttpRequestCtx * pHttpRequestCtx)
{
	if (NULL == pHttpRequestCtx)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SentryPages, "null request context handle."));
		return;
	}

	std::string uri = pHttpRequestCtx->GetUri();
	const std::string proc = uri.substr(0, uri.find_last_of('.'));

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryPages, "SystemPage() request uri[%s] proc[%s]"), uri.c_str(), proc.c_str());

	for (size_t i = 0; i < sizeof(_pageTbl) / sizeof(_pageTbl[0]); ++i)
	{
		if (0 == stricmp(_pageTbl[i].name, proc.c_str()))
		{
			(this->*_pageTbl[i].func)(pHttpRequestCtx);
			return;
		}
	}

	//unknown procedure
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SentryPages, "unknown procedure call.[%s]"), proc.c_str());
		pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_NOT_FOUND;
	}
}

}} // namespace
