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
// Ident : $Id: MetaLibImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/MetaLib/MetaLibImpl.cpp $
// 
// 4     1/11/16 6:04p Dejian.fei
// 
// 3     12/12/13 1:57p Hui.shao
// %lld for int64
// 
// 2     2/06/13 4:22p Hui.shao
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 14    10-07-09 14:29 Yixin.tian
// modify %lld to %lld
// 
// 13    10-07-06 17:41 Yixin.tian
// merge for Linux OS
// 
// 12    10-06-22 15:10 Haoyuan.lu
// 
// 11    10-05-10 14:14 Haoyuan.lu
// 
// 10    10-04-16 15:27 Haoyuan.lu
// 
// 9     10-04-15 17:58 Haoyuan.lu
// 
// 8     10-02-24 16:56 Haoyuan.lu
// 
// 7     09-07-03 15:19 Haoyuan.lu
// 
// 6     09-06-29 16:45 Li.huang
// 
// 5     09-05-13 13:59 Haoyuan.lu
// 
// 4     09-04-02 11:15 Hui.shao
// 
// 3     09-04-01 16:57 Hui.shao
// check-in works as of 4/1/2009
// 
// 2     08-03-18 15:47 Hui.shao
// 
// 1     08-03-17 21:08 Hui.shao
// ===========================================================================
#include "MetaLibImpl.h"
#include "MetaLibCmds.h"
#include "Guid.h"
#include "Log.h"
#include "FileSystemOp.h"

extern "C"
{
#ifdef ZQ_OS_MSWIN
	#include <io.h>
#endif
};

namespace ZQTianShan {
namespace MetaLib {
	
typedef ::std::vector< Ice::Identity > IdentCollection;
#define IdentityToObj(_CLASS, _ID) ::TianShanIce::Repository::_CLASS##Prx::uncheckedCast(_adapter->createProxy(_ID))
#define IdentityToObj2(_CLASS, _ID) ::TianShanIce::Repository::_CLASS##Prx::checkedCast(_adapter->createProxy(_ID))
#define CATEG_Txn			"Txn"

#define INSTANCE_AMICB(_API)	if (!_env._amiCB##_API)	_env._amiCB##_API = new _API##AmiCBImpl(_env)
#define INDEXFILENAME(_IDX)			#_IDX "Idx"
#define METADATA_FILENAME_PREFFIX		"md_"
#define METADATAVALINDEX_FN_PREFFIX		"mv"

#define		MAX_BATCH_ITERATOR_SIZE		1024

// -----------------------------
// class LibMetaValueImpl
// -----------------------------
LibMetaValueImpl::LibMetaValueImpl(MetaLibImpl& lib)
:_lib(lib)
{
}
	
::Ice::Identity LibMetaValueImpl::getObjId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return identObj;
}

::TianShanIce::Repository::MetaDataValue LibMetaValueImpl::get(const ::Ice::Current& c) const
{
	::TianShanIce::Repository::MetaDataValue ret;
	RLock sync(*this);
	ret.value = value;
	ret.hintedType = hintedType;

	return ret;
}

void LibMetaValueImpl::set(const ::TianShanIce::Repository::MetaDataValue& value, const ::Ice::Current& c)
{
	WLock sync(*this);
	this->identObj = identObj;
	this->value = value.value;
	this->hintedType = value.hintedType;
}

// -----------------------------
// class LibMetaObjectImpl
// -----------------------------
LibMetaObjectImpl::LibMetaObjectImpl(MetaLibImpl& lib)
: _lib(lib)
{
}

::Ice::Identity LibMetaObjectImpl::getObjId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return ident;
}

::std::string LibMetaObjectImpl::getType(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return type;
}

::std::string LibMetaObjectImpl::getCreatedTime(const ::Ice::Current& c) const
{
	char buf[64];
	RLock sync(*this);
	return ::ZQTianShan::TimeToUTC(stampCreated, buf, sizeof(buf)-2);
}
    
::TianShanIce::Repository::MetaDataValue LibMetaObjectImpl::get(const ::std::string& key, const ::Ice::Current& c) const
{
	::TianShanIce::Repository::MetaDataValue ret;
	ret.hintedType = ::TianShanIce::vtUnknown;
	RLock sync(*this);
   #pragma message ( __MSGLOC__ "TODO: query the metadata DB file for the result")
    return _lib.getMetaData(ident, key);
//	return ret;
}

void LibMetaObjectImpl::set(const ::std::string& key, const ::TianShanIce::Repository::MetaDataValue& value, const ::Ice::Current& c)
{
	RLock sync(*this);
#pragma message ( __MSGLOC__ "TODO: update this value to the DB")
    _lib.setMetaData(ident.name, key, value, false, c);
}

::TianShanIce::Repository::MetaDataMap LibMetaObjectImpl::getMetaDataMap(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return metaDataMap;
}

// -----------------------------
// module MetaLibImpl
// -----------------------------
::Ice::Identity MetaLibImpl::covertToMetaDataIdent(const ::Ice::Identity& identIn, const std::string& metadataName)
{
	::Ice::Identity ident;
	ident.category = metaDataCategory(metadataName);

//	std::string::size_type pos = identIn.name.find(OBJMD_SEP);
//	ident.name = metadataName + OBJMD_SEP + (0 == ident.category.compare(identIn.category) && pos != std::string::npos ? identIn.name.substr(pos+1) : identIn.name);
	ident.name = identIn.name;
	return ident;
}

::Ice::Identity MetaLibImpl::covertToObjectIdent(const ::Ice::Identity& identIn)
{
	if (0 == identIn.category.compare(META_OBJECT))
		return identIn;

	::Ice::Identity ident;
	ident.category = META_OBJECT;
//	std::string::size_type pos = identIn.name.find(OBJMD_SEP);
//	ident.name = (pos != std::string::npos ? identIn.name.substr(pos+1) : identIn.name);
	ident.name = identIn.name;
	return ident;
}

std::string MetaLibImpl::metaDataCategory(const std::string& metadataName)
{
	return std::string(METADATA_FILENAME_PREFFIX) + metadataName;
}

MetaLibImpl::MetaLibImpl(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool, Ice::CommunicatorPtr& communicator, const ::TianShanIce::StrValues& nonIndices, const char* databasePath, bool bIsCombineDB)
	: _log(log), _thpool(threadPool), _communicator(communicator), _bIsCombineDB(bIsCombineDB), _nonIndices(nonIndices)
{
	try
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "open NULL adapter"));
		char endpoint[256] = { 0 };
		snprintf(endpoint, sizeof(endpoint), "%s %d", "default -p", DEFAULT_BINDPORT_MetaLib);
		_adapter = _communicator->createObjectAdapterWithEndpoints("MetaLib", endpoint); // no ZQ adapter here
	}
	catch(const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib, "Create adapter failed, caught exception[%s]"), ex.ice_name().c_str());
		throw ex;
	}

	std::string strName = FS::getImagePath();
	if ( strName.size() > 0)
	{
		char path[MAX_PATH] = {0};
		strcpy(path, strName.c_str());
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
				*p='\0';
		}
		_programRootPath = path;
	}
	else 
		_programRootPath = ".";
	
	_programRootPath += FNSEPS;

	openDB(databasePath);
	
	// init the object factory
// 	_factory = new MetaLibFactory(*this);

	char buf[20];
	sprintf(buf, "%p", this);
	_identBind.name = buf;
	_identBind.category = "MetaLib";
	_adapter->add(this, _identBind);
	_adapter->activate();
}

MetaLibImpl::~MetaLibImpl()
{
	closeDB();
	_factory = NULL;
	_adapter=NULL;
}

::TianShanIce::Repository::MetaLibPrx& MetaLibImpl::proxy()
{
	try {
		if (!_thisPrx)
			_thisPrx = ::TianShanIce::Repository::MetaLibPrx::checkedCast(_adapter->createProxy(_identBind)->ice_collocationOptimized(false));
//			_thisPrx = ::TianShanIce::Repository::MetaLibPrx::checkedCast(_adapter->createProxy(_identBind));
	}
	catch(const Ice::Exception&)
	{
	}

	return _thisPrx;
}

void MetaLibImpl::openDB(const char* databasePath)
{
	closeDB();

	if (NULL == databasePath || strlen(databasePath) <1)
		_dbPath = _programRootPath + "data" FNSEPS "MetaLib" FNSEPS;
	else 
		_dbPath = databasePath;
	
	if (FNSEPC != _dbPath[_dbPath.length()-1])
		_dbPath += FNSEPS;
	
	try 
	{	
		// open the Indexes
		_log(ZQ::common::Log::L_INFO, CLOGFMT(MetaLib, "opening database at path: %s"), _dbPath.c_str());
		ZQTianShan::createDBFolderEx(_log, _dbPath); 

		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "create Type Index"));
		_idxType = new TianShanIce::Repository::TypeIdx("idxType");
		
		Ice::PropertiesPtr proper = _adapter->getCommunicator()->getProperties();
		proper->setProperty("Freeze.Evictor.UseNonmutating",      "1");

		std::string dbAttrPrefix = std::string("Freeze.DbEnv.") + _dbPath;
		proper->setProperty(dbAttrPrefix + ".DbRecoverFatal",      "1");
		proper->setProperty(dbAttrPrefix + ".DbPrivate",           "0");
		proper->setProperty(dbAttrPrefix + ".OldLogsAutoDelete",   "1");

		std::string evictorAttrPrefix = std::string("Freeze.Evictor.");
		proper->setProperty(evictorAttrPrefix + META_OBJECT ".dat.$default.BtreeMinKey",      "16");
//		proper->setProperty(evictorAttrPrefix + META_OBJECT ".dat.$index:$default.idxType.BtreeMinKey",      "8");
		proper->setProperty(evictorAttrPrefix + META_OBJECT ".dat.PageSize",      "8192");


		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxType);

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "create evictor %s with index %s"), "metaObj", "TypeIdx");
#if ICE_INT_VERSION / 100 >= 303
			_eMetaObj = ::Freeze::createBackgroundSaveEvictor(_adapter, _dbPath, META_OBJECT ".dat", 0, indices);
#else
			_eMetaObj = Freeze::createEvictor(_adapter, _dbPath, META_OBJECT ".dat", 0, indices);
#endif
			_adapter->addServantLocator(_eMetaObj, "");
			_eMetaObj->setSize(100);
		}

		///if the metaData evictor needs combine
		///create combine evictor for store metedata entity
		if(_bIsCombineDB)
		{
			MetaDataContainer ctn;
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "create Value Index"));

			ctn.idxValue = new TianShanIce::Repository::ValueIdx("idxValue");
			ctn.metaDataName = METADATA_COMBINE_KEYNAME;

			std::string mdFilename = metaDataCategory(METADATA_COMBINE_KEYNAME);

			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(ctn.idxValue);

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "create evictor %s with index %s"), "metaValue", "ValueIdx");
#if ICE_INT_VERSION / 100 >= 303
			ctn.eMetaData = ::Freeze::createBackgroundSaveEvictor(_adapter, _dbPath,  (mdFilename+".dat").c_str(), 0, indices);
#else
			ctn.eMetaData = Freeze::createEvictor(_adapter, _dbPath, (mdFilename+".dat").c_str(), 0, indices);
#endif
			_adapter->addServantLocator(ctn.eMetaData, "");
			ctn.eMetaData->setSize(1000);

#if ICE_INT_VERSION / 100 >= 306
			WLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#else
			IceUtil::WLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#endif
			_metaDataContainerMap.insert(MetaDataContainerMap::value_type(ctn.metaDataName, ctn));
			_log(ZQ::common::Log::L_INFO, CLOGFMT(MetaLib, "metadata[%s] initialized as %s"), ctn.metaDataName.c_str(), mdFilename.c_str());

		}
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow< ::TianShanIce::ServerError> (_log, EXPFMT(MetaLib, 1001, "MetaLib() caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow< ::TianShanIce::ServerError> (_log, EXPFMT(MetaLib, 1001, "MetaLib() caught unknown exception"));
	}
	if(!_bIsCombineDB)
	{
		std::string strPattern = METADATA_FILENAME_PREFFIX "*.dat";
		std::vector<std::string> filesV = FS::searchFiles(_dbPath, strPattern);
		std::vector<std::string>::iterator it;
		for(it = filesV.begin(); it != filesV.end(); it++)
		{
			std::string mdfile = *it;
            size_t pos = mdfile.find_last_of(".");
            std::string metaDataName = pos > 2 ? mdfile.substr(strlen(METADATA_FILENAME_PREFFIX), pos-strlen(METADATA_FILENAME_PREFFIX)) : mdfile;

            bool indexFlag = true;
            for(::TianShanIce::StrValues::const_iterator iter = _nonIndices.begin(); iter != _nonIndices.end(); iter++)
            {
                if(metaDataName == *iter)
                {
                    indexFlag = false;
                    break;
                }
            }
            try {
                if(indexFlag)
                {
                    openMetaData(metaDataName, indexFlag);
                }
            }
            catch(...) {} // already logged the errors in openMetaData()
		}

	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(MetaLib, "database ready"));
}

void MetaLibImpl::openMetaData(const std::string& metaDataName, bool indexFlag)
{
	{
#if ICE_INT_VERSION / 100 >= 306
		RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#else
		IceUtil::RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#endif
		MetaDataContainerMap::iterator cit = _metaDataContainerMap.find(metaDataName);
		if (_metaDataContainerMap.end() != cit)
			return; // already opened
	}	
	try 
	{	
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "openMetaData() metadata[%s], initializing"), metaDataName.c_str());
		MetaDataContainer ctn;
		ctn.metaDataName = metaDataName;
		std::vector<Freeze::IndexPtr> indices;
		if(indexFlag)
		{
			ctn.idxValue = new TianShanIce::Repository::ValueIdx(std::string(METADATAVALINDEX_FN_PREFFIX) + metaDataName);
			indices.push_back(ctn.idxValue);
		}
		else
		{
			ctn.idxValue = NULL;
		}

		std::string mdFilename = metaDataCategory(metaDataName);

		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "create evictor %s with value index"), mdFilename.c_str());
 		Ice::PropertiesPtr proper = _adapter->getCommunicator()->getProperties();
		std::string evictorAttrPrefix = std::string("Freeze.Evictor.");
		proper->setProperty(evictorAttrPrefix + (std::string)METADATA_FILENAME_PREFFIX + metaDataName + ".dat.$default.BtreeMinKey",      "16");
//		proper->setProperty(evictorAttrPrefix + (std::string)METADATA_FILENAME_PREFFIX + metaDataName + ".dat.$index:$default." + std::string(METADATAVALINDEX_FN_PREFFIX) + metaDataName + ".BtreeMinKey",      "64");
		proper->setProperty(evictorAttrPrefix + (std::string)METADATA_FILENAME_PREFFIX + metaDataName + ".dat.PageSize",      "8192");
#if ICE_INT_VERSION / 100 >= 303
		ctn.eMetaData = ::Freeze::createBackgroundSaveEvictor(_adapter, _dbPath, (mdFilename+".dat").c_str(), 0, indices);
#else
		ctn.eMetaData = ::Freeze::createEvictor(_adapter, _dbPath, (mdFilename+".dat").c_str(), 0, indices);
#endif
		_adapter->addServantLocator(ctn.eMetaData, mdFilename.c_str());
		ctn.eMetaData->setSize(100);
#if ICE_INT_VERSION / 100 >= 306	
		WLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#else
		IceUtil::WLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#endif
		_metaDataContainerMap.insert(MetaDataContainerMap::value_type(ctn.metaDataName, ctn));
		_log(ZQ::common::Log::L_INFO, CLOGFMT(MetaLib, "metadata[%s] initialized as %s"), metaDataName.c_str(), mdFilename.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow< ::TianShanIce::ServerError> (_log, EXPFMT(MetaLib, 2001, "openMetaData() caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow< ::TianShanIce::ServerError> (_log, EXPFMT(MetaLib, 2001, "openMetaData() caught unknown exception"));
	}
}

void MetaLibImpl::closeDB()
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "closing database at path: %s"), _dbPath.c_str());
	
#if ICE_INT_VERSION / 100 >= 306	
	WLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#else
	IceUtil::WLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#endif
	_metaDataContainerMap.clear();

	_eMetaObj = NULL;
	_idxType = NULL;
}

void MetaLibImpl::registerMetaClass(const ::std::string& type, const ::TianShanIce::Repository::MetaDataMap& metaDataSchema, const ::Ice::Current& c)
{
	///if the metaData evictor doesn't need combine
	///create the evictor for each metadata
	if(!_bIsCombineDB)
	{	
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "registering meta class type: %s"), type.c_str());

		for (::TianShanIce::Repository::MetaDataMap::const_iterator it = metaDataSchema.begin(); it != metaDataSchema.end(); it++)
		{
			bool indexFlag = true;
			std::string mdName = it->first;
			if(it->second.value == METADATA_NONINDEX)
				indexFlag = false;
			
			if (mdName.empty() || std::string::npos != mdName.find_first_of("? \t"))
				ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (_log, EXPFMT(MetaLib, 2005, "registerMetaClass() class[%s] illegal metadata name[%s]"), type.c_str(), mdName.c_str());
			if(indexFlag)
			{
				openMetaData(mdName, indexFlag);
			}
		}
	}
}

void MetaLibImpl::setDefaultClassType(const ::std::string& type, const ::Ice::Current& c)
{
	_defaultType = type;
}

::std::string MetaLibImpl::getDefaultClassType(const ::Ice::Current& c)
{
	return _defaultType;
}

void MetaLibImpl::removeObjects(const ::TianShanIce::StrValues& objIds, const ::Ice::Current& c)
{

	typedef std::vector < MetaLibImpl::MetaDataContainer > MetaDataContainers;
	MetaDataContainers mdContainers;
	{
#if ICE_INT_VERSION / 100 >= 306
		RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#else
		IceUtil::RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#endif
		for (MetaDataContainerMap::iterator it =_metaDataContainerMap.begin(); it != _metaDataContainerMap.end(); it++)
			mdContainers.push_back(it->second);
	}

	for (::TianShanIce::StrValues::const_iterator itObjId = objIds.begin(); itObjId < objIds.end(); itObjId ++)
	{
		::Ice::Identity ident;
		ident.name = *itObjId; ident.category = META_OBJECT;
		
		try 
		{	
//			IceUtil::LockT<IceUtil::RecMutex> lk(_evitObjLock);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "cleaning object[%s:%s]"), ident.name.c_str(), ident.category.c_str());
			_eMetaObj->remove(ident);
		}
		catch(const Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "removeObject() delete object[%s:%s] caught exception[%s]"), ident.name.c_str(), ident.category.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "removeObject() delete object[%s:%s] caught unknown exception"), ident.name.c_str(), ident.category.c_str());
		}

		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "removeObject() cleaning metadatas of obj[%s:%s]"), ident.name.c_str(), ident.category.c_str());

		if(!_bIsCombineDB)
		{
			for (MetaDataContainers::iterator it =mdContainers.begin(); it < mdContainers.end(); it++)
			{
				try 
				{
					it->eMetaData->remove(covertToMetaDataIdent(ident, it->metaDataName));
				}
				catch(const Ice::NotRegisteredException&)
				{
					// skip log here
				}
				catch(const Ice::Exception& ex)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "removeObject() cleaning metadata[%s] caught exception[%s]"), it->metaDataName.c_str(), ex.ice_name().c_str());
				}
				catch(...)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "removeObject() cleaning metadata[%s] caught unknown exception"), it->metaDataName.c_str());
				}
			}		
		}
		else
		{
			std::string ctnkeyname = METADATA_COMBINE_KEYNAME;

			Freeze::EvictorPtr eMetaData;
			{
#if ICE_INT_VERSION / 100 >= 306
				RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#else
				IceUtil::RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#endif
				MetaDataContainerMap::iterator it =_metaDataContainerMap.find(ctnkeyname);
				if (_metaDataContainerMap.end() == it)
					ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (_log, EXPFMT(MetaLib, 3001, "setMetaData() metaData[%s] hasn't been registered"), ctnkeyname.c_str());

				eMetaData = it->second.eMetaData;	
				// get the iterater of evictor and go thru each of the item
				::Freeze::EvictorIteratorPtr its;
				its = eMetaData->getIterator("", MAX_BATCH_ITERATOR_SIZE);
				while(its->hasNext())
				{
					::Ice::Identity ident = its->next();
					if(ident.name == *itObjId)
						eMetaData->remove(ident);
				}
			}
		}		
	}
}

void MetaLibImpl::removeObject(const ::std::string& objId, const ::Ice::Current& c)
{
	::TianShanIce::StrValues objIds;
	objIds.push_back(objId);

	removeObjects(objIds, c);
}

void MetaLibImpl::setMetaData(const ::std::string& objId, const ::std::string& name, const ::TianShanIce::Repository::MetaDataValue& value, bool skipSearch, const ::Ice::Current& c)
{
	::Ice::Identity identObj;
	identObj.name = objId; identObj.category = META_OBJECT;
	
	setMetaDataEx(identObj, name, value, skipSearch);
}

void MetaLibImpl::setMetaDataEx(const ::Ice::Identity& identObj, const ::std::string& name, const ::TianShanIce::Repository::MetaDataValue& value, bool skipSearch, bool bVerifyObj)
{
#ifndef  NDEBUG
	try {
		if (bVerifyObj)
			IdentityToObj2(LibMetaObject, identObj);
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow< ::TianShanIce::EntityNotFound> (_log, EXPFMT(MetaLib, 3000, "setMetaData() failed to open MetaObject[%s], exception[%s]"), identObj.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow< ::TianShanIce::EntityNotFound> (_log, EXPFMT(MetaLib, 3000, "setMetaData() failed to open MetaObject[%s], unknown exception"), identObj.name.c_str());
	}
#endif // !NDEBUG

	::Ice::Identity ident = covertToMetaDataIdent(identObj, name);
    
	std::string ctnkeyname = name;
	if(_bIsCombineDB)
	{
		ctnkeyname = METADATA_COMBINE_KEYNAME;
	}

	Freeze::EvictorPtr eMetaData;
	{
#if ICE_INT_VERSION / 100 >= 306
		RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#else
		IceUtil::RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#endif
		MetaDataContainerMap::iterator it =_metaDataContainerMap.find(ctnkeyname);
		if (_metaDataContainerMap.end() == it)
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (_log, EXPFMT(MetaLib, 3001, "setMetaData() metaData[%s] hasn't been registered"), name.c_str());

		eMetaData = it->second.eMetaData;
	}

	::TianShanIce::Repository::LibMetaValuePrx MDVal;
	if(!skipSearch)
	{
		::Ice::Long stamp = ZQTianShan::now();
		try
		{
			MDVal = IdentityToObj2(LibMetaValue, ident);

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "setMetaData() [%s:%s] set %s=%s"), identObj.name.c_str(), identObj.category.c_str(), name.c_str(), value.value.c_str());
			MDVal->set(value);
			return;
		}
		catch (...) 
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "IdentityToObj2() [%s:%s] took %lldms"), identObj.name.c_str(), identObj.category.c_str(), ZQTianShan::now() - stamp);
		}
	}

	try
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "setMetaData() adding the new value[%s] of obj[%s:%s] into DB"), name.c_str(), identObj.name.c_str(), identObj.category.c_str());
		LibMetaValueImpl::Ptr pMDVal = new LibMetaValueImpl(*this);
		if (!pMDVal)
			ZQTianShan::_IceThrow< ::TianShanIce::ServerError> (_log, EXPFMT(MetaLib, 3002, "setMetaData() out of memory"));
		
		pMDVal->identObj = identObj;
		pMDVal->value    = value.value;
		pMDVal->hintedType = value.hintedType;
		
		eMetaData->add(pMDVal, ident);	
		_log(ZQ::common::Log::L_INFO, CLOGFMT(MetaLib, "setMetaData() added the new value[%s] of obj[%s:%s] into DB"), name.c_str(), identObj.name.c_str(), identObj.category.c_str());
		
		MDVal = IdentityToObj2(LibMetaValue, ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(_log, EXPFMT(MetaLib, 3003, "setMetaData() caught %s: %s"), 
			ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::AlreadyRegisteredException& ex)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "setMetaData() ice exception[%s]"), ex.ice_name().c_str());
	}
	catch (Ice::Exception& ex) 
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(_log, EXPFMT(MetaLib, 3003, "setMetaData() ice exception[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(_log, EXPFMT(MetaLib, 3003, "setMetaData() unknown exception"));
	}
}
::TianShanIce::Repository::MetaDataValue 
MetaLibImpl::getMetaData(const ::Ice::Identity& identObj, const ::std::string& key)
{
	::TianShanIce::Repository::MetaDataValue ret;
	ret.hintedType = ::TianShanIce::vtUnknown;

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "getMetaData() metaData[%s, %s]]"),
		identObj.name.c_str(), key.c_str());

	std::string ctnkeyname;
	if(_bIsCombineDB)
	{
		ctnkeyname = METADATA_COMBINE_KEYNAME;
	}
	else
	{
		ctnkeyname = key;
	}	
	MetaLibImpl::MetaDataContainer mdctn;	
	Freeze::EvictorPtr eMetaData;
	{
#if ICE_INT_VERSION / 100 >= 306
		RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#else
		IceUtil::RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#endif
		MetaDataContainerMap::iterator it =_metaDataContainerMap.find(ctnkeyname);
		if (_metaDataContainerMap.end() == it)
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (_log, EXPFMT(MetaLib, 3001, "gettMetaData() metaData[%s] hasn't been registered"), ctnkeyname.c_str());

		mdctn.eMetaData = it->second.eMetaData;	

		// get the iterater of evictor and go thru each of the item
		Ice::Identity mdValueIdent = covertToMetaDataIdent(identObj, key);
		::Freeze::EvictorIteratorPtr its;
		its = mdctn.eMetaData->getIterator("", MAX_BATCH_ITERATOR_SIZE);
		while(its->hasNext())
		{
			::Ice::Identity identValue = its->next();
			if(identValue == mdValueIdent)
			{
				::TianShanIce::Repository::LibMetaValuePrx MDVal;
				try
				{
					MDVal = IdentityToObj2(LibMetaValue, identValue);
					ret = MDVal->get();
				}
				catch (Ice::Exception& ex) 
				{
					ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (_log, 
						EXPFMT(MetaLib, 3002, "getMetaData() metaData[%s, %s] caught Ice exception %s"), 
						identObj.name.c_str(), key.c_str(), ex.ice_name().c_str());
				}
				break;
			}
		}
	}
	return ret;
}

void MetaLibImpl::setMetaDatas(const ::std::string& objId, const ::TianShanIce::Repository::MetaDataMap& valueMap, bool skipSearch, const ::Ice::Current& c)
{
	::Ice::Identity identObj;
	identObj.name = objId; identObj.category = META_OBJECT;
	
	for (::TianShanIce::Repository::MetaDataMap::const_iterator it = valueMap.begin(); it != valueMap.end(); it++)
	{
		try
		{
			setMetaDataEx(identObj, it->first, it->second, skipSearch);
		}
		catch (::TianShanIce::InvalidParameter&)
		{
			continue;
		}
	}
}
/*
void MetaLibImpl::addMetaDatas(const ::std::string& objId, const ::TianShanIce::Repository::MetaDataMap& valueMap, const ::Ice::Current& c)
{
	::Ice::Identity identObj;
	identObj.name = objId; identObj.category = META_OBJECT;

	for (::TianShanIce::Repository::MetaDataMap::const_iterator it = valueMap.begin(); it != valueMap.end(); it++)
	{
		try
		{
			::Ice::Identity ident = covertToMetaDataIdent(identObj, it->first);

			std::string ctnkeyname = it->first;
			if(_bIsCombineDB)
			{
				ctnkeyname = METADATA_COMBINE_KEYNAME;
			}

			Freeze::EvictorPtr eMetaData;
			{
				IceUtil::RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
				MetaDataContainerMap::iterator iter =_metaDataContainerMap.find(ctnkeyname);
				if (_metaDataContainerMap.end() == iter)
					ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (_log, EXPFMT(MetaLib, 3001, "setMetaData() metaData[%s] hasn't been registered"), it->first.c_str());

				eMetaData = iter->second.eMetaData;
			}

			try
			{
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "addMetaData() adding the new value[%s] of obj[%s:%s] into DB"), it->first.c_str(), identObj.name.c_str(), identObj.category.c_str());
				LibMetaValueImpl::Ptr pMDVal = new LibMetaValueImpl(*this);
				if (!pMDVal)
					ZQTianShan::_IceThrow< ::TianShanIce::ServerError> (_log, EXPFMT(MetaLib, 3002, "setMetaData() out of memory"));

				pMDVal->identObj = identObj;
				pMDVal->value    = it->second.value;
				pMDVal->hintedType = it->second.hintedType;

				eMetaData->add(pMDVal, ident);	
				_log(ZQ::common::Log::L_INFO, CLOGFMT(MetaLib, "addMetaData() added the new value[%s] of obj[%s:%s] into DB"), it->first.c_str(), identObj.name.c_str(), identObj.category.c_str());

				::TianShanIce::Repository::LibMetaValuePrx MDVal = IdentityToObj2(LibMetaValue, ident);
			}
			catch (Ice::Exception& ex) 
			{
				::ZQTianShan::_IceThrow<TianShanIce::ServerError>(_log, EXPFMT(MetaLib, 3003, "setMetaData() ice exception[%s]"), ex.ice_name().c_str());
			}
			catch (...)
			{
				::ZQTianShan::_IceThrow<TianShanIce::ServerError>(_log, EXPFMT(MetaLib, 3003, "setMetaData() unknown exception"));
			}
		}
		catch (::TianShanIce::InvalidParameter&)
		{
			continue;
		}
	}
}
*/
::TianShanIce::Repository::MetaObjectInfo MetaLibImpl::openObject(const ::std::string& objId, const ::TianShanIce::StrValues& expectedMetaDataNames, const ::Ice::Current& c)
{
	::Ice::Identity identObj;
	identObj.name = objId; identObj.category = META_OBJECT;

	::TianShanIce::Repository::LibMetaObjectPrx obj = IdentityToObj(LibMetaObject, identObj);
	if (!obj)
		::ZQTianShan::_IceThrow<TianShanIce::EntityNotFound>(_log, EXPFMT(MetaLib, 5001, "openObject() obj[%s:%s] not found"), identObj.name.c_str(), identObj.category.c_str());

	::TianShanIce::Repository::MetaObjectInfo ret;
	ret.id = identObj.name;
	ret.type = identObj.category;

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "openObject() collecting metadata for new obj[%s:%s]"), identObj.name.c_str(), identObj.category.c_str());
	for (::TianShanIce::StrValues::const_iterator it = expectedMetaDataNames.begin(); it < expectedMetaDataNames.end(); it++)
	{
		if (it->empty())
			continue;

		::TianShanIce::Repository::LibMetaValuePrx MDVal = IdentityToObj(LibMetaValue, covertToMetaDataIdent(identObj, *it));
		if (!MDVal)
			continue;
		
		ret.metaDatas.insert(::TianShanIce::Repository::MetaDataMap::value_type(*it, MDVal->get()));
	}

	return ret;
}

std::string MetaLibImpl::createObject(const ::std::string& type, ::Ice::Long timeout, const ::Ice::Current& c)
{
	::Ice::Identity identObj;
	identObj.category = META_OBJECT;
	::TianShanIce::Repository::LibMetaObjectPrx obj;

	do {
		static ZQ::common::Guid guid;
		guid.create();
		char buf[32];
		guid.toCompactIdstr(buf, sizeof(buf)-2);
		identObj.name = buf;
        try
		{
		   obj = IdentityToObj2(LibMetaObject, identObj);
		}
		catch(Ice::ObjectNotExistException&)
		{
			obj = NULL;
		}
		catch(Ice::Exception&)
		{
		}
	} while (obj);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "createObject() adding new obj[%s:%s]"), identObj.name.c_str(), identObj.category.c_str());
	LibMetaObjectImpl::Ptr pObj = new LibMetaObjectImpl(*this);
	
	if (!pObj)
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(_log, EXPFMT(MetaLib, 5002, "openObject() creating obj[%s:%s] out of memory"), identObj.name.c_str(), identObj.category.c_str());
	
	try {
		pObj->ident = identObj;
		pObj->type = !type.empty() ? type : _defaultType;
		pObj->stampCreated = ZQTianShan::now(); ///< timestamp as of created
		pObj->timeout = timeout;
		
		_eMetaObj->add(pObj, identObj);	
		_log(ZQ::common::Log::L_INFO, CLOGFMT(MetaLib, "openObject() added new obj[%s:%s]"), identObj.name.c_str(), identObj.category.c_str());
		
#ifdef _DEBUG
		obj = IdentityToObj2(LibMetaObject, identObj);
#endif // _DEBUG

		return identObj.name;
	}
	catch (Ice::Exception& ex) 
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(_log, EXPFMT(MetaLib, 5003, "openObject() ice exception[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(_log, EXPFMT(MetaLib, 5003, "openObject() unknown exception"));
	}

	return "";
}

// TianShanIce::Repository::LibMetaObjectPtr MetaLibImpl::newObject(const ::std::string& type, ::Ice::Long timeout)
// {
// 	if (0 == type.compare(META_OBJECT))
// 		return new LibMetaObjectImpl(*this);
// 	else
// 		return NULL;
// }

bool MetaLibImpl::addObject(::Ice::ObjectPtr obj, Ice::Identity ident)
{
	try {
//		IceUtil::LockT<IceUtil::RecMutex> lk(_evitObjLock);
		_eMetaObj->add(obj, ident);	
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "openObject() added new obj[%s:%s]"), ident.name.c_str(), ident.category.c_str());
	}
	catch(Freeze::DatabaseException& dx)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "openObject() added new object[%s:%s] caught exception[%s]"), ident.name.c_str(), ident.category.c_str(), dx.message.c_str());
	}
	catch (Ice::AlreadyRegisteredException& ex)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "openObject() added new object[%s:%s] caught exception[%s]"), ident.name.c_str(), ident.category.c_str(), ex.ice_name().c_str());
		throw;
	}
	catch (Ice::Exception& ex) 
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib, "openObject() added new object[%s:%s] caught exception[%s]"), ident.name.c_str(), ident.category.c_str(), ex.ice_name().c_str());
		throw;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib, "openObject() added new object[%s:%s] caught unknown exception"), ident.name.c_str(), ident.category.c_str());
		throw;
	}
	return true;
}

void MetaLibImpl::removeObject(::Ice::Identity _ident)
{
	typedef std::vector < MetaLibImpl::MetaDataContainer > MetaDataContainers;
	MetaDataContainers mdContainers;
	{
#if ICE_INT_VERSION / 100 >= 306
		RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#else
		IceUtil::RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#endif
		for (MetaDataContainerMap::iterator it =_metaDataContainerMap.begin(); it != _metaDataContainerMap.end(); it++)
			mdContainers.push_back(it->second);
	}

	try 
	{	
//		IceUtil::LockT<IceUtil::RecMutex> lk(_evitObjLock);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "cleaning object[%s:%s]"), _ident.name.c_str(), _ident.category.c_str());
		_eMetaObj->remove(_ident);
	}
	catch(const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "removeObject() delete object[%s:%s] caught exception[%s]"), _ident.name.c_str(), _ident.category.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "removeObject() delete object[%s:%s] caught unknown exception"), _ident.name.c_str(), _ident.category.c_str());
	}

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLib, "removeObject() cleaning metadatas of obj[%s:%s]"), _ident.name.c_str(), _ident.category.c_str());

	if(!_bIsCombineDB)
	{
		for (MetaDataContainers::iterator it =mdContainers.begin(); it < mdContainers.end(); it++)
		{
			try 
			{
				it->eMetaData->remove(covertToMetaDataIdent(_ident, it->metaDataName));
			}
			catch(const Ice::NotRegisteredException&)
			{
				// skip log here
			}
			catch(const Ice::Exception& ex)
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "removeObject() cleaning metadata[%s] caught exception[%s]"), it->metaDataName.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLib, "removeObject() cleaning metadata[%s] caught unknown exception"), it->metaDataName.c_str());
			}
		}		
	}
	else
	{
		std::string ctnkeyname = METADATA_COMBINE_KEYNAME;

		Freeze::EvictorPtr eMetaData;
		{
#if ICE_INT_VERSION / 100 >= 306
			RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#else
			IceUtil::RLockT <IceUtil::RWRecMutex> lk(_metaDataContainerLocker);
#endif
			MetaDataContainerMap::iterator it =_metaDataContainerMap.find(ctnkeyname);
			if (_metaDataContainerMap.end() == it)
				ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (_log, EXPFMT(MetaLib, 3001, "setMetaData() metaData[%s] hasn't been registered"), ctnkeyname.c_str());

			eMetaData = it->second.eMetaData;	
			// get the iterater of evictor and go thru each of the item
			::Freeze::EvictorIteratorPtr its;
			its = eMetaData->getIterator("", MAX_BATCH_ITERATOR_SIZE);
			while(its->hasNext())
			{
				::Ice::Identity ident = its->next();
				if(ident.name == _ident.name)
					eMetaData->remove(ident);
			}
		}
	}		
}


void MetaLibImpl::lookup_async(const ::TianShanIce::Repository::AMD_MetaDataLibrary_lookupPtr& amdCB, const ::std::string& type, const ::TianShanIce::Properties& searchForMetaData, const ::TianShanIce::StrValues& expectedMetaDataNames, const ::Ice::Current& c)
{
	try {
		(new LookupCmd(*this, amdCB, type, searchForMetaData, expectedMetaDataNames))->execute();
	}
	catch(const ::Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib,"lookUp_async() failed to initialize LookupCmd: exception[%s]"), ex.ice_name().c_str());
		amdCB->ice_exception(::TianShanIce::ServerError("MetaLib", 500, "failed to initialize LookupCmd: exception"));
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib,"lookUp_async() failed to initialize LookupCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("MetaLib", 500, "failed to initialize LookupCmd"));
	}
}

/*
// -----------------------------
// callback OnProvisionStreamableAmiCBImpl
// -----------------------------
OnProvisionStreamableAmiCBImpl::OnProvisionStreamableAmiCBImpl(MetaLib& env)
:_env(env)
{
}

void OnProvisionStreamableAmiCBImpl::ice_exception(const ::Ice::Exception& ex)
{
	_env.logProvisionSessionBindAmiCBException("OnProvisionStreamable", ex);
}

// -----------------------------
// callback OnProvisionDestroyedAmiCBImpl
// -----------------------------
OnProvisionDestroyedAmiCBImpl::OnProvisionDestroyedAmiCBImpl(MetaLib& env)
:_env(env)
{
}

void OnProvisionDestroyedAmiCBImpl::ice_exception(const ::Ice::Exception& ex)
{
	_env.logProvisionSessionBindAmiCBException("OnProvisionDestroyed", ex);
}
*/
}} // namespace
