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
// Ident : $Id: PathHelperMgr.cpp $
// Branch: $Name:  $
// Author: huang li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/application/GB_MOD2/MODHelperMgr.cpp $
// 
// 5     1/15/15 5:45p Li.huang
// fix bug 20607
// 
// 4     6/29/11 9:47a Li.huang
// add  GB AAA
// 
// 3     4/14/11 11:41a Fei.huang
// * migrated to linux
// 
// 2     4/12/11 4:04p Li.huang
// implement TV-Now_Advertisement
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 7     10-02-11 14:51 Li.huang
// 
// 6     09-10-30 12:00 Li.huang
// add ActiveConnetCount and purchase timeout reason to 200030 200040
// 
// 5     09-04-28 16:10 Li.huang
// 
// 4     09-03-16 10:56 Li.huang
// add new errorcode
// 
// 3     08-07-22 14:31 Li.huang
// 
// 2     08-03-24 17:04 Li.huang
// 
// 1     08-03-10 16:58 Li.huang
// ===========================================================================

#include "MODHelperMgr.h"
#include "Log.h"

#ifdef ZQ_OS_MSWIN
extern "C"
{
	#include <io.h>
};
#endif
#include "FileSystemOp.h"

namespace ZQTianShan {
namespace Application{
namespace MOD{
#define  MODAPPHelperMgr  "MODAppHelperMgr "
MODAppHelperMgr::MODAppHelperMgr()
{
}

MODAppHelperMgr::~MODAppHelperMgr()
{
	ZQ::common::MutexGuard sync(_lockMHOMap);
	for (MHOMap::iterator it = _MHOMap.begin(); it != _MHOMap.end(); it++)
	{
		if (!it->second)
			continue;

		try {
			MHOPluginFacet	mhoFacet(*(it->second));
			mhoFacet.UnInitMHO();
		}catch(...) {}
		
		try{delete it->second;}catch (...) {}
	}
	_MHOMap.clear();
}


IAuthorization* MODAppHelperMgr::registerAuthorization(const char* type, IAuthorization& helper, void* pCtx)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		MHOAuthorNode node;
		node.pAuthorMHO = &helper; node.fnPlugin = (const char*) (NULL == pCtx ? "<Internal>" : pCtx);
		glog(ZQ::common::Log::L_INFO, 
			CLOGFMT(MODAPPHelperMgr, "associate Authorization type \"%s\" with MHO: \"%s\""), 
			type, node.fnPlugin.c_str());
		ZQ::common::MutexGuard gd(_lockAuthorizationMap);
		_AuthorizationMap.insert(MHOAuthorMap::value_type(type, node));
	}
	
	return findAuthorizationHelper(type);
}

IAuthorization* MODAppHelperMgr::unregisterAuthorization(const char* type)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		ZQ::common::MutexGuard gd(_lockAuthorizationMap);
		MHOAuthorMap::iterator it = _AuthorizationMap.find(type);
		if (_AuthorizationMap.end() == it)
			return NULL;

		IAuthorization* ret = (IAuthorization*) it->second.pAuthorMHO;
		
		_AuthorizationMap.erase(it);
		return ret;
	}
}

IPlayListQuery* MODAppHelperMgr::registerPlayListQuery(const char* type, IPlayListQuery& helper, void* pCtx)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		MHOPLQueryNode node;
		node.pPLQueryMHO = &helper; node.fnPlugin = (const char*) (NULL == pCtx ? "<Internal>" : pCtx);
		glog(ZQ::common::Log::L_INFO, 
			CLOGFMT(MODAPPHelperMgr, "associate PlayListQuery type \"%s\" with MHO: \"%s\""),
			type, node.fnPlugin.c_str());
		ZQ::common::MutexGuard gd(_lockPLQueryMap);
		if (_PLQueryMap.end() != _PLQueryMap.find(type))
			glog(ZQ::common::Log::L_INFO, CLOGFMT(MODAPPHelperMgr, "Pre-associated PlayListQuery type \"%s\" is being overwritted by MHO: \"%s\""), type, node.fnPlugin.c_str());
		_PLQueryMap.insert(MHOPLQueryMap::value_type(type, node));
	}
	
	return findPlayListQueryHelper(type);
}

IPlayListQuery* MODAppHelperMgr::unregistPlayListQuery(const char* type)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		ZQ::common::MutexGuard gd(_lockPLQueryMap);
		MHOPLQueryMap::iterator it = _PLQueryMap.find(type);
		if (_PLQueryMap.end() == it)
			return NULL;

		IPlayListQuery* ret = (IPlayListQuery*) it->second.pPLQueryMHO;
		
		_PLQueryMap.erase(it);
		return ret;
	}
}
IAdsReplacement* MODAppHelperMgr::registerAdsReplacement(const char* type, IAdsReplacement& helper, void* pCtx)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		MHOAdsNode node;
		node.pAdsMHO = &helper; node.fnPlugin = (const char*) (NULL == pCtx ? "<Internal>" : pCtx);
		glog(ZQ::common::Log::L_INFO, 
			CLOGFMT(MODAPPHelperMgr, "associate AdsReplacement type \"%s\" with MHO: \"%s\""),
			type, node.fnPlugin.c_str());
		ZQ::common::MutexGuard gd(_lockAdsMap);
		if (_AdsMap.end() != _AdsMap.find(type))
			glog(ZQ::common::Log::L_INFO, CLOGFMT(MODAPPHelperMgr, "Pre-associated AdsReplacement type \"%s\" is being overwritted by MHO: \"%s\""), type, node.fnPlugin.c_str());
		_AdsMap.insert(MHOAdsMap::value_type(type, node));
	}

	return findAdsReplacement(type);
}
IAdsReplacement* MODAppHelperMgr::unregistAdsReplacement(const char* type)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		ZQ::common::MutexGuard gd(_lockAdsMap);
		MHOAdsMap::iterator it = _AdsMap.find(type);
		if (_AdsMap.end() == it)
			return NULL;

		IAdsReplacement* ret = (IAdsReplacement*) it->second.pAdsMHO;

		_AdsMap.erase(it);
		return ret;
	}
}
::TianShanIce::StrValues MODAppHelperMgr::listSupportedAuthorizationTypes()
{
	::TianShanIce::StrValues types;
	ZQ::common::MutexGuard gd(_lockAuthorizationMap);
	for (MHOAuthorMap::const_iterator it = _AuthorizationMap.begin(); it != _AuthorizationMap.end(); it++)
	{
		if (it->first.empty() || NULL == it->second.pAuthorMHO)
			continue;
		types.push_back(it->first);
	}

	return types;
}

::TianShanIce::StrValues MODAppHelperMgr::listSupportedPlayListQueryTypes()
{
	::TianShanIce::StrValues types;
	ZQ::common::MutexGuard gd(_lockPLQueryMap);
	for (MHOPLQueryMap::const_iterator it = _PLQueryMap.begin(); it != _PLQueryMap.end(); it++)
	{
		if (it->first.empty() || NULL == it->second.pPLQueryMHO)
			continue;
		types.push_back(it->first);
	}

	return types;
}
::TianShanIce::StrValues MODAppHelperMgr::listSupportedAdsReplacementTypes()
{
	::TianShanIce::StrValues types;
	ZQ::common::MutexGuard gd(_lockAdsMap);
	for (MHOAdsMap::const_iterator it = _AdsMap.begin(); it != _AdsMap.end(); it++)
	{
		if (it->first.empty() || NULL == it->second.pAdsMHO)
			continue;
		types.push_back(it->first);
	}

	return types;
}
IAuthorization* MODAppHelperMgr::findAuthorizationHelper(const char* type)
{
	ZQ::common::MutexGuard gd(_lockAuthorizationMap);
	MHOAuthorMap::iterator it = _AuthorizationMap.find(type);
	if (_AuthorizationMap.end() == it)
		return NULL;
	
	return (IAuthorization*) it->second.pAuthorMHO;
}

IPlayListQuery* MODAppHelperMgr::findPlayListQueryHelper(const char* type)
{
	ZQ::common::MutexGuard gd(_lockPLQueryMap);
	MHOPLQueryMap::iterator it = _PLQueryMap.find(type);
	if(_PLQueryMap.end() == it)
		return NULL;
	
	return (IPlayListQuery*) it->second.pPLQueryMHO;
}
IAdsReplacement* MODAppHelperMgr::findAdsReplacement(const char* type)
{
	ZQ::common::MutexGuard gd(_lockAdsMap);
	MHOAdsMap::iterator it = _AdsMap.find(type);
	if(_AdsMap.end() == it)
		return NULL;
	
	return (IAdsReplacement*) it->second.pAdsMHO;
}
bool MODAppHelperMgr::getAuthorizationInfo(const ::std::string& type, ::ZQ::common::DynSharedObj::ImageInfo& info)
{
	std::string filename;
	{
		ZQ::common::MutexGuard gd(_lockAuthorizationMap);
		MHOAuthorMap::iterator it = _AuthorizationMap.find(type);
		if (_AuthorizationMap.end() == it)
			return false;
		filename = it->second.fnPlugin;
	}
	{
		ZQ::common::MutexGuard gd(_lockMHOMap);
		MHOMap::iterator it = _MHOMap.find(filename);
		memcpy(&info, it->second->getImageInfo(), sizeof(info));
	}
	return true;
}
bool MODAppHelperMgr::getPlayListQueryInfo(const ::std::string& type, ::ZQ::common::DynSharedObj::ImageInfo& info)
{
	std::string filename;
	{
		ZQ::common::MutexGuard gd(_lockPLQueryMap);
		MHOPLQueryMap::iterator it = _PLQueryMap.find(type);
		if (_PLQueryMap.end() == it)
			return false;
		filename = it->second.fnPlugin;
	}
	{
		ZQ::common::MutexGuard gd(_lockMHOMap);
		MHOMap::iterator it = _MHOMap.find(filename);
		memcpy(&info, it->second->getImageInfo(), sizeof(info));
	}
	return true;
}

int MODAppHelperMgr::OnAuthPurchase(const char* type, IAuthorization::AuthorInfo& authorInfo, ::TianShanIce::ValueMap& privData)	
{
	int retCode;
	IAuthorization *pAuthor = NULL;
	try
	{
		pAuthor = findAuthorizationHelper(type);

		if(NULL == pAuthor)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, 
				"can't find the Authorization Object with type = %s"), type);
			retCode = IAuthorization::UNKNOWNTYPE;

			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,
				MODAPPHelperMgr, retCode, "can't find the Authorization Object with type=%s", 
				type);
			return retCode;
		}		   
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, 
			"can't find the Authorization Object with type = %s"), type);
		retCode = IAuthorization::UNKNOWN;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,
			MODAPPHelperMgr, retCode, "can't find the Authorization Object with type=%s", 
			type);
		return retCode;
	}

	return pAuthor->OnAuthPurchase(authorInfo, privData);
}

int MODAppHelperMgr::OnDestroyPurchase(const char* type, IAuthorization::AuthorInfo& authorInfo, const ::TianShanIce::Properties& prop)
{
	int retCode;
	IAuthorization *pAuthor = NULL;	
	try
	{		
		pAuthor = findAuthorizationHelper(type);	   
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, 
			"Unknown error OnDestroyPurchase with type = %s"), type);

		retCode = IAuthorization::UNKNOWN;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,
			MODAPPHelperMgr, retCode, " can't find the Authorization Object with type=%s", 
			type);

		return retCode;
	}

	if(NULL == pAuthor)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, 
			"can't find the Authorization Object with type=%s"), type);
		retCode = IAuthorization::UNKNOWNTYPE;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,
			MODAPPHelperMgr, retCode, " can't find the Authorization Object with type=%s", 
			type);
		return retCode;
	}	
	return pAuthor->OnDestroyPurchase(authorInfo, prop);
}

int  MODAppHelperMgr::getPlayList(const char* type, IPlayListQuery::PlayListInfo& plinfo, AEReturnData& aedata)
{
	int retCode;
	IPlayListQuery *pPLQuery = NULL;  

	try
	{
		pPLQuery = findPlayListQueryHelper(type);		   
	}
	catch (...)
	{		
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, 
			"Unknown error getPlayList with type = %s"), type);

		retCode = IPlayListQuery::UNKNOWN;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,
			MODAPPHelperMgr, retCode, " can't find the PlayListQuery Object with type=%s", 
			type);

		return retCode;
	}

	if(NULL == pPLQuery)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, 
			"can't find the PlayListQuery Object with type = %s"), type);
		retCode = IAuthorization::UNKNOWNTYPE;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,
			MODAPPHelperMgr, retCode, " can't find the PlayListQuery Object with type=%s", 
			type);
		return retCode;
	}
	return pPLQuery->getPlayList(plinfo, aedata);
}
int MODAppHelperMgr::getAdsRepalcement(const char* type, IAdsReplacement::AdsInfo& adsInfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData)
{
	int retCode;
	IAdsReplacement *pAdsReplacement = NULL;  

	try
	{
		pAdsReplacement = findAdsReplacement(type);		   
	}
	catch (...)
	{		
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, 
			"Unknown error AdsReplacement with type = %s"), type);

		retCode = IAdsReplacement::UNKNOWN;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,
			MODAPPHelperMgr, retCode, " can't find the AdsReplacement Object with type=%s", 
			type);

		return retCode;
	}

	if(NULL == pAdsReplacement)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, 
			"can't find the AdsReplacement Object with type = %s"), type);
		retCode = IAuthorization::UNKNOWNTYPE;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,
			MODAPPHelperMgr, retCode, " can't find the AdsReplacement Object with type=%s", type);

		return retCode;
	}
	return pAdsReplacement->getAdsReplacement(adsInfo, aedata, privData);
}
void MODAppHelperMgr::setExtraData(const char* configFile,const char* logFolder, std::string MODinstanceID, Ice::CommunicatorPtr& ic)
{
	m_InstanceID = MODinstanceID;
	m_ic = ic;
	if(configFile&&strlen(configFile)>0)
	{
		_strConfigFile=configFile;
	}
	else
	{
		_strConfigFile="";
	}
	if(logFolder&&strlen(logFolder)>0)
	{
		_strLogFolder=logFolder;
	}
	else
	{
		_strLogFolder="";
	}
}
int MODAppHelperMgr::populate(const std::vector<std::string>& plugins)
{
	std::vector<std::string>::const_iterator file = plugins.begin();
	for(; file != plugins.end(); ++file) {
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(MODAPPHelperMgr,"load file[%s]"), file->c_str());
		loadMHO(file->c_str());
	}
	return _MHOMap.size();
}
int MODAppHelperMgr::populate(const char* pathToMHO)
{
	if (NULL == pathToMHO)
		return _MHOMap.size();
	
	std::string wkpath = pathToMHO;
	std::string testwkpath =wkpath +PHSEPS;
	std::string searchpath = ::getenv("PATH");
	searchpath +=PHSEPS;
	
#ifdef WIN32
	// in windows, the pathnames are case-insensitive, make all lower case before comparing
	std::transform(testwkpath.begin(), testwkpath.end(), testwkpath.begin(), (int(*)(int)) tolower);
	std::transform(searchpath.begin(), searchpath.end(), searchpath.begin(), (int(*)(int)) tolower);
#endif
	
	int pos =searchpath.find(testwkpath);
	if (pos<0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(MODAPPHelperMgr,
			"populate() append %s into evironment variable PATH"), wkpath.c_str());
		searchpath = "PATH=";
		searchpath += getenv("PATH");
		searchpath += PHSEPS;
		searchpath += wkpath;
		putenv(const_cast<char*>(searchpath.c_str()));
	}
	
	if (wkpath[wkpath.length()-1] != FNSEPC)
		wkpath +=FNSEPS;

	std::vector<std::string> result = FS::searchFiles(wkpath, MHO_FILENAME_PREFIX "*" MHO_EXT);	
	std::vector<std::string>::iterator file = result.begin();
	for(; file != result.end(); ++file) {
		loadMHO(file->c_str());
	}

	return _MHOMap.size();
}

ZQ::common::DynSharedObj* MODAppHelperMgr::loadMHO(const char* filename)
{
	if (NULL == filename)
		return NULL;

	ZQ::common::DynSharedObj* pDHO = new ZQ::common::DynSharedObj(filename);

	if (NULL == pDHO)
		return NULL;

	ZQ::common::MutexGuard sync(_lockMHOMap);
	MHOMap::iterator it = _MHOMap.find(pDHO->getImageInfo()->filename);
	if (_MHOMap.end() != it)
	{
		delete pDHO;
		return it->second;
	}
	
	bool bValidPHO = false;
	try
	{
		MHOPluginFacet	mhoFacet(*pDHO);
		bValidPHO = mhoFacet.isValid();
		if (bValidPHO)
		{
			mhoFacet.InitMHO(*this, NULL, _strConfigFile.c_str(),_strLogFolder.c_str(),m_InstanceID.c_str(), m_ic);
		}
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, "loadMHO() failed to load %s"), pDHO->getImageInfo()->filename);
		delete pDHO;
		return NULL;
	}

	if (!bValidPHO)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, "loadMHO() invalid MHO: %s"), pDHO->getImageInfo()->filename);
		delete pDHO;
		return NULL;
	}

	_MHOMap[pDHO->getImageInfo()->filename] = pDHO;
	glog(ZQ::common::Log::L_INFO, CLOGFMT(MODAPPHelperMgr, "loadMHO() MHO: %s"), filename);
	return pDHO;
}
const char * MODAppHelperMgr::getErrorMsg(int errCode)
{
	static const char* szErrors[] = {
		"successful",
			"failed",
			"ice exception",
			"unknown  type",
			"internal error",
			"not support this interface"
	};
	if(errCode < 0 || errCode > 5)
		return "unknown error";
	else
		return szErrors[errCode];
}

IAAA* MODAppHelperMgr::registerAAA(const char* type, IAAA& helper, void* pCtx)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		MHOAAANode node;
		node.pAAAMHO = &helper; node.fnPlugin = (const char*) (NULL == pCtx ? "<Internal>" : pCtx);
		glog(ZQ::common::Log::L_INFO, 
			CLOGFMT(MODAPPHelperMgr, "associate AAA type \"%s\" with MHO: \"%s\""),
			type, node.fnPlugin.c_str());
		ZQ::common::MutexGuard gd(_lockAAAMap);
		if (_AAAMap.end() != _AAAMap.find(type))
			glog(ZQ::common::Log::L_INFO, CLOGFMT(MODAPPHelperMgr, "Pre-associated AAA type \"%s\" is being overwritted by MHO: \"%s\""), type, node.fnPlugin.c_str());
		_AAAMap.insert(MHOAAAMap::value_type(type, node));
	}

	return findAAA(type);
}
IAAA* MODAppHelperMgr::unregistAAA(const char* type)
{
	if (NULL == type || strlen(type) <=0)
		return NULL;
	else
	{
		ZQ::common::MutexGuard gd(_lockAAAMap);
		MHOAAAMap::iterator it = _AAAMap.find(type);
		if (_AAAMap.end() == it)
			return NULL;

		IAAA* ret = (IAAA*) it->second.pAAAMHO;

		_AAAMap.erase(it);
		return ret;
	}
}
IAAA* MODAppHelperMgr::findAAA(const char* type)
{
	ZQ::common::MutexGuard gd(_lockAAAMap);
	MHOAAAMap::iterator it = _AAAMap.find(type);
	if(_AAAMap.end() == it)
		return NULL;

	return (IAAA*) it->second.pAAAMHO;
}

::TianShanIce::StrValues MODAppHelperMgr::listSupportedAAATypes()
{
	::TianShanIce::StrValues types;
	ZQ::common::MutexGuard gd(_lockAAAMap);
	for (MHOAAAMap::const_iterator it = _AAAMap.begin(); it != _AAAMap.end(); it++)
	{
		if (it->first.empty() || NULL == it->second.pAAAMHO)
			continue;
		types.push_back(it->first);
	}

	return types;
}
int MODAppHelperMgr::OnAuthorize(const char* type, IAAA::AAAInfo& aaaInfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData)
{
	int retCode;
	IAAA *pAAA = NULL;  

	try
	{
		pAAA = findAAA(type);		   
	}
	catch (...)
	{		
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, "Unknown error AAA with type = %s"), type);

		retCode = IAAA::UNKNOWN;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,MODAPPHelperMgr, retCode, " can't find the AAA Object with type=%s", type);

		return retCode;
	}

	if(NULL == pAAA)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, "can't find the AAA Object with type = %s"), type);
		retCode = IAAA::UNKNOWNTYPE;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,MODAPPHelperMgr, retCode, " can't find the AAA Object with type=%s", type);

		return retCode;
	}
	return pAAA->OnAuthorize(aaaInfo, aedata, privData);
}

int MODAppHelperMgr::OnStatusNotice(const char* type, IAAA::AAAInfo& aaaInfo, const ::TianShanIce::Properties& prop)
{
	int retCode;
	IAAA *pAAA = NULL;  

	try
	{
		pAAA = findAAA(type);		   
	}
	catch (...)
	{		
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, "Unknown error AAA with type = %s"), type);

		retCode = IAAA::UNKNOWN;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,MODAPPHelperMgr, retCode, " can't find the AAA Object with type=%s", type);

		return retCode;
	}

	if(NULL == pAAA)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MODAPPHelperMgr, "can't find the AAA Object with type = %s"), type);
		retCode = IAAA::UNKNOWNTYPE;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,MODAPPHelperMgr, retCode, " can't find the AAA Object with type=%s", type);

		return retCode;
	}
	return pAAA->OnStatusNotice(aaaInfo,prop);
}

}}}// namespace
