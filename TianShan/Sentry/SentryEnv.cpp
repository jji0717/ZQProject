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
// Ident : $Id: SentryEnv.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Sentry/SentryEnv.cpp $
// 
// 16    3/16/16 10:53a Ketao.zhang
// 
// 15    4/01/15 1:26p Build
// cleaned old snmp
// 
// 14    3/19/15 9:58p Zhiqiang.niu
// use ZQSnmp instead of old snmp
// 
// 13    3/19/14 1:58p Zonghuan.xiao
// rollback
// 
// 11    12/20/12 10:58a Hongquan.zhang
// 
// 10    9/06/12 6:12p Hongquan.zhang
// 
// 9     9/06/12 5:49p Zonghuan.xiao
// convert  hostname  to  ip  if  host  is  not  ip  address
// 
// 7     8/16/12 4:24p Zonghuan.xiao
// change  get method from  const  to no-const
// 
// 6     8/09/12 8:00p Zonghuan.xiao
// add  snmp table logs
// 
// 5     7/31/12 1:04p Zonghuan.xiao
// refector table of TianShan Modules(Sentry)  Oid =
// .1.3.6.1.4.1.22839.4.1.1100.3.1.1.1
// 
// 4     7/30/12 3:24p Zonghuan.xiao
// refector  table of TianShan Modules(Sentry)  Oid =
// .1.3.6.1.4.1.22839.4.1.1100.3.1.1.1
// add  column IP
// 
// 3     7/27/12 4:55p Zonghuan.xiao
// implement  table of TianShan Modules(Sentry)  Oid =
// .1.3.6.1.4.1.22839.4.1.1100.3.1.1.1
// 
// 2     5/11/12 2:13p Hui.shao
// added config <http idleTimeout="300000" >
// 
// 2     5/11/12 2:07p Hui.shao
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 30    10-10-15 17:46 Fei.huang
// + keep unique guid in TianShan.xml
// * fix bug: 13395
// 
// 29    10-09-17 17:43 Fei.huang
// 
// 28    10-07-16 17:39 Fei.huang
// * throw exception when init fails instead of just exit without flushing
// the log file
// 
// 27    09-12-16 15:31 Fei.huang
// 
// 27    09-12-08 12:58 Fei.huang
// * fix memory leak in reading config
// * fix log format error
// + seperate impl for setenv
// 
// 26    09-12-01 13:51 Xiaohui.chai
// add initialization progress info
//
// 25    09-11-24 15:09 Fei.huang
// * move Neighbor config from TianShan.xml to Sentry.xml
// 
// 24    09-11-23 15:17 Xiaohui.chai
// move several conf item from Registry to xml file: GroupBind,
// GroupAddress, GroupPort
// 
// 23    09-11-17 16:35 Xiaohui.chai
// move the config item WebBindAddr to xml:
// /TianShan/Sentry/http/@bindAddr
// 
// 22    09-11-12 11:27 Fei.huang
// * bug#10476
// + complete os info
// 
// 21    09-11-11 16:18 Fei.huang
// * move Neighborhood config to TianShan.xml
// + mem info added
// 
// 20    09-07-27 12:43 Yixin.tian
// 
// 19    09-07-06 14:33 Fei.huang
// * linux port
// 
// 18    08-06-10 11:31 Xiaohui.chai
// added machine type
// 
// 17    08-04-07 14:18 Xiaohui.chai
// new config loader
// 
// 16    08-03-25 11:44 Xiaohui.chai
// 
// 15    08-02-28 16:54 Xiaohui.chai
// 
// 14    08-01-29 14:22 Xiaohui.chai
// 
// 13    08-01-22 18:00 Xiaohui.chai
// 
// 12    07-12-17 17:56 Xiaohui.chai
// 
// 11    07-12-14 16:18 Xiaohui.chai
// 
// 10    07-11-06 18:34 Xiaohui.chai
// 
// 9     07-11-05 15:59 Xiaohui.chai
// 
// 8     07-10-19 18:12 Xiaohui.chai
// 
// 7     07-09-18 12:56 Hongquan.zhang
// 
// 6     07-07-19 17:29 Hongquan.zhang
// 
// 5     07-06-07 12:30 Hui.shao
// 
// 4     07-06-04 14:45 Hui.shao
// separated html pages from env
// 
// 3     07-05-29 18:47 Hui.shao
// rewrote the ZQAdapter
// 
// 2     07-05-22 17:30 Hui.shao
// added exporting logger information
// ===========================================================================

#include <boost/thread.hpp>
#include "SentryEnv.h"
#include "Log.h"
#include "Guid.h"
#include "../LogPaserManagement.h"
#include "SentryConfig.h"
#include "XMLPreferenceEx.h"
#ifdef ZQ_OS_LINUX
#include <sys/utsname.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
extern ZQ::common::XMLPreferenceEx* getPreferenceNode(const std::string& path, ZQ::common::XMLPreferenceDocumentEx& config);
#endif

namespace ZQTianShan {
namespace Sentry {

typedef ::std::vector< Ice::Identity > IdentCollection;

// -----------------------------
// class SentryEnv
// -----------------------------
SentryEnv::SentryEnv(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool,
					 Ice::CommunicatorPtr& communicator, const char* endpoint, const char* databasePath)
: _groupPort(DEFAULT_GROUPPORT_Sentry), _timeout(DEFAULT_GROUP_TIMEOUT),
  _thpool(threadPool), _webPort(0), _communicator(communicator), _loopbackPort(0), _log(log), _pages(*this, NULL)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "progress: set web root directory as: %s"), _strWebRoot.c_str());
    {
        // initialize http settings with service's config
        _webPort =(unsigned short) gSentryCfg.lHttpServePort;
        _strWebRoot = gSentryCfg.szWebRoot;
        if(!_strWebRoot.empty())
        {
            if(FNSEPC != _strWebRoot[_strWebRoot.size() - 1])
                _strWebRoot.append(FNSEPS);
        }
        _pages.setHomeDir(_strWebRoot.c_str());
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "progress: set web root directory as: %s"), _strWebRoot.c_str());
        _strDllConfig = gSentryCfg.szWebLayoutConfig;
        _httpConnIdleTimeout  = gSentryCfg.httpConnIdleTimeout;
	    _reference = gSentryCfg.webRefs;
    }
	_groupAddr.setAddress(DEFAULT_GROUPADDR_Sentry);
	_endpoint = (endpoint && strlen(endpoint)>0) ? endpoint : DEFAULT_ENDPOINT_Sentry;

	if (_httpConnIdleTimeout < 10*1000)
		_httpConnIdleTimeout = 10*1000;

	char  szBuf[MAX_PATH];
	{
#ifdef ZQ_OS_MSWIN
		// initialize with Registry settings
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "initialize settings with registry key [%s]"), REG_KEY_Sentry);
		HKEY  hKey;
		DWORD dwType;
		DWORD dwSize;
		DWORD dwValue =0;
				
		if (ERROR_SUCCESS != ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, REG_KEY_Sentry, 0, KEY_READ|KEY_WRITE, &hKey))
		{
			sprintf(szBuf, "failed to open registry key [%s], exit", REG_KEY_Sentry);
			throw ZQ::common::Exception(szBuf);
		}

        dwValue = 0;
		dwSize = sizeof(dwValue); 
		if (ERROR_SUCCESS == ::RegQueryValueExA(hKey, "AdapterCollectorPort", NULL, &dwType, (LPBYTE)&dwValue, &dwSize) && REG_DWORD == dwType)
		_loopbackPort = dwValue;
		if (_loopbackPort <=0)
			_loopbackPort = LOOPBACK_DEFAULT_PORT;
#else
        ZQ::common::XMLPreferenceDocumentEx xmlDoc;
        ZQ::common::XMLPreferenceEx* pnode = 0;

		try {
			if(!xmlDoc.open(TIANSHAN_CONFIG)) {
				sprintf(szBuf, "failed to load config file [%s]", TIANSHAN_CONFIG);
				throw ZQ::common::Exception(szBuf);
            }

            pnode = getPreferenceNode("Sentry/server", xmlDoc);
            if(!pnode) {
				sprintf(szBuf, "failed to get server configuration");
				throw ZQ::common::Exception(szBuf);
            }
        }
		catch(ZQ::common::XMLException xmlex) {
			sprintf(szBuf, "open config file [%s] cathch a exception [%s]", TIANSHAN_CONFIG, xmlex.getString());
			pnode->free();
			throw ZQ::common::Exception(szBuf);
		}
		catch(...) {
			sprintf(szBuf, "open config file [%s] cathch a exception", TIANSHAN_CONFIG);
			pnode->free();
			throw ZQ::common::Exception(szBuf);
		}

        char value[128];
        memset(value, '\0', sizeof(value));

        bool res = pnode->getAttributeValue("AdapterCollectorPort", value);
		pnode->free();

        if(res) {
            _loopbackPort = atoi(value);
            if (_loopbackPort <=0) {
                _loopbackPort = LOOPBACK_DEFAULT_PORT;
            }
        }
#endif
        _groupPort = gSentryCfg.neighborGroupPort;
		if (_groupPort <=0)
			_groupPort = DEFAULT_GROUPPORT_Sentry;		        
        gSentryCfg.neighborGroupPort = _groupPort;

		{ // GroupAddress
			strcpy(szBuf, gSentryCfg.neighborGroupAddr.c_str());
			try {
				_groupAddr.setAddress(szBuf);
			}
			catch(...)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(SentryEnv, "illegal group address: %s, use %s instead"), szBuf, DEFAULT_GROUPADDR_Sentry);
				_groupAddr.setAddress(DEFAULT_GROUPADDR_Sentry);
			}
            gSentryCfg.neighborGroupAddr = _groupAddr.getHostAddress();
		}

		const char * UNSPEC_BIND = (_groupAddr.family() == PF_INET6) ? "::0" : "0.0.0.0";
		_groupBind.setAddress(UNSPEC_BIND);

		{ // GroupBind
			strcpy(szBuf, gSentryCfg.neighborGroupBind.c_str());
			try {
				_groupBind.setAddress(szBuf);
				if (_groupBind.family() != _groupAddr.family())
				{
					sprintf(szBuf, "Error: <bindIP> and <groupIP> are not in the same IP protocol family");
					throw ZQ::common::Exception(szBuf);
				}
			}
			catch(...)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(SentryEnv, "illegal bind address for group: %s, use %s instead"), szBuf, UNSPEC_BIND);
				_groupBind.setAddress(UNSPEC_BIND);
			}
            gSentryCfg.neighborGroupBind = _groupBind.getHostAddress();
		}

		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, 
                         "progress: Got groupBind(%s) groupAddress(%s) groupPort(%d)"), 
                         gSentryCfg.neighborGroupBind.c_str(), gSentryCfg.neighborGroupAddr.c_str(), _groupPort);

#ifdef ZQ_OS_MSWIN
		// prepare _selfInfo;
		dwSize = sizeof(szBuf) -2; 
		if (ERROR_SUCCESS == ::RegQueryValueExA(hKey, "NodeId", NULL, &dwType, (LPBYTE)szBuf, &dwSize) && REG_SZ == dwType)
		{
			szBuf[dwSize] = '\0';
			if (dwSize>6)
				_selfInfo.id = szBuf;
		}

		if (_selfInfo.id.empty())
		{
			// generate one and save it in the registry key
			ZQ::common::Guid id;
			id.create();
			id.toCompactIdstr(szBuf, sizeof(szBuf) -2);
			_selfInfo.id = szBuf;

			LONG ret = ::RegSetValueExA(hKey, "NodeId", 0, REG_SZ, (LPBYTE) szBuf, strlen(szBuf));
		}
#else
		memset(value, '\0', sizeof(value));
		pnode = getPreferenceNode("Sentry/server", xmlDoc);
		if(!pnode) {
			throw ZQ::common::Exception("failed to get server configuration");
		}
		res = false;
		res = pnode->getAttributeValue("NodeId", value);

        if(res) {
			if(strlen(value) > 6) {
				_selfInfo.id = value;
			}
        }
        if (_selfInfo.id.empty()) {
            ZQ::common::Guid id;
            id.create();
            id.toCompactIdstr(value, sizeof(value)-2);
            _selfInfo.id = value;

			pnode->setAttributeValue("NodeId", value);
			try {
				xmlDoc.save(TIANSHAN_CONFIG);
			} catch (ZQ::common::XMLException ex) {
				pnode->free();
				throw ZQ::common::Exception(ex.getString());
			}
        }
		pnode->free();
#endif
		gethostname(szBuf, sizeof(szBuf) - 2);
		_selfInfo.name = szBuf;
		
        _webBind.clear();
        
		{ // read the web bind address from xml
            strcpy(szBuf, gSentryCfg.webBindAddr.c_str());

            // validate the ip address
            try
            {
                ZQ::common::InetHostAddress localAddress;
                if(localAddress.setAddress(szBuf))
                {
                    _webBind = localAddress.getHostAddress();
                }
                else
                {
                    // use a local address
                    _webBind = ZQ::common::InetHostAddress::getLocalAddress().getHostAddress();
                    _log(ZQ::common::Log::L_WARNING, CLOGFMT(SentryEnv, 
                                "illegal bind address for http: %s, use %s instead"), szBuf, _webBind.c_str());
                }
            }
            catch (...)
            {
                _webBind.clear();
            }

		}

        if(_webBind.empty())
        {
            // use a local address
            _webBind = ZQ::common::InetHostAddress::getLocalAddress().getHostAddress();
        }
        gSentryCfg.webBindAddr = _webBind;

		if (_webPort <=0)
			_webPort = 80;
        _log(ZQ::common::Log::L_INFO, CLOGFMT(SentryEnv, "use http bind address: %s, port: %u"), _webBind.c_str(), _webPort);

        if(gSentryCfg.webPubAddr.empty())
        {
            gSentryCfg.webPubAddr = _webBind;
        }

		_selfInfo.adminRootUrl = std::string("http://") + gSentryCfg.webPubAddr +":" +itoa(_webPort, szBuf, 10) + "/";

		_selfInfo.sentrysvcPrx = SERVICE_NAME_Sentry ": ";
		_selfInfo.sentrysvcPrx += endpoint;
		::ZQTianShan::Adapter::appendServAddrs(_selfInfo.sentrysvcPrx);

		_selfInfo.lastChange = now();
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "progress: Self node information prepared"));
#ifdef ZQ_OS_MSWIN		
		::RegCloseKey(hKey);
#endif
	}

    _programRootPath = ZQTianShan::getProgramRoot();
    _programRootPath += FNSEPS;
	
    {
        // add TianShan\utils to search path
        std::string utilPath = _programRootPath + "utils";
        _log(ZQ::common::Log::L_DEBUG,CLOGFMT(SentryEnv,"add utils to search path.[%s]"), utilPath.c_str());

        const char *searchPath = getenv("PATH");
#ifdef ZQ_OS_MSWIN
        std::string pathEnvString = "PATH=";
        pathEnvString += (searchPath ? searchPath : "");
        pathEnvString += PHSEPS;
        pathEnvString += utilPath;
        if(0 != putenv(const_cast<char*>(pathEnvString.c_str())))
        {
            _log(ZQ::common::Log::L_ERROR,CLOGFMT(SentryEnv,"fail to add utils to search path.[%s]"), pathEnvString.c_str());
        }
#else
        std::string pathEnv = std::string(searchPath) + PHSEPS + utilPath;
        setenv("PATH", pathEnv.c_str(), 1);
#endif
    }
    {
        // set the machine type
        _selfInfo.type = gSentryCfg.machineType;
    }
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "open adapter %s at \"%s\""), ADAPTER_NAME_Sentry, _endpoint.c_str());
	try
	{
        _adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_Sentry, _endpoint.c_str(), _log);
	}
	catch(Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR,CLOGFMT(SentryEnv,"Create adapter failed with endpoint=%s and exception is %s"),
							endpoint,ex.ice_name().c_str());
		throw ex;
	}

	// open the loopback adpater
	try 
	{
        std::ostringstream loopEndpoint;
        loopEndpoint<< "default -h localhost -p " << _loopbackPort;

		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "open loopback adapter at \"%s\""), loopEndpoint.str().c_str());
		_loopbackAdapter = _communicator->createObjectAdapterWithEndpoints("LoopbackAdapter", loopEndpoint.str());
	}
	catch(const ::Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(SentryEnv, "Failed to open loopback adapter at \"default -h localhost\": %s"), ex.ice_name().c_str());
		throw ex;
	}

	gatherStaticSystemInfo();
	refreshSystemUsage();

    _pages.prepareLayoutInfo();

	// open multicast receiver and sender
	_groupBarker   = new SentryBarker(*this);
	_groupListener = new SentryListener(*this);
	//_snmpTableAgent = new ZQ::Snmp::Subagent(1100 , 3 );
	//assert( _snmpTableAgent != NULL );

	//_snmpTableAgent->setLogger(&_log);

	//_snmpTableAgent->start();
	_groupListener->start();
	_groupBarker->start();
    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "progress: initialization complete"));
}

SentryEnv::~SentryEnv()
{
    if(_groupListener)
    {
        delete _groupListener;
        _groupListener = NULL;
    }
    if(_groupBarker)
    {
        delete _groupBarker;
        _groupBarker = NULL;
    }

	/*if (NULL != _snmpTableAgent)
	{
		delete _snmpTableAgent;
		_snmpTableAgent = NULL;
	}*/
	_adapter=NULL;
}

void SentryEnv::gatherStaticSystemInfo()
{
#ifdef ZQ_OS_MSWIN
	char buf[MAX_PATH];

	// cpu info
	try
	{
		SYSTEM_INFO sysinfo;
		::GetSystemInfo(&sysinfo);

		switch (sysinfo.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_INTEL:
			_selfInfo.cpu += "Intel"; break;
		case PROCESSOR_ARCHITECTURE_MIPS:
			_selfInfo.cpu += "MIPS"; break;
		case PROCESSOR_ARCHITECTURE_PPC:
			_selfInfo.cpu += "PowerPC"; break;
		case PROCESSOR_ARCHITECTURE_IA64:
			_selfInfo.cpu += "AMD64"; break;
		case PROCESSOR_ARCHITECTURE_ALPHA64:
			_selfInfo.cpu += "Alpha64"; break;
		default:
			_selfInfo.cpu += "Unknown"; break;
		}

		// Get the processor speed info.
		HKEY hKey;
		DWORD dataSize;

		// Check if the function has succeeded.
		if (ERROR_SUCCESS == ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey))
		{
			dataSize = sizeof (buf);
			if (ERROR_SUCCESS == ::RegQueryValueExA(hKey, "VendorIdentifier", NULL, NULL, (LPBYTE)buf, &dataSize))
				_selfInfo.cpu += std::string(" vendor=") + buf +";";

			DWORD data, dataSize = sizeof(data);
			if (ERROR_SUCCESS == ::RegQueryValueExA(hKey, "~MHz", NULL, NULL, (LPBYTE)&data, &dataSize))
				_selfInfo.cpuClockMHz = data;

			RegCloseKey (hKey);
		}

		_selfInfo.cpuCount = sysinfo.dwNumberOfProcessors;
//		_selfInfo.cpu += std::string(" pagesize:") + itoa(sysinfo.dwPageSize, buf, 10) +";";
	}
	catch(...) {}

	// os info
	try
	{
		OSVERSIONINFOEX osinfo;
		memset(&osinfo, 0, sizeof(osinfo));
		osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		
		if (::GetVersionEx((OSVERSIONINFO *)&osinfo))
		{
			osinfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if(::GetVersionEx((OSVERSIONINFO *)&osinfo))
			{
				switch(osinfo.dwPlatformId)
				{
				case VER_PLATFORM_WIN32_NT:
					_selfInfo.os += "Windows NT "; break;
				case VER_PLATFORM_WIN32_WINDOWS:
				default:
					_selfInfo.os += "Windows "; break;
				}
				_selfInfo.os += std::string("Version ") + itoa(osinfo.dwMajorVersion, buf, 10) + "." + itoa(osinfo.dwMinorVersion, buf, 10);
			}
		}

        // get OS startup time
        {
            LARGE_INTEGER cntr, freq;
            QueryPerformanceFrequency(&freq);
            QueryPerformanceCounter(&cntr);
            if(0 != freq.QuadPart)
            {
                _selfInfo.osStartup = now() - (cntr.QuadPart / freq.QuadPart * 1000);
            }
            else
            {
                _log(ZQ::common::Log::L_WARNING, CLOGFMT(SentryEnv, "failed to get performance frequency."));
                _selfInfo.osStartup = 0;
            }
        }
	}
	catch(...) {}
#else
    FILE* fd = fopen("/proc/cpuinfo", "r");
    if(!fd) {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(SentryEnv, "failed to read cpu info."));
        return;
    }
    const unsigned SIZE = 1024;
    char buf[SIZE];
    memset(buf, '\0', SIZE);

    short resource = 3;
    while(!feof(fd)) {
        fgets(buf, 1024, fd);
        char* p = strchr(buf, ':');
        if(!p) {
            break;
        }
        if(strstr(buf, "model name")) {
            _selfInfo.cpu = p+1;
            --resource;
        }
        else if(strstr(buf, "cpu MHz")) {
            _selfInfo.cpuClockMHz = atoi(p+1);
            --resource;
        }
        else if(strstr(buf, "cpu cores")) {
            _selfInfo.cpuCount = atoi(p+1);
            --resource;
        }
        
        if(!resource) break;
    }

    fclose(fd);

    fd = fopen("/etc/redhat-release", "r");
    if(!fd) {
        struct utsname n;
        int res = uname(&n);
        if(res < 0) {
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(SentryEnv, "failed to read release info."));
            return;
        }
        _selfInfo.os = n.sysname;
    }
    else {
        memset(buf, '\0', SIZE);
        fgets(buf, SIZE, fd);
        _selfInfo.os = buf;
        
        fclose(fd);
    }

    fd = fopen("/proc/uptime", "r");
    if(!fd) {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(SentryEnv, "failed to read system uptime."));
        return;
    }
     
    memset(buf, '\0', SIZE);
    fgets(buf, SIZE, fd);
    
    _selfInfo.osStartup = now() - atol(buf)*1000;
    
    fclose(fd);
#endif
}

void SentryEnv::refreshSystemUsage()
{
#ifdef ZQ_OS_MSWIN
	// Memory
	try
	{
		MEMORYSTATUS ms;
		memset(&ms, 0x00, sizeof(ms));
		ms.dwLength = sizeof(ms);
		::GlobalMemoryStatus(&ms);
		_selfInfo.memAvailPhys = ms.dwAvailPhys;
		_selfInfo.memTotalPhys = ms.dwTotalPhys;
		_selfInfo.memAvailVirtual = ms.dwAvailVirtual;
		_selfInfo.memTotalVirtual = ms.dwTotalVirtual;
	}
	catch(...) {}
#else
    int fd = open("/proc/meminfo", O_RDONLY);
    if(fd < 0) {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(SentryEnv, "failed to read memory info."));
        return;
    }

    char buf[1024];
    memset(buf, '\0', 1024);

    ssize_t bytes = read(fd, buf, sizeof(buf)-1);
    buf[bytes] = '\0';

    close(fd);

    char* pos = strstr(buf, "MemTotal");
    unsigned total = 0;
    if(pos) {
        pos += 9;
        sscanf(pos, "%u", &total);
    }

    pos = strstr(pos, "MemFree");
    unsigned free = 0;
    if(pos) {
        pos += 8;
        sscanf(pos, "%u", &free);
    }

    pos = strstr(pos, "Buffers");
    unsigned buffers = 0;
    if(pos) {
        pos += 8;
        sscanf(pos, "%u", &buffers);
    }

    pos = strstr(pos, "Cached");
    unsigned cached = 0;
    if(pos) {
        pos += 7;
        sscanf(pos, "%u", &cached);
    }

    pos = strstr(pos, "VmallocTotal");
    unsigned vmTotal = 0;
    if(pos) {
        pos += 13;
        sscanf(pos, "%u", &vmTotal);
    }

    pos = strstr(pos, "VmallocUsed");
    unsigned vmUsed = 0;
    if(pos) {
        pos += 12;
        sscanf(pos, "%u", &vmUsed);
    }

    _selfInfo.memAvailPhys = (free+buffers+cached)<<10;
    _selfInfo.memTotalPhys = total<<10; 
    _selfInfo.memAvailVirtual = (vmTotal-vmUsed)<<10;
    _selfInfo.memTotalVirtual = vmTotal<<10;

#endif
}

bool SentryEnv::gatherNodeMap(FatNodeMap & fatNodeMap)
{
	{
		::Ice::Long stampExp = now() - _timeout * 2000;

		ZQ::common::MutexGuard gd(_lockNeighbors);
		for (SentryEnv::NodeMap::iterator it = _neighbors.begin(); it != _neighbors.end(); it++)
		{
			if(it->second.lastHeartbeat < stampExp)
				continue;
			FatNodeInfo fatNodeInfo;
			fatNodeInfo.baseNodeInfo = it->second;
			fatNodeInfo.services.clear();
			fatNodeMap.insert(FatNodeMap::value_type(fatNodeInfo.baseNodeInfo.id, fatNodeInfo));
		}
	}
	{
		ZQ::common::MutexGuard gd(_lockRemoteServices);
		for (SentryEnv::RemoteServices::iterator it = _remoteServices.begin(); it < _remoteServices.end(); it++)
		{
			if (fatNodeMap.end() == fatNodeMap.find(it->nodeid))
				continue;

			fatNodeMap[it->nodeid].services.push_back(it->baseInfo);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*
using namespace ZQ::Snmp;
template<typename Type>
class ModulesVar: public ZQ::Snmp::IVariable
{
public:
	ModulesVar(Type var):val_(var){}

	~ModulesVar(){}

	virtual bool get(SmiValue& val, AsnType desiredType)
	{
		return smivalFrom(val, val_, desiredType);
	}

	virtual bool set(const SmiValue& val)
	{
		return smivalTo(val, val_);
	}

	virtual bool validate(const SmiValue& val) const
	{
		return true;
	}

private:
	Type val_;
};

// neighbors pages -> TianShan Modules

// ZQ::Snmp::TablePtr 	SentryEnv::initNeighborsModulesTable()
// {
// 	ZQ::Snmp::TablePtr tbModulesUsage(new ZQ::Snmp::Table());
// 
// 	tbModulesUsage->addColumn( 1,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//host name
// 	tbModulesUsage->addColumn( 2,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//Interface 
// 	tbModulesUsage->addColumn( 3,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//Adapter
// 	tbModulesUsage->addColumn( 4,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//Endpoint
// 	tbModulesUsage->addColumn( 5,	ZQ::Snmp::AsnType_Integer,		ZQ::Snmp::aReadOnly);//PID
// 	tbModulesUsage->addColumn( 6,	ZQ::Snmp::AsnType_Octets,	    ZQ::Snmp::aReadOnly);//Activated
// 	tbModulesUsage->addColumn( 7,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//IP
// 
// 	FatNodeMap fatNodeMap;
// 	gatherNodeMap(fatNodeMap);
// 	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "create modules table end, column[%d], fatNodeMap size[%d]"), 7, fatNodeMap.size());
// 
// 	int rowIndex = 1;
// 	for (FatNodeMap::iterator it =fatNodeMap.begin(); it != fatNodeMap.end(); it++)
// 	{
// 		FatNodeInfo& nodeinfo = it->second;
// 		if(nodeinfo.services.empty())
// 			continue;
// 
// 		for (::ZqSentryIce::ServiceInfos::iterator itVector = nodeinfo.services.begin();
// 			itVector < nodeinfo.services.end();
// 			++itVector, ++rowIndex)
// 		{
// 			char tempbuf[80];
// 			memset(tempbuf, 0, sizeof(tempbuf));
// 			std::string  interfaces (itVector->name);
// 			std::string  adapterId (itVector->adapterId);
// 			int          processId (itVector->processId);
// 			std::string  endpoint  (itVector->proxystr);
// 			std::string  lastChange(ZQTianShan::TimeToUTC(nodeinfo.baseNodeInfo.lastChange, tempbuf, sizeof(tempbuf)-2));
//            
// 			int          offEndpoint =  endpoint.find("-h") + 2;
// 		    int          countIp     = endpoint.find("-p") - offEndpoint - 1;
// 			std::string  ipSubEndpoint(endpoint.substr(offEndpoint, countIp));
// 			if( gSentryCfg.reverseDomainName >= 1 )
// 			{
// 				uint64 startReverse = ZQ::common::now();
// 				struct hostent* host = gethostbyname(ipSubEndpoint.c_str());
// 				uint64 timeDelta = ZQ::common::now() - startReverse;
// 				if( timeDelta > 0 )
// 				{
// 					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv,"it took us [%llu]ms to reverse domain name [%s] to ip addr"),
// 						timeDelta,ipSubEndpoint.c_str() );
// 				}
// 				if(host && host->h_length > 0 && host->h_addr != 0 )
// 				{
// 					struct in_addr in = {0};
// 					memcpy((void*)&in,(void*)host->h_addr,sizeof(in));
// 					ipSubEndpoint = inet_ntoa(in);
// 				}				
// 			}
// 
// 			ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex( rowIndex );
// 
// 			tbModulesUsage->addRowData( 1, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(nodeinfo.baseNodeInfo.name) ));//host name
// 			tbModulesUsage->addRowData( 2, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(interfaces) ));				//Interface
// 			tbModulesUsage->addRowData( 3, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(adapterId) ));			//Adapter
// 			tbModulesUsage->addRowData( 4, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(endpoint) ));			//Endpoint
// 			tbModulesUsage->addRowData( 5, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<int>(processId) ));					//PID
// 			tbModulesUsage->addRowData( 6, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(lastChange) ));		//Activated
// 			tbModulesUsage->addRowData( 7, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(ipSubEndpoint) ));			//IP
// 		}
// 	}
// 
// 	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "create modules table end, column[%d], row[%d], fatNodeMap size[%d]"), 7, rowIndex, fatNodeMap.size());
// 	return tbModulesUsage;
// }

//bool SentryEnv::refreshNeighborsModulesTable()
//{	
//	ZQ::Snmp::Module& mod = _snmpTableAgent->module();
//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "refresh modules table"));
//	mod.remove(ZQ::Snmp::Oid("1.1.1"));//clear and remove table from snmplib
//
//	return _snmpTableAgent->addObject( ZQ::Snmp::Oid("1.1.1"), initNeighborsModulesTable() );//refresh table
//}

// tianshan  site cope -> neighbors -> participant machine

// ZQ::Snmp::TablePtr 	SentryEnv::initParticipantMachineTable()
// {
// 	//initialize sentry Usage Table
// 	ZQ::Snmp::TablePtr tbParticipantMachineUsage(new ZQ::Snmp::Table());
// 
// 	enum TableColumn
// 	{
//       HOST_NAME = 1,
// 	  NODE_ID,
// 	  ROOT_URL,
// 	  PROCESSOR,
// 	  MEMORY,
// 	  OS_VERSION,
// 	  STARTUP,
// 	  TABLE_COLUNM_COUNT
// 	};
// 
// 	tbParticipantMachineUsage->addColumn( HOST_NAME,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//host name
// 	tbParticipantMachineUsage->addColumn( NODE_ID,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//NodeId 
// 	tbParticipantMachineUsage->addColumn( ROOT_URL,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//rootURL
// 	tbParticipantMachineUsage->addColumn( PROCESSOR,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//Processor
// 	tbParticipantMachineUsage->addColumn( MEMORY,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//Memory
// 	tbParticipantMachineUsage->addColumn( OS_VERSION,	ZQ::Snmp::AsnType_Octets,	    ZQ::Snmp::aReadOnly);//OS
// 	tbParticipantMachineUsage->addColumn( STARTUP,	ZQ::Snmp::AsnType_Octets,	    ZQ::Snmp::aReadOnly);//Startup
// 
// 	FatNodeMap fatNodeMap;
// 	gatherNodeMap(fatNodeMap);
// 
// 	int rowIndex = 1;
// 	for (FatNodeMap::iterator it = fatNodeMap.begin();
// 		it != fatNodeMap.end(); ++it)
// 	{
// 		char tempbuf[80];
// 		memset(tempbuf, 0, sizeof(tempbuf));
// 		FatNodeInfo& NodeInfoMap = it->second;
// 		std::ostringstream out;
// 		std::string    memory;
// 		std::string	   processor;
// 		std::string    nodeId(NodeInfoMap.baseNodeInfo.id );
// 		std::string    hostName(NodeInfoMap.baseNodeInfo.name);
// 		std::string    rootURL((NodeInfoMap.baseNodeInfo).adminRootUrl);
// 		std::string    OS(NodeInfoMap.baseNodeInfo.os); 
// 		std::string    startUp(ZQTianShan::TimeToUTC(NodeInfoMap.baseNodeInfo.osStartup, tempbuf, sizeof(tempbuf)-2));
// 
// 		out<< NodeInfoMap.baseNodeInfo.memTotalPhys/1024/1024 << "MB / " << NodeInfoMap.baseNodeInfo.memTotalVirtual/1024/1024 << "MB";
// 		memory = out.str();
// 		out.str("");
// 
// 		out<< NodeInfoMap.baseNodeInfo.cpu << "  " << NodeInfoMap.baseNodeInfo.cpuClockMHz << "MHz x" << (NodeInfoMap.baseNodeInfo).cpuCount ;
// 		processor = out.str();
// 		out.str("");
// 
// 		ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex( rowIndex );
// 
// 		tbParticipantMachineUsage->addRowData( HOST_NAME, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(hostName) ));//host name
// 		tbParticipantMachineUsage->addRowData( NODE_ID, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(nodeId) ));//NodeId
// 		tbParticipantMachineUsage->addRowData( ROOT_URL, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(rootURL) ));//rootURL
// 		tbParticipantMachineUsage->addRowData( PROCESSOR, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(processor) ));//Processor
// 		tbParticipantMachineUsage->addRowData( MEMORY, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(memory) ));//Memory
// 		tbParticipantMachineUsage->addRowData( OS_VERSION,	indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(OS) ));//OS
// 		tbParticipantMachineUsage->addRowData( STARTUP,	indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(startUp) ));//Startup
// 
// 		rowIndex++;
// 	}
// 
// 	return tbParticipantMachineUsage;
// }
// 

//bool SentryEnv::refreshParticipantMachineTable()
//{	
//	ZQ::Snmp::Module& mod = _snmpTableAgent->module();
//	mod.remove(ZQ::Snmp::Oid("12.1.1"));//clear and remove table from snmplib
//
//	return _snmpTableAgent->addObject( ZQ::Snmp::Oid("12.1.1"), initParticipantMachineTable() );//refresh table
//}
*/

}} // namespace
	
