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
// Ident : $Id: ZqAdapter.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/ZqAdapter.cpp $
// 
// 12    1/11/16 5:37p Dejian.fei
// 
// 11    2/03/15 2:36p Hui.shao
// 
// 8     9/11/14 5:23p Hongquan.zhang
// 
// 7     6/16/14 4:24p Build
// 
// 6     6/06/14 3:25p Zonghuan.xiao
// remove optimize for adptrClctr in case segmentation fault 
// 
// 5     6/05/14 11:13a Zonghuan.xiao
// move AdapterBarker member adptrClctr to  run as  local variable
// 
// 4     6/04/14 3:45p Zonghuan.xiao
// enhance for bug#19032
// 
// 3     5/20/14 3:19p Hui.shao
// according to bug#19032, comment#2
// 
// 2     11/26/13 11:54a Zonghuan.xiao
// fix  adapterBarker  smart pointers circular references
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 24    10-10-14 13:26 Fei.huang
// * fix: use wrong node name of Sentry in TianShan.xml, which is not
// using uppercase any more   
// 
// 23    10-02-04 17:47 Yixin.tian
// add closeBarker() function
// 
// 22    09-12-09 12:11 Build
// modify include path
// 
// 21    09-02-18 10:01 Xiaohui.chai
// change log parsing scheme
// 
// 20    08-12-30 10:30 Yixin.tian
// 
// 19    08-12-05 17:14 Hui.shao
// 
// 18    08-11-10 11:39 Hui.shao
// use the primary adapter name as the display name
// 
// 17    08-03-20 11:59 Yixin.tian
// modify sem_trywait to sem_timedwait for linux os
// 
// 16    08-03-18 14:01 Hui.shao
// 
// 15    08-03-14 12:19 Fei.huang
// 
// 14    08-03-07 10:07 Yixin.tian
// WIN32 replaced by ZQ_OS_MSWIN
// 
// 13    08-03-06 16:51 Ken.qian
// set _gCallbackAdapter name to empty avoid acess violation run with
// ICE3.2
// 
// 12    08-03-06 12:50 Xiaohui.chai
// changed AdapterBarker::_adptrClctr to non-static member
// 
// 11    08-03-05 14:34 Yixin.tian
// merge for linux
// 
// 10    08-02-19 14:41 Hui.shao
// 
// 9     08-01-24 16:37 Xiaohui.chai
// 
// 8     07-11-06 16:17 Xiaohui.chai
// 
// 7     07-09-18 12:56 Hongquan.zhang
// 
// 6     07-06-15 18:01 Hongquan.zhang
// 
// 5     07-06-04 14:46 Hui.shao
// add log to Adapter
// 
// 4     07-05-29 18:47 Hui.shao
// rewrote the ZQAdapter
// 
// 3     07-05-22 17:30 Hui.shao
// added exporting logger information
// 
// 2     07-05-21 11:52 Hui.shao
// ===========================================================================

#include "ZQ_common_conf.h"
#include "TianShanDefines.h"
#include "TianShanIce.h"
#include "ZqSentryIce.h"
#include "Guid.h"
#include "TimeUtil.h"
#include "InetAddr.h"
#ifdef ZQ_OS_LINUX
#include <XMLPreferenceEx.h>
#endif

#ifndef _DEBUG
#  define DEFAULT_ADAPTER_HEARTBEAT (30*1000) // 30 sec
#else
#  define DEFAULT_ADAPTER_HEARTBEAT (5*1000)  // 5 sec
#endif // _DEBUG

namespace ZQTianShan {

// -----------------------------
// class AdapterBarker
// -----------------------------
class AdapterBarker : public ZqSentryIce::AdapterCB, public ZQ::common::NativeThread
{
public:
	AdapterBarker(Adapter& owner);

	~AdapterBarker();

    // impl of virtual AdapterCB
	virtual ::Ice::Long getActiveTime(const ::Ice::Current& c) { return _stampActive; }
    virtual ::std::string getEndpoint(const ::Ice::Current& c) { return _thisEndpoint; }
    virtual ::std::string getId(const ::Ice::Current& c) { return _adptrId; }
    virtual ::TianShanIce::StrValues listInterfaces(const ::Ice::Current& c);
    virtual ::ZqSentryIce::LoggerInfos listLoggers(const ::Ice::Current& c);

	::Ice::Long                 _stampActive;	
	ZqSentryIce::AdapterCBPrx	_thisPrx;
	Ice::ObjectAdapterPtr		_loopbackAdapter;
	Adapter&    				_owner;
	::Ice::Identity				_cbIdent;

	::std::string		  _thisEndpoint;
	::std::string		  _adptrId;
	::Ice::Long			  _lastUpdate;

	typedef struct _IntefaceInfo
	{
		std::string name;
		::Ice::Identity ifid;
		::Ice::ObjectPrx ifprx;
	} IntefaceInfo;

	typedef std::map <std::string, IntefaceInfo > InterfaceMap; // interface name to interfaceInfo map
	InterfaceMap _intfMap;

	static ::ZqSentryIce::LoggerInfos _gLoggerInfos;
	static ::ZQ::common::Mutex		  _gLoggerInfoLock;

	void quit();
	void wakeup()
	{
#ifdef ZQ_OS_MSWIN
		::SetEvent(_hWakeupEvent);
#else
		sem_post(&_semWatkeup);
#endif
	}

protected:

	virtual bool init(void);
	virtual int run(void);
	virtual void final(void);

	bool				  _bQuit;
#ifdef ZQ_OS_MSWIN
	HANDLE				  _hWakeupEvent;
#else
	sem_t				  _semWatkeup;
#endif

	ZQ::common::Log&	  _log;
};


#ifdef ZQ_OS_LINUX
bool GetConfigItem(std::map<std::string,std::string>& promap);
#endif
// -----------------------------
// class Adapter
// -----------------------------
::Ice::ObjectAdapterPtr		Adapter::_gCallbackAdapter;
Ice::Identity				Adapter::_gAdapterCBIdent;
uint32						Adapter::_gPid = 0;
::TianShanIce::StrValues	Adapter::_gTsNetIfs;

Adapter::Adapter(ZQ::common::Log& log, ::Ice::ObjectAdapterPtr& iceAdapter, const char* endpoint)
: _theAdapter(iceAdapter), _log(log), _pBarker(NULL)
{
#ifdef ZQ_OS_MSWIN
	if (_gPid <=0)
		_gPid =::GetCurrentProcessId();
#else
	if (_gPid <=0)
		_gPid =::getpid();
#endif

	
	_pBarker = new AdapterBarker(*this);
	_pBarker->_thisEndpoint = endpoint ? endpoint :"";
	_pBarker->_thisPrx      = NULL;

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "Adapter[%s@%d] created"), _theAdapter->getName().c_str(), _gPid);

	if (!_pBarker->_thisEndpoint.empty() && _pBarker->_thisEndpoint.npos == _pBarker->_thisEndpoint.find(" -h "))
		appendServAddrs(_pBarker->_thisEndpoint);

	_log(ZQ::common::Log::L_INFO, CLOGFMT(Adapter, "Adapter[%s@%d] fixed up endpoint: %s"), _theAdapter->getName().c_str(), _gPid, _pBarker->_thisEndpoint.c_str());

	if (!_gCallbackAdapter)
	{
		// create the global callback adapter
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "Adapter[%s@%d] enable callback interface"), _theAdapter->getName().c_str(), _gPid);
		_gCallbackAdapter = _theAdapter->getCommunicator()->createObjectAdapter(""); // No name specified, otherwise run with Ice3.2 will cause memory access violation
		_gCallbackAdapter->activate();
	}
}

Adapter::~Adapter()
{
	if(_pBarker)
		_pBarker = NULL;
#ifdef _DEBUG
//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "Adapter[%s@%d] closed"), _theAdapter->getName().c_str(), _gPid);
#endif
}

void Adapter::closeBarker()
{
	_pBarker->quit();
}

bool Adapter::appendServAddrs(::std::string& endpoint)
{
	::TianShanIce::StrValues servNics = getServeAddress();
	bool ret =false;

	for (::TianShanIce::StrValues::iterator it = servNics.begin(); it < servNics.end(); it ++)
	{
		if (endpoint.npos == endpoint.find(*it))
		{
			endpoint += std::string(" -h ") + ::ZQ::common::InetAddress(it->c_str()).getHostAddress();
			ret = true;
		}
	}

	return ret;
}

bool Adapter::publishLogger(const char* logfilename, const char* logsyntaxfile, const char* syntaxKey, const char*logtype, const ::TianShanIce::Properties& ctx)
{
	if (NULL == logfilename || NULL == logsyntaxfile || NULL == syntaxKey)
		return false;

//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "publish log \"%s\" with syntax \"%s\""), logfilename, logsyntaxfile);

	std::string searchfor = logfilename; 
//#ifdef _WIN32
	std::transform(searchfor.begin(), searchfor.end(), searchfor.begin(), (int(*)(int)) tolower);
//#endif // _WIN32
	
	::ZQ::common::MutexGuard gd(AdapterBarker::_gLoggerInfoLock);
	::ZqSentryIce::LoggerInfos::iterator it = AdapterBarker::_gLoggerInfos.begin();
	for (; it < AdapterBarker::_gLoggerInfos.end(); it++)
	{
		std::string logfile = it->logFile;
//#ifdef _WIN32
		std::transform(logfile.begin(), logfile.end(), logfile.begin(), (int(*)(int)) tolower);
//#endif // _WIN32
		if (0 == searchfor.compare(logfile))
			break;
	}
	
	if (AdapterBarker::_gLoggerInfos.end() == it) 
	{
		// not exist before
		AdapterBarker::_gLoggerInfos.push_back(::ZqSentryIce::LoggerInfo());
		it = AdapterBarker::_gLoggerInfos.end();
		it --;
	}

	it->logFile = logfilename;
	it->syntaxFile = logsyntaxfile;
    it->syntaxKey = syntaxKey;
	it->logType = logtype?logtype:"";
    it->ctx = ctx;

	return true;
}

bool Adapter::unpublishLogger(const char* logfilename)
{
//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "unpublish log \"%s\""), logfilename);

	if (NULL == logfilename)
		return false;

	std::string searchfor = logfilename; 
//#ifdef _WIN32
	std::transform(searchfor.begin(), searchfor.end(), searchfor.begin(), (int(*)(int)) tolower);
//#endif // _WIN32
	
	::ZQ::common::MutexGuard gd(AdapterBarker::_gLoggerInfoLock);
	::ZqSentryIce::LoggerInfos::iterator it = AdapterBarker::_gLoggerInfos.begin();
	for (; it < AdapterBarker::_gLoggerInfos.end(); it++)
	{
		std::string logfile = it->logFile;
//#ifdef _WIN32
		std::transform(logfile.begin(), logfile.end(), logfile.begin(), (int(*)(int)) tolower);
//#endif // _WIN32
		if (0 == searchfor.compare(logfile))
			break;
	}
	
	if (AdapterBarker::_gLoggerInfos.end() == it)
		return false;

	AdapterBarker::_gLoggerInfos.erase(it);
	
	return true;
}

Adapter::Ptr Adapter::create(ZQ::common::Log& log, ::Ice::CommunicatorPtr& communicator, const char* name, const char* endpoint)
{
	if (NULL == name || NULL == endpoint)
		return NULL;

	log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "create adapter \"%s\" with endpoint \"%s\""), name, endpoint);

	Ice::ObjectAdapterPtr ptr = communicator->createObjectAdapterWithEndpoints(name, endpoint);
	return new Adapter(log, ptr, endpoint);
}

const ::TianShanIce::StrValues& Adapter::getServeAddress()
{
#ifdef ZQ_OS_MSWIN
	if (_gTsNetIfs.size() <=0)
	{
		// read the TianShan serving NIC address(es)
		DWORD dwType;
		CHAR  szBuf[MAX_PATH];
		DWORD dwSize = sizeof(szBuf) -2;
		HKEY  hKey;
		
		if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_Sentry, 0, KEY_READ, &hKey))
		{
			if (ERROR_SUCCESS == ::RegQueryValueExA(hKey, "ServeAddress", NULL, &dwType, (LPBYTE)szBuf, &dwSize) && REG_SZ == dwType)
			{
				CHAR* p = szBuf, *q;
				for (q = strchr(p, ' '); q !=NULL; p = q+1, q = strchr(p, ' '))
				{
					*q = '\0';
					if (strlen(p) >0)
						_gTsNetIfs.push_back(p);
				}

				if (strlen(p) >0)
					_gTsNetIfs.push_back(p);
			}
					
			::RegCloseKey(hKey);
		}
	}
#else
	if (_gTsNetIfs.size() <=0)
	{
		// read the TianShan serving NIC address(es)
		char  szBuf[MAX_PATH] = {0};
		
		std::map<std::string,std::string> propmap;
		GetConfigItem(propmap);			
		strcpy(szBuf,propmap["ServeAddress"].c_str());

		if(strlen(szBuf) > 0)
		{
			char* p = szBuf, *q;
			for (q = strchr(p, ' '); q !=NULL; p = q+1, q = strchr(p, ' '))
			{
				*q = '\0';
				if (strlen(p) >0)
					_gTsNetIfs.push_back(p);
			}

			if (strlen(p) >0)
				_gTsNetIfs.push_back(p);
		}
		else
		{
#ifdef _DEBUG
//			fprintf(stderr,"Adapter::getServeAddress() not get ServeAddress value");
#endif
		}
	}
#endif

	return _gTsNetIfs;
}

::Ice::ObjectPrx Adapter::add(const ::Ice::ObjectPtr& intf, const ::std::string& interfaceName)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "Adapter[%s@%d] add interface \"%s\""), _theAdapter->getName().c_str(), _gPid, interfaceName.c_str());

	AdapterBarker::IntefaceInfo ifinfo;
	ifinfo.name = interfaceName;
	ifinfo.ifid = _theAdapter->getCommunicator()->stringToIdentity(ifinfo.name);
	ifinfo.ifprx = _theAdapter->add(intf, ifinfo.ifid);

	if (ifinfo.ifprx)
		_pBarker->_intfMap.insert(std::make_pair<std::string, AdapterBarker::IntefaceInfo>(ifinfo.name, ifinfo));

	_pBarker->_lastUpdate = now();
	_pBarker->wakeup();

	return ifinfo.ifprx;
}

void Adapter::activate()
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "Adapter[%s@%d] active"), _theAdapter->getName().c_str(), _gPid);
	_theAdapter->activate();
	_pBarker->_stampActive = _pBarker->_lastUpdate = now();
	_pBarker->start();
}

Ice::Long Adapter::getActivateTime() const
{
	return _pBarker->getActiveTime(::Ice::Current());
}

// -----------------------------
// class AdapterBarker
// -----------------------------
::ZqSentryIce::LoggerInfos			AdapterBarker::_gLoggerInfos;
::ZQ::common::Mutex					AdapterBarker::_gLoggerInfoLock;
//ZqSentryIce::AdapterCollectorPrx	AdapterBarker::_adptrClctr;

AdapterBarker::AdapterBarker(Adapter& owner)
: _owner(owner), _bQuit(false), _log(owner._log)
{
	_stampActive = _lastUpdate = now();

	if (_owner._theAdapter)
		_adptrId = _owner._theAdapter->getName(); // + "_B";

	if (_adptrId.empty())
	{
		char buf[32];
		ZQ::common::Guid id;
		id.create();
		id.toCompactIdstr(buf, sizeof(buf));
		_adptrId = buf;
	}
	
	_cbIdent.name =IceUtil::generateUUID();
	_cbIdent.category = "AdapterCB";
#ifdef ZQ_OS_MSWIN
	 _hWakeupEvent = NULL;
	_hWakeupEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	sem_init(&_semWatkeup,0,0);
#endif
}

AdapterBarker::~AdapterBarker()
{
	quit();
}

void AdapterBarker::quit()
{
	if(!_bQuit)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(AdapterBarker, "AdapterBarker[%s@%d] quit"), _adptrId.c_str(), Adapter::_gPid);
		try {
			if (_owner._gCallbackAdapter)
				_owner._gCallbackAdapter->remove(_cbIdent);
		}
		catch (const Ice::ObjectAdapterDeactivatedException& ) {}
		catch (const Ice::Exception& ) {}
		catch (...) {}

		_bQuit = true;
		wakeup();
		//::Sleep(1);
		waitHandle(-1);

#ifdef ZQ_OS_MSWIN
		if(_hWakeupEvent)
			::CloseHandle(_hWakeupEvent);
		_hWakeupEvent = NULL;
#else
		sem_destroy(&_semWatkeup);
#endif
	}
}

bool AdapterBarker::init(void)
{
	if (! _owner._gCallbackAdapter)
	{
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(Adapter, "Adapter[%s@%d] global adapter hasn't been initialized, barker stopped"), _adptrId.c_str(), Adapter::_gPid);
		return false;
	}

	try
	{
		_owner._gCallbackAdapter->add(_owner._pBarker, _cbIdent);
	}
	catch(...)
	{
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(Adapter, "Adapter[%s@%d] failed to add callback into the global adapter, barker stopped"), _adptrId.c_str(), Adapter::_gPid);
		return false;
	}

	return true;
}

int AdapterBarker::run(void)
{
	ZqSentryIce::AdapterCollectorPrx  adptrClctr = NULL;
	long                              sleepTime  = -1;
	
	while (!_bQuit)
	{
#ifdef ZQ_OS_MSWIN
		::WaitForSingleObject(_hWakeupEvent, sleepTime);
#else
		struct timespec ts;
		struct timeval tmval;

		gettimeofday(&tmval, (struct timezone*)NULL);
		int64 nmicrosec = sleepTime * 1000ll + tmval.tv_usec;
		ts.tv_sec = tmval.tv_sec + nmicrosec / 1000000;
		ts.tv_nsec = (nmicrosec % 1000000) * 1000;
		sem_timedwait(&_semWatkeup, &ts);
#endif // ZQ_OS_MSWIN

		if (_bQuit)
			break;

		if (_owner._theAdapter)
		{
			if (!adptrClctr)
			{
				// no connection has been established to the SentryService

				int port = 0;

				// step 1. read the loopback port of AdapterCollector from the registry
#ifdef ZQ_OS_MSWIN
				DWORD dwType;
				DWORD dwValue = 0;
				DWORD dwSize = sizeof(dwValue);
				HKEY  hKey;

				if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_Sentry, 0, KEY_READ, &hKey))
				{
					//no sentry is found, skip rest
					::WaitForSingleObject(_hWakeupEvent, DEFAULT_ADAPTER_HEARTBEAT);
					continue;
				}

				dwSize = sizeof(dwValue);
				if (ERROR_SUCCESS == ::RegQueryValueEx(hKey, "AdapterCollectorPort", NULL, &dwType, (LPBYTE)&dwValue, &dwSize) && REG_DWORD == dwType)
					port = dwValue;

				::RegCloseKey(hKey);
#else
				char  szBuf[20] = { 0 };

				std::map<std::string, std::string> propmap;
				if (!GetConfigItem(propmap))
				{
					struct timeval tmval;
					struct timespec ts;

					gettimeofday(&tmval, (struct timezone*)NULL);
					int64 nMicro = tmval.tv_usec + DEFAULT_ADAPTER_HEARTBEAT * 1000ll;
					ts.tv_sec = tmval.tv_sec + nMicro / 1000000;
					ts.tv_nsec = (nMicro % 1000000) * 1000;
					sem_timedwait(&_semWatkeup, &ts);
					continue;
				}

				strcpy(szBuf, propmap["AdapterCollectorPort"].c_str());
				if (strlen(szBuf) > 0)
					port = atoi(szBuf);
#endif // OS
				if (port <= 0)
					port = LOOPBACK_DEFAULT_PORT;

				try {
					char buf[64];
					sprintf(buf, SERVICE_NAME_AdapterCollector ": tcp -h localhost -p %d", port);

					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "Adapter[%s@%d] establishing connection to local AdapterCollector at: \"%s\""), _adptrId.c_str(), Adapter::_gPid, buf);
					
					// step 2. connect to the AdapterCollector
					::Ice::ObjectPrx prx= _owner._gCallbackAdapter->getCommunicator()->stringToProxy(buf);
					adptrClctr = ::ZqSentryIce::AdapterCollectorPrx::checkedCast(prx);

					// step 3. enable the bidirectional connections and register the callback
					adptrClctr->ice_getConnection()->setAdapter(_owner._gCallbackAdapter);
					int isOptOrig = adptrClctr->ice_isCollocationOptimized();
					if (isOptOrig)
					{// remove optimize in case segmentation fault
						Ice::ObjectPrx  objPrx = NULL;
						#if ICE_INT_VERSION / 100 >= 306
							objPrx      =  adptrClctr->ice_collocationOptimized(false);
						#else
							objPrx      =  adptrClctr->ice_collocationOptimization(false);
						#endif
						adptrClctr =  ::ZqSentryIce::AdapterCollectorPrx::uncheckedCast(objPrx);
					}

					int isOptFinal = adptrClctr->ice_isCollocationOptimized();
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "Adapter[%s@%d] establishing connection to local AdapterCollector succeedfully optimized[%d -> %d] at: \"%s\""),
						_adptrId.c_str(), Adapter::_gPid, isOptOrig, isOptFinal, buf);
				}
				catch (const ::Ice::ObjectAdapterDeactivatedException&)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(Adapter, "Adapter[%s@%d] ObjectAdapterDeactivatedException caught, quit."), _adptrId.c_str(), Adapter::_gPid);
					_bQuit = true;
				}
				catch (const ::Ice::CollocationOptimizationException&)
				{
					// appears the same process of AdapterCollector itself, quit
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(Adapter, "Adapter[%s@%d] CollocationOptimizationException caught, the adapter appears as in the same process of the AdapterCollector, quit."), _adptrId.c_str(), Adapter::_gPid);
					_bQuit = true;
				}
				catch (const ::Ice::Exception& ex)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(Adapter, "Adapter[%s@%d] register to local Sentry, exception caught: %s, force to reconnect at next time"), _adptrId.c_str(), Adapter::_gPid, ex.ice_name().c_str());
					adptrClctr = NULL;
				}
			}

			try {
				// update this adapter information

				if (adptrClctr && _owner._gCallbackAdapter == adptrClctr->ice_getConnection()->getAdapter())
				{
#ifdef _DEBUG
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Adapter, "Adapter[%s@%d] update the adapter info to SentrySvc"), _adptrId.c_str(), Adapter::_gPid);
#endif
					sleepTime = adptrClctr->updateAdapter(_owner._gPid, _adptrId, _lastUpdate, _cbIdent) *1000;
				}
				else
					adptrClctr = NULL; // force the reconnect
			}
			catch (const ::Ice::Exception& ex)
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(Adapter, "Adapter[%s@%d] update info, exception caught: %s, force to reconnect at next timer"), _adptrId.c_str(), Adapter::_gPid, ex.ice_name().c_str());
				adptrClctr = NULL; // force the reconnect
			}
		}

		if (sleepTime < 0)
			sleepTime = DEFAULT_ADAPTER_HEARTBEAT;
	} // while

	if (adptrClctr)
	{
		adptrClctr =  NULL;
	}

	_log(ZQ::common::Log::L_WARNING, CLOGFMT(Adapter, "Adapter[%s@%d] end of loop"), _adptrId.c_str(), Adapter::_gPid);

	return 0;
}

void AdapterBarker::final(void)
{
	_bQuit = true;
	_log(ZQ::common::Log::L_WARNING, CLOGFMT(Adapter, "Adapter[%s@%d] quit barker"), _adptrId.c_str(), Adapter::_gPid);
}

::TianShanIce::StrValues AdapterBarker::listInterfaces(const ::Ice::Current& c)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(AdapterCB, "listInterfaces(), adapter[%s@%d]"), _adptrId.c_str(), Adapter::_gPid);
	::TianShanIce::StrValues ret;

	for (InterfaceMap::iterator it = _intfMap.begin(); it != _intfMap.end(); it++)
		ret.push_back(it->first);

	return ret;
}

::ZqSentryIce::LoggerInfos AdapterBarker::listLoggers(const ::Ice::Current& c)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(AdapterCB, "listLoggers(), adapter[%s@%d]"), _adptrId.c_str(), Adapter::_gPid);
	::ZQ::common::MutexGuard gd(_gLoggerInfoLock);
	return _gLoggerInfos;
}

#ifdef ZQ_OS_LINUX
bool GetConfigItem(std::map<std::string,std::string>& promap)
{
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	bool bOpen = false;

	try
	{
		bOpen = xmlDoc.open(TIANSHAN_CONFIG);
	}
	catch(ZQ::common::XMLException xmlex)
	{
#ifdef _DEBUG
//		fprintf(stderr,"Adapter::getServeAddress GetConfigItem()");
#endif
		return false;
	}
	catch(...)
	{
#ifdef _DEBUG
//		fprintf(stderr,"Adapter::getServeAddress GetConfigItem()");
#endif
		return false;
	}
	if (!bOpen)
		return false;
	ZQ::common::XMLPreferenceEx* rootNode = xmlDoc.getRootPreference();
	if(!rootNode)
		return false;
	ZQ::common::XMLPreferenceEx* nodeItem = rootNode->firstChild("Sentry");
	if(!nodeItem)
		return false;
	else
	{
		ZQ::common::XMLPreferenceEx* serverItem = nodeItem->firstChild("server");
		if(!serverItem)
			return false;
		promap = nodeItem->getProperties();			

		serverItem->free();
	}
	nodeItem->free();
	rootNode->free();
	
	return true;
}
#endif

}

