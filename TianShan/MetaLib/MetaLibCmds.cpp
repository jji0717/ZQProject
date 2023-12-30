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
// Ident : $Id: MetaLibCmds.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/MetaLib/MetaLibCmds.cpp $
// 
// 3     1/11/16 6:04p Dejian.fei
// 
// 2     12/12/13 1:57p Hui.shao
// %lld for int64
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 16    10-07-09 14:29 Yixin.tian
// modify %lld to %lld
// 
// 15    10-07-06 17:41 Yixin.tian
// merge for Linux OS
// 
// 14    10-06-22 15:09 Haoyuan.lu
// 
// 13    10-05-10 14:14 Haoyuan.lu
// 
// 12    10-04-28 10:56 Haoyuan.lu
// 
// 11    10-04-21 13:50 Li.huang
// 
// 9     10-04-16 15:25 Li.huang
// 
// 8     10-04-15 17:19 Li.huang
// optimize lookup code
// 
// 7     10-04-12 15:21 Haoyuan.lu
// 
// 6     10-02-24 16:56 Haoyuan.lu
// 
// 5     09-09-10 17:11 Haoyuan.lu
// 
// 4     09-06-29 16:45 Li.huang
// 
// 3     09-05-13 13:59 Haoyuan.lu
// 
// 2     08-03-18 15:50 Hui.shao
// 
// 1     08-03-17 21:08 Hui.shao
// ===========================================================================

#include "MetaLibCmds.h"
#include "MetaLibImpl.h"
#include <algorithm>

namespace ZQTianShan {
namespace MetaLib {

#define IdentityToObj(_CLASS, _ID) ::TianShanIce::Repository::_CLASS##Prx::uncheckedCast(_lib._adapter->createProxy(_ID))
#define IdentityToObj2(_CLASS, _ID) ::TianShanIce::Repository::_CLASS##Prx::checkedCast(_lib._adapter->createProxy(_ID))

// -----------------------------
// class BaseCmd
// -----------------------------
BaseCmd::BaseCmd(MetaLibImpl& lib)
	: ThreadRequest(lib._thpool), _lib(lib)
{
}

BaseCmd::~BaseCmd()
{
}
typedef struct 
{
	std::string key;
	std::string value;
	IdentCollection objids;
}SearchMetaData;
typedef std::vector<SearchMetaData>SearchMetaDatas;
bool mod_less(SearchMetaData sermetadata1, SearchMetaData sermetadata2)
{
	if(sermetadata1.objids.size() >= sermetadata2.objids.size())
		return false;
	return true;
}
// -----------------------------
// class LookupCmd
// -----------------------------
LookupCmd::LookupCmd(MetaLibImpl& lib, const ::TianShanIce::Repository::AMD_MetaDataLibrary_lookupPtr& amdCB, const ::std::string& type, const ::TianShanIce::Properties& searchForMetaData, const ::TianShanIce::StrValues& expectedMetaDataNames)
: BaseCmd(lib), _amdCB(amdCB), _searchForMetaData(searchForMetaData), _expectedMetaDataNames(expectedMetaDataNames)
{
	_dummyIdentObj.category = !type.empty() ? type : _lib._defaultType;
}

int LookupCmd::run(void)
{
	std::string lastError;
	bool bIsType = false;
	_lib._log(ZQ::common::Log::L_DEBUG, CLOGFMT(LookupCmd, "looking up the matched metadata"));
	Ice::Long inTime = ZQTianShan::now();
 
	::TianShanIce::Properties::iterator itorType = _searchForMetaData.find(OBJECTTYPE);
	if(itorType != _searchForMetaData.end())
	{
      bIsType = true;
	}
	::TianShanIce::Properties nonIndexMetaData;
	nonIndexMetaData.clear();

	try {
		SearchMetaDatas searchmetadatas;
		for (::TianShanIce::Properties::iterator it = _searchForMetaData.begin(); it != _searchForMetaData.end(); it++)
		{
			Ice::Long lStart = ZQTianShan::now();
			IdentCollection objids;
#if ICE_INT_VERSION / 100 >= 306
			RLockT <IceUtil::RWRecMutex> lk(_lib._metaDataContainerLocker);
#else
			IceUtil::RLockT <IceUtil::RWRecMutex> lk(_lib._metaDataContainerLocker);
#endif
			std::string ctnkeyname;
			if(_lib._bIsCombineDB)
			{
				ctnkeyname = METADATA_COMBINE_KEYNAME;
			}
			else
			{
				ctnkeyname = it->first;
			}

			MetaLibImpl::MetaDataContainerMap::iterator itMetaData = _lib._metaDataContainerMap.find(ctnkeyname);
			if (_lib._metaDataContainerMap.end() == itMetaData)
			{
				//	ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (_lib._log, EXPFMT(MetaLib, 3001, "LookupCmd() look up metaData[%s] not found"), ctnkeyname.c_str());
				nonIndexMetaData.insert(std::make_pair(it->first, it->second));
				continue;
			}

			TianShanIce::Repository::ValueIdxPtr idxValue = itMetaData->second.idxValue;
			if(idxValue)
				objids = idxValue->find(it->second);
			else
			{
				nonIndexMetaData.insert(std::make_pair(it->first, it->second));
				continue;
				//	ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (_lib._log, EXPFMT(MetaLib, 3001, "LookupCmd() look up metaData[%s] is non index"), ctnkeyname.c_str());
			}
			if(objids.size() <= 0)
			{
				TianShanIce::Repository::MetaObjectInfos result;
				_amdCB->ice_response(result);
				return 0;
			}
			SearchMetaData searchmetadata;
			searchmetadata.key = it->first;
			searchmetadata.value = it->second;
			searchmetadata.objids = objids;
			searchmetadatas.push_back(searchmetadata);
			_lib._log(ZQ::common::Log::L_DEBUG, CLOGFMT(LookupCmd, "[%s=%s] %d, took total %lldms"),
				it->first.c_str(), it->second.c_str(), objids.size(),  ZQTianShan::now() - lStart);

		}

		bool bFirstFilled = true;
		_objsFound.clear();
		::Ice::Long stampBegin = ZQTianShan::now(), stampFinish= stampBegin;

		sort(searchmetadatas.begin(), searchmetadatas.end(), mod_less);

		for (SearchMetaDatas::iterator it = searchmetadatas.begin(); it != searchmetadatas.end(); it++)
		{
            IdentCollection &objids = it->objids;
			::Ice::Long stampStart = ZQTianShan::now();
			{
#if ICE_INT_VERSION / 100 >= 306
				RLockT <IceUtil::RWRecMutex> lk(_lib._metaDataContainerLocker);
#else
				IceUtil::RLockT <IceUtil::RWRecMutex> lk(_lib._metaDataContainerLocker);
#endif
				if(_lib._bIsCombineDB)
				{
					IdentCollection obj4category;
					for (IdentCollection::iterator itObj = objids.begin(); itObj != objids.end(); itObj++)
					{
						if(itObj->category.compare((*it).key))
							obj4category.push_back(*itObj);
					}
					objids.clear();
					objids.assign(obj4category.begin(), obj4category.end());
				}
			}

			::Ice::Long stampSearch = ZQTianShan::now();
			
			::std::sort(objids.begin(), objids.end());
			::Ice::Long stampSort = ZQTianShan::now();
			
			IdentCollection matchedIds;
			IdentCollection::iterator itResult = _objsFound.begin();
			
			for (IdentCollection::iterator itObj = objids.begin(); (bFirstFilled || itResult < _objsFound.end()) && itObj < objids.end(); itObj++)
			{
				::Ice::Identity identObj = _lib.covertToObjectIdent(*itObj);
				if (bFirstFilled)
				{
					matchedIds.push_back(identObj);
					continue;
				}
 
				while (itResult < _objsFound.end() && itResult->name.compare(identObj.name) <0)
					itResult ++;
				
				if(itResult == _objsFound.end())
					break;

				if (itResult->name.compare(identObj.name) ==0)
					matchedIds.push_back(identObj);
			}
		
			stampFinish = ZQTianShan::now();

			bFirstFilled =false;
			_objsFound = matchedIds;
			
			_lib._log(ZQ::common::Log::L_INFO, CLOGFMT(LookupCmd, "found %d records matched [%s=%s] took total %lldms: find=%lldms, sort=%lldms, intersect=%lldms"), objids.size(), it->key.c_str(), it->value.c_str(),
				stampFinish - stampStart, stampSearch - stampStart, stampSort - stampSearch, stampFinish - stampSort);

			if(objids.size() == 0 || _objsFound.size() == 0)
			{
				TianShanIce::Repository::MetaObjectInfos result;
				_amdCB->ice_response(result);
				return 0;
			}
		}
		searchmetadatas.clear();
		// search metadata is all non index metadata, throw exception (low search efficiency cause timeout)
		if(_searchForMetaData.size() == nonIndexMetaData.size())
		{
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (_lib._log, EXPFMT(MetaLib, 3001, "LookupCmd() search condition is all non index metadata"));
		}
		
		_lib._log(ZQ::common::Log::L_INFO, CLOGFMT(LookupCmd, "found %d records matched all expressions, took %lldms"), _objsFound.size(), stampFinish - stampBegin);
		
		TianShanIce::Repository::MetaObjectInfos result;
	
		for (IdentCollection::iterator itObj = _objsFound.begin(); itObj < _objsFound.end(); itObj++)
		{
			::Ice::Identity identObj = *itObj;

			::TianShanIce::Repository::MetaObjectInfo objInfo;
			objInfo.id = identObj.name;
			objInfo.type = identObj.category;
			
			try {
				if(!bIsType)
				{
					::TianShanIce::Repository::LibMetaObjectPrx libObjectPrx = IdentityToObj2(LibMetaObject, identObj);
					objInfo.type = libObjectPrx->getType();
					if (0 != _dummyIdentObj.category.compare("*") && 0 != _dummyIdentObj.category.compare(objInfo.type))
						continue;
				}
			}
			catch(const ::Ice::ObjectNotExistException& )
			{
				continue; //if not continue, the result will be confused 
			}
			catch(const ::Ice::Exception& ex)
			{
				_lib._log(ZQ::common::Log::L_WARNING, CLOGFMT(LookupCmd, "read metaObject[%s] exception[%s], ingore the object"), objInfo.id.c_str(), ex.ice_name().c_str());
#ifndef NDEBUG
				continue;
#endif // NDEBUG
			}
			catch(...)
			{
				_lib._log(ZQ::common::Log::L_WARNING, CLOGFMT(LookupCmd, "read metaObject[%s] unknown exception ingore the object"), objInfo.id.c_str());
#ifndef NDEBUG
				continue;
#endif // NDEBUG
			}

			bool bMatched = true;
			::TianShanIce::Repository::MetaDataMap metaDataMap;

			if(!nonIndexMetaData.empty() || !_expectedMetaDataNames.empty())
			{
				try {
					::TianShanIce::Repository::LibMetaObjectPrx libObjectPrx = IdentityToObj2(LibMetaObject, identObj);
					metaDataMap = libObjectPrx->getMetaDataMap();
				}
				catch(const ::Ice::Exception& ex)
				{
					_lib._log(ZQ::common::Log::L_ERROR, CLOGFMT(LookupCmd, "getMetaDataMap of [%s] caught exception[%s]"), identObj.name.c_str(), ex.ice_name().c_str());
					continue;
				}
				catch(...)
				{
					_lib._log(ZQ::common::Log::L_ERROR, CLOGFMT(LookupCmd, "getMetaDataMap of [%s] caught unknown exception"), identObj.name.c_str());
					continue;
				}
			}

			if (!nonIndexMetaData.empty())
			{
				for (::TianShanIce::Properties::iterator it=nonIndexMetaData.begin(); it != nonIndexMetaData.end(); it++)
				{
					::TianShanIce::Repository::MetaDataMap::iterator map_iter = metaDataMap.find(it->first);
					if( map_iter == metaDataMap.end() || map_iter->second.value != it->second)
					{
						// not matched
						bMatched = false;
						break;
					}
				}
			}

			if(!bMatched)
				continue;

			if(!_expectedMetaDataNames.empty())
			{
				for (size_t i=0; i < _expectedMetaDataNames.size(); i++)
				{
					::TianShanIce::Repository::MetaDataMap::iterator map_iter = metaDataMap.find(_expectedMetaDataNames[i]);
					if(map_iter != metaDataMap.end())
						objInfo.metaDatas.insert(::TianShanIce::Repository::MetaDataMap::value_type(_expectedMetaDataNames[i], map_iter->second));
				}
			}

			result.push_back(objInfo);
		}
		
		_amdCB->ice_response(result);
		Ice::Long outTime = ZQTianShan::now();
		_lib._log(ZQ::common::Log::L_INFO, CLOGFMT(LookupCmd, "finish the lookup, took %lldms"), outTime - inTime);
		_lib._log(ZQ::common::Log::L_DEBUG, CLOGFMT(LookupCmd, "looking up return"));
		return 0;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_amdCB->ice_exception(ex);
		return 1;
	}
	catch (const Freeze::DatabaseException& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "LookupCmd caught exception[%s: %s]", ex.ice_name().c_str(), ex.message.c_str());
		lastError = buf;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "LookupCmd caught exception[%s]", ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "LookupCmd caught unknown exception");
		lastError = buf;
	}
	
	TianShanIce::ServerError ex("LookupCmd", 501, lastError);
	_amdCB->ice_exception(ex);
	
	return 1;
}
	
}} // namespace
