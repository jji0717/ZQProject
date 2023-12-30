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
// Ident : $Id: ProvisionFactory.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ProvisionFactory.cpp $
// 
// 7     1/23/16 6:39p Li.huang
// 
// 7     1/23/16 6:37p Li.huang
// 
// 6     4/18/13 5:27p Li.huang
// fix bug 18035
// 
// 5     8/09/11 2:13p Li.huang
// add DirectIO
// 
// 4     3/21/11 2:17p Li.huang
// fix bug 13454 1#
// 
// 3     1/10/11 11:12a Li.huang
// 
// 2     10-12-15 14:13 Li.huang
// use new bufferpool
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 18    10-11-02 14:29 Li.huang
//  megre from 1.10
// 
// 17    09-12-29 18:52 Fei.huang
// 
// 16    09-12-29 18:36 Fei.huang
// * log error message using new interface of Shared Obj
// 
// 15    09-12-29 11:37 Fei.huang
// * log message if plugin loading failed
// 
// 14    09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 14    09-04-14 19:25 Jie.zhang
// improve the buffer queue manager performance
// 
// 13    08-12-16 11:12 Jie.zhang
// 
// 12    08-12-15 17:54 Yixin.tian
// merge for Linux OS
// 
// 11    08-11-18 11:09 Jie.zhang
// merge from TianShan1.8
// 
// 14    08-10-27 13:40 Xia.chen
// 
// 13    08-10-24 14:48 Jie.zhang
// fixed "cancelProvision" return error status
// 
// 12    08-08-21 18:18 Xia.chen
// 
// 11    08-08-21 18:11 Xia.chen
// 
// 10    08-06-19 18:58 Jie.zhang
// change memory free position
// 
// 9     08-06-02 16:39 Jie.zhang
// 
// 8     08-04-25 16:11 Jie.zhang
// 
// 7     08-04-09 11:48 Hui.shao
// added ProvisionCost
// 
// 6     08-03-27 16:53 Jie.zhang
// 
// 7     08-03-17 19:56 Jie.zhang
// 
// 6     08-03-07 18:13 Jie.zhang
// 
// 5     08-03-04 17:15 Jie.zhang
// 
// 4     08-02-22 18:59 Jie.zhang
// fixed the getConfigDir string issue
// 
// 3     08-02-15 12:23 Jie.zhang
// changes check in
// 
// 2     08-02-14 18:49 Hui.shao
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#include "ProvisionFactory.h"
#include "CPEEnv.h"
#include "CPEImpl.h"
#include "Log.h"
#include "QueueBufMgr.h"
#include "CPECfg.h"

extern "C"
{
#ifdef ZQ_OS_MSWIN
	#include <io.h>
#endif
};

namespace ZQTianShan {
namespace CPE {

ProvisionFactory::ProvisionFactory(CPEEnv& env)
: _env(env)
{
// 	_pAlloc = new QueueBufMgr();	
// 	_pAlloc->setLog(&env._log);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ProvisionFactory, "Mediasample  buffer[%d] maxBufferPoolSize[%d], minBufferPoolSize[%d]"),
		_env._mediasamplebuffer, _env._maxBufferPoolSize, _env._minBufferPoolSize);
	_pAlloc = new ZQ::common::BufferPool();
	_pAlloc->initialize(_env._mediasamplebuffer, _env._maxBufferPoolSize, _env._minBufferPoolSize, _env._alignTo);
}

ProvisionFactory::~ProvisionFactory()
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ProvisionFactory, "~ProvisionFactory() destroying all helper sessions"));
	std::vector <ZQTianShan::ContentProvision::ICPHSession* > helperSessions;
	{
		ZQ::common::MutexGuard g(_lockSessMap);
		for (HelperSessionMap::iterator it  = _sessMap.begin(); it != _sessMap.end(); it++)
		{
			if (NULL != it->second)
				helperSessions.push_back(it->second);
		}
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ProvisionFactory, "~ProvisionFactory() %d helper sessions to destroy"), helperSessions.size());
	for (int i =0; i< (int)helperSessions.size(); i++)
	{
		try {
			helperSessions[i]->terminate(false);
		}catch(...) {}
	}

	int nWaited = 0;
	int nSessionCount=0;

#define MAX_WAIT_EXIT_MS	50000
	if (_gCPECfg._dwMaxWaitMsForQuit>MAX_WAIT_EXIT_MS)
		_gCPECfg._dwMaxWaitMsForQuit= MAX_WAIT_EXIT_MS;

	while(nWaited<_gCPECfg._dwMaxWaitMsForQuit)
	{
#ifdef ZQ_OS_MSWIN
		::Sleep(500);
#else
		usleep(500*1000);
#endif
		nWaited += 500;

		ZQ::common::MutexGuard g(_lockSessMap);
		nSessionCount = _sessMap.size();
		if (!nSessionCount)
			break;
	}
	if (nWaited>=_gCPECfg._dwMaxWaitMsForQuit)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ProvisionFactory, "~ProvisionFactory() wait for running session time out, alive session %d"),
			nSessionCount);
	}

	if (_pAlloc)
	{
// 		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ProvisionFactory, "Buffer queue available[%d], total[%d]"), 
// 			_pAlloc->getAvailableSize(), _pAlloc->getTotalSize());

		delete _pAlloc;
		_pAlloc = NULL;
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ProvisionFactory, "~ProvisionFactory() unloading helpers"));
	ZQ::common::MutexGuard sync(_lockSoMap);
	for (SoMap::iterator it = _soMap.begin(); it != _soMap.end(); it++)
	{
		if (!it->second)
			continue;

		try {
			ZQTianShan::ContentProvision::CPHFacet cphoFacet(*(it->second));
			cphoFacet.UninitCPH(this, (void*) (it->second));
		}catch(...) {}
		
		try{delete it->second;}catch (...) {}
	}
	_soMap.clear();
}

::TianShanIce::StrValues ProvisionFactory::listSupportedMethods()
{
	::TianShanIce::StrValues types;
	ZQ::common::MutexGuard gd(_lockMethodMap);
	for (MethodMap::const_iterator it = _methodMap.begin(); it != _methodMap.end(); it++)
	{
		if (it->first.empty() || NULL == it->second.pPlugin)
			continue;
		types.push_back(it->first);
	}

	return types;
}

ZQTianShan::ContentProvision::ICPHSession* ProvisionFactory::resolve(::TianShanIce::ContentProvision::ProvisionSessionExPtr pSess)
{
	if (!pSess)
		return NULL;

	{
		ZQ::common::MutexGuard g(_lockSessMap);
		
		HelperSessionMap::iterator it = _sessMap.find(pSess->ident.name);
		if( it != _sessMap.end())
			return it->second;
	}

	ZQTianShan::ContentProvision::ICPHelper* pHelper = findHelper(pSess->methodType.c_str());
	if (NULL == pHelper)
		return NULL;

	return pHelper->createHelperSession(pSess->methodType.c_str(), pSess);
}

ZQTianShan::ContentProvision::ICPHSession* ProvisionFactory::findHelperSession(const char* provisionId)
{
	ZQ::common::MutexGuard g(_lockSessMap);
	if (NULL ==provisionId)
		return NULL;

	HelperSessionMap::iterator it = _sessMap.find(provisionId);
	if( it == _sessMap.end())
		return NULL;
	
	return it->second;
}

bool ProvisionFactory::registerHelperSession(const char* provisionId, ZQTianShan::ContentProvision::ICPHSession* pSess)
{
	if (NULL == provisionId || NULL == pSess || strlen(provisionId) <=0)
		return false;

	ZQ::common::MutexGuard g(_lockSessMap);
	MAPSET(HelperSessionMap, _sessMap, provisionId, pSess);
//	_sessMap.insert(HelperSessionMap::value_type(provisionId, pSess));
	return true;
}

void ProvisionFactory::unregisterHelperSession(const char* provisionId)
{
	if (NULL == provisionId)
		return;
	
	ZQ::common::MutexGuard g(_lockSessMap);
	_sessMap.erase(provisionId);
}


ZQTianShan::ContentProvision::ICPHelper* ProvisionFactory::findHelper(const char* methodType)
{
	if (NULL == methodType || strlen(methodType) <=0)
		return NULL;

	ZQ::common::MutexGuard gd(_lockMethodMap);
	MethodMap::iterator it = _methodMap.find(methodType);
	if (_methodMap.end() == it)
		return NULL;

	return it->second.pPlugin;
}


int ProvisionFactory::populate()
{
	std::map<std::string, int>::iterator itPlugin;
	for( itPlugin = _gCPECfg.cphPlugins.begin(); 
		itPlugin!=_gCPECfg.cphPlugins.end();
		itPlugin++)
	{		
		if ((*itPlugin).second == 1)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(ProvisionFactory, "load plugIn %s"),(*itPlugin).first.c_str());
			loadPlugin((*itPlugin).first.c_str());
		}
	}
	return _soMap.size();
}

ZQ::common::DynSharedObj* ProvisionFactory::loadPlugin(const char* filename)
{
	if (NULL == filename)
		return NULL;

    ZQ::common::DynSharedObj* pPlugin = new ZQ::common::DynSharedObj(filename);
    if(!pPlugin) {
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ProvisionFactory, "%s"), pPlugin->getErrMsg());
        return NULL;
    }

	ZQ::common::MutexGuard sync(_lockSoMap);
	SoMap::iterator it = _soMap.find(pPlugin->getImageInfo()->filename);
	if (_soMap.end() != it)
	{
		delete pPlugin;
		return it->second;
	}
	
	bool bValidPlugin = false;
	try
	{
		ZQTianShan::ContentProvision::CPHFacet	cphFacet(*pPlugin);
		if (cphFacet.isValid())
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(ProvisionFactory, "InitCPH() %s"), filename);
			if (cphFacet.InitCPH(this, (void*) pPlugin))
			{
				bValidPlugin = true;
			}
			else
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ProvisionFactory, "loadPlugin() CPH [%s] failed to InitCPH()"), filename);
			  cphFacet.UninitCPH(this, (void*) pPlugin);
			}
		}
	}
	catch(const char*& szMsg)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ProvisionFactory, "loadPlugin() failed to load %s with error[%s]"), 
			pPlugin->getImageInfo()->filename, szMsg);
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ProvisionFactory, "loadPlugin() failed to load %s"), pPlugin->getImageInfo()->filename);
	}

	if (!bValidPlugin)
	{
		//sleep for a while to avoid crashing, only in windows we need it
#ifdef ZQ_OS_MSWIN
		::Sleep(2000);
#endif

		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ProvisionFactory, "invalid plugin [%s] error[%s]"), filename, pPlugin->getErrMsg());

		delete pPlugin;
		return NULL;
	}

	_soMap[pPlugin->getImageInfo()->filename] = pPlugin;
	return pPlugin;
}

ZQ::common::Log* ProvisionFactory::getLogger() const
{
	return &_env._log;
}

ZQ::common::NativeThreadPool& ProvisionFactory::getThreadPool() const
{
	return _env._thpool;
}

bool ProvisionFactory::registerHelper(const char* methodType, ZQTianShan::ContentProvision::ICPHelper* helper, void* pCtx)
{
	if (NULL == methodType)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ProvisionFactory, "registerHelper() NULL method"));
		return false;
	}

	if (NULL ==helper)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ProvisionFactory, "registerHelper() invalid parameters method:%s, helper[%08x], ctx[%08x]"), methodType, helper, pCtx);
		return false;
	}
		
	ZQ::common::MutexGuard gd(_lockMethodMap);
	MethodMap::iterator it = _methodMap.find(methodType);
	if (_methodMap.end() != it)
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ProvisionFactory, "registerHelper() overwritting method: %s, old helper[%08x@%s]"), methodType, it->second.pPlugin, it->second.fnPlugin.c_str());

	CPHNode node;
	node.pPlugin = helper;
	node.fnPlugin = pCtx ? ((::ZQ::common::DynSharedObj*) pCtx)->getImageInfo()->filename : "n/a";
	_methodMap.insert(MethodMap::value_type(methodType, node));
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ProvisionFactory, "registerHelper() registered method: %s, helper[%08x@%s]"), methodType, node.pPlugin, node.fnPlugin.c_str());

	return true;
}

bool ProvisionFactory::unregisterHelper(const char* methodType, void* pCtx)
{
	if (NULL == methodType)
		return false;

	std::string pluginFilename = pCtx ? ((::ZQ::common::DynSharedObj*) pCtx)->getImageInfo()->filename : "n/a";

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ProvisionFactory, "unregisterHelper() unregister method[%s], plugin: %s"), methodType, pluginFilename.c_str());
	
	ZQ::common::MutexGuard gd(_lockMethodMap);
	MethodMap::iterator it = _methodMap.find(methodType);
	if (_methodMap.end() != it)
	{
		_methodMap.erase(it);
		return true;
	}

	return false;
}

const char* ProvisionFactory::getConfigDir() const
{
	static std::string strConfigDir;
	strConfigDir=(_env._programRootPath + "etc" FNSEPS);
	return strConfigDir.c_str();
}

const char* ProvisionFactory::getLogDir() const
{
	static std::string strLogDir;
	strLogDir=(_env._programRootPath + "logs" FNSEPS);

	return strLogDir.c_str();
}

const char* ProvisionFactory::getMethodType(const char* provisionId) const
{
	if (NULL == provisionId || strlen(provisionId) <=0)
		return NULL;

	::Ice::Identity ident;
	ident.name = provisionId; ident.category = DBFILENAME_ProvisionSession;

	try
	{
		TianShanIce::ContentProvision::ProvisionSessionPrx sess = IdentityToObjEnv(_env, ProvisionSession, ident);
		if (sess)
			return sess->getMethodType().c_str();
	}
	catch (...) {}

	return NULL;
}

ZQTianShan::ContentProvision::IPushSource* ProvisionFactory::findPushSource(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey) const
{
	return _env._ftpServer.findPushSource(contentKey.contentStoreNetId.c_str(),
		contentKey.volume.c_str(),
		contentKey.content.c_str());
}

//IMemAlloc* ProvisionFactory::getMemoryAllocator()
ZQ::common::BufferPool* ProvisionFactory::getMemoryAllocator()
{
	return _pAlloc;
}
void  ProvisionFactory::registerLimitation(const char* methodName,int maxSession, int64 maxBw, const char*  groupId, uint32 flags)
{
	MethodOfGroupsMap::iterator itor;
	MethodInfos::const_iterator itorMethod;
	MethodInfo  regMethodinfo;
	regMethodinfo.methodName = methodName;
	regMethodinfo.maxSessions = maxSession;
	regMethodinfo.maxBw = maxBw;

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ProvisionFactory, 
		"registerLimitation()  groupId[%s] , method[%s], maxSessions[%d], maxBw[%lld]"), 
         groupId, methodName, maxSession, maxBw);

	ZQ::common::MutexGuard gd(_lockmethodofgroupsMap);
	itor = _methodofgroups.find(groupId);
	if(itor != _methodofgroups.end())
	{  
		for(itorMethod = itor->second.begin(); itorMethod != itor->second.end(); itorMethod++)
		{
			if((*itorMethod).methodName == methodName)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ProvisionFactory, "registerLimitation() duplicate groupId[%s], method[%s]"), groupId, methodName);
				return;
			}
		}
		itor->second.push_back(regMethodinfo);
	}
	else
	{
		MethodInfos methodinfos;
		methodinfos.push_back(regMethodinfo);
		MAPSET(MethodOfGroupsMap, _methodofgroups, groupId, methodinfos);
	}
}

const int  ProvisionFactory::getMaxSessioOfGroup(const char* groupId) const
{
   MethodOfGroupsMap::const_iterator itor;
   MethodInfos::const_iterator itorMethod;
   int maxSessions = 0;

   ZQ::common::MutexGuard gd(_lockmethodofgroupsMap);
   if(stricmp(groupId, "") == 0)
   {
	   for(itor = _methodofgroups.begin(); itor != _methodofgroups.end(); itor++)
	   {
		   int methodmaxsession = 0;

		   itorMethod = itor->second.begin();
		   while(itorMethod != itor->second.end())
		   {
			   if(methodmaxsession < itorMethod->maxSessions)
				   methodmaxsession = itorMethod->maxSessions;
			   itorMethod++;
		   }
		   maxSessions +=  methodmaxsession;
	   }
   }
   else
   {
	   itor = _methodofgroups.find(groupId);
	   if(itor != _methodofgroups.end())
	   {
		   //find maxsession
		   itorMethod = itor->second.begin();
		   while(itorMethod != itor->second.end())
		   {
			   if(maxSessions < itorMethod->maxSessions)
				   maxSessions = itorMethod->maxSessions;
			   itorMethod++;
		   }
	   }
   } 
   envlog(ZQ::common::Log::L_INFO, CLOGFMT(ProvisionFactory, "getMaxSessioOfGroup() groupId [%s] maxSessions[%d]"), groupId, maxSessions);
   return maxSessions;
}
}} // namespace
