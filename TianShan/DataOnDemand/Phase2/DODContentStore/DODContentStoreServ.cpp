// ============================================================================================
// Copyright (c) 2006, 2007 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Ken Qian
// Desc  : DOD ContentStore Service 
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/DODContentStoreServ.cpp 2     1/02/14 3:04p Zonghuan.xiao $
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/DODContentStoreServ.cpp $
// 
// 2     1/02/14 3:04p Zonghuan.xiao
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 2     08-12-09 17:01 Li.huang
// 
// 1     08-12-08 11:11 Li.huang
// 
// 12    08-10-31 16:08 Li.huang
// 
// 11    08-10-31 15:42 Li.huang
// 
// 10    08-10-29 15:24 Li.huang
// 
// 9     07-08-06 13:44 Li.huang
// 
// 8     07-06-22 15:54 Li.huang
// 
// 7     07-04-26 18:11 Ken.qian
// 
// 6     07-04-20 16:32 Ken.qian
// 
// 5     07-04-20 10:37 Ken.qian
// 
// 4     07-04-17 10:54 Ken.qian
// 
// 3     07-04-16 15:07 Ken.qian
// 
// 2     07-04-16 11:03 Ken.qian
// 
// 1     07-04-11 20:10 Ken.qian



#define CONTENT_STORE_ADAPTER_NAME   "ContentStore"
#define MAX_SESSION_COUNT            5
#define MAX_YIELD_TIME               50                 // 50 ms


#include "DODContentStoreServ.h"
#include "ZQResource.h"
#include "Log.h"
#include "IceLog.h"
#include "DODContentStoreCfg.h"
#include "ice/Initialize.h"

/* DODConententStoreServ */
DODContentStoreServ g_dodCSServ;
ZQ::common::BaseZQServiceApplication *Application = &g_dodCSServ;

// DODContentStoreConfig config;
// ZQ::common::ConfigLoader* configLoader = &config;

ZQ::common::Config::Loader<DODContentStoreCfg> config("DODContentStore.xml");
ZQ::common::Config::ILoader *configLoader = &config;
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

DODContentStoreServ::DODContentStoreServ()
: _dodGraphFactory(NULL), _graphPool(NULL)
{
}

DODContentStoreServ::~DODContentStoreServ()
{
}


HRESULT DODContentStoreServ::OnInit(void)
{
	BaseZQServiceApplication::OnInit();

	glog(ZQ::common::Log::L_INFO, "DODContentStoreServ::OnInit() enter");

	/*
	* Get Configuration from Application Level
	*/

	/*	DBPath */
	size_t size = config.dbPath.size();
	if(size > 0 && config.dbPath[size-1] != '\\' || config.dbPath[size-1] != '/')
	{
		config.dbPath += "\\";
	}
	
	/* NetID */
	if(config.netId == "")
	{
		glog(ZQ::common::Log::L_ERROR, "[netId] must be configured");
		return E_HANDLE;
	}

	/* endPointer */
	if(config.endpoint == "")
	{
		glog(ZQ::common::Log::L_ERROR, "[endpoint] must be configured");
		return E_HANDLE;
	}	

	/* maxSessionCount */
	if(0 == config.maxSessionCount)
	{
		config.maxSessionCount = 1;
	}

	/* buffPoolSize */
	if(0 == config.buffPoolSize)
	{
		config.buffPoolSize = DEFAULT_BUFFER_POOL_SIZE;
	}
	
	/*buffSizeInPool*/
	if(0 == config.buffSize)
	{
		config.buffSize = DEFAULT_POOL_BUFFER_SIZE;
	}

	if(0 == config.progressReportInterval)
	{
		config.progressReportInterval = (DEFAULT_PROV_PROGRESS_RPT_INTERVAL / 1000); // turn to seconds
	}

	/* check homeDirectory */
	if(config.homeDirectory != "")
	{		
		CreateDirectoryA(config.homeDirectory.c_str(), NULL);
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR, "[homeDirectory] must be configured");
		return E_HANDLE;
	}

	WIN32_FIND_DATA FindData;
	HANDLE hFindFile = FindFirstFileA(config.homeDirectory.c_str(), &FindData);
	if( INVALID_HANDLE_VALUE == hFindFile)
	{
		glog(ZQ::common::Log::L_ERROR, "[homeDirectory] is not valid");
		return E_HANDLE;		
	}
	FindClose(hFindFile);
	// add / to the homedirectory
	int len = config.homeDirectory.size();
	if(config.homeDirectory[len-1] != '/' && 
		config.homeDirectory[len-1] != '\\')
	{
		config.homeDirectory += "\\";
	}

	if(config.desURLPrefix == "")
	{
		glog(ZQ::common::Log::L_ERROR, "[desURLPrefix] must be configured");
		return E_HANDLE;
	}

	if(config.yieldTime > MAX_YIELD_TIME)
	{
		config.yieldTime = MAX_YIELD_TIME;
	}
	/*
	* Get Configuration from Application's - ICE
	*/

	/* create the default Ice properties */
	_properties = Ice::createProperties();
	
	const std::map<std::string, std::string>& iceConfig = config.icePropMap;
	std::map<std::string, std::string>::const_iterator iter = iceConfig.begin();
	for (; iter != iceConfig.end(); ++iter) 
	{
		_properties->setProperty(iter->first, iter->second);
	}

	/*
	* initialize the mini dump
	*/
	_minidump.setDumpPath((char *)config.dbPath.c_str());
	_minidump.enableFullMemoryDump(true);
	_minidump.setExceptionCB(MiniDumpCallback);
	
	glog(ZQ::common::Log::L_INFO, "DODContentStoreServ::OnInit() leave");

	return S_OK;
}

HRESULT DODContentStoreServ::OnStart(void)
{
	glog(ZQ::common::Log::L_INFO, 
		"---------------------- DODContentStore (%s) loading -------------------", ZQ_PRODUCT_VER_STR3);
	
	glog(ZQ::common::Log::L_INFO, "DODContentStoreServ::OnStart() enter");

	BaseZQServiceApplication::OnStart();

	/*
	* Initialize Ice communicator properties
	*/
	int argc = 0;
	try
	{
		_properties->setProperty("ContentStore.Endpoints", config.endpoint);

		TianShanIce::common::IceLogIPtr icelog= new TianShanIce::common::IceLogI(m_pReporter);

//		_communicator = Ice::initializeWithPropertiesAndLogger(argc, 0, _properties, icelog);
		Ice::InitializationData  initdata;
		initdata.logger = icelog;
		initdata.properties = _properties;
		_communicator = Ice::initialize(argc, 0, initdata);

	}
	catch (const IceUtil::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, "Ice initialize properties met exception with error: %s", 
						ex.ice_name().c_str());
		return E_HANDLE;					
	}

	try
	{
		/*
		* Start myself as an ICE server
		*/
		_adapter = _communicator->createObjectAdapter(CONTENT_STORE_ADAPTER_NAME);

		_dodStoreI = new DODContentStoreI(config.netId);

		Ice::Identity cntstoreId;
		cntstoreId.name = CONTENT_STORE_ADAPTER_NAME;
	
		_adapter->add(_dodStoreI, cntstoreId);
		_adapter->activate();


		/*
		* Create DODGraphFactory and GraphPool
		*/
		_dodGraphFactory = new DODGraphFactory(config.maxSessionCount, m_pReporter, config.homeDirectory, config.yieldTime, config.buffPoolSize, config.buffSize);
		_graphPool = new ZQ::Content::Process::GraphPool(*_dodGraphFactory, m_pReporter, config.traceProvDetails);
	}
	catch(const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, "Create ContentStoreI failed with error %s", 
					ex.message);
		return E_HANDLE;		
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, "Create ContentStore Servant on endpoint %s met Ice exception with error: %s", 
					config.endpoint, ex.ice_name().c_str());
		return E_HANDLE;
	}
	catch(const char* msg)
	{
		glog(ZQ::common::Log::L_ERROR, "Create ContentStore Servant on endpoint %s met exception with error: %s", 
					config.endpoint, msg);
		return E_HANDLE;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, "Create ContentStore Servant on endpoint %s met unknow exception", 
					config.endpoint);
		return E_HANDLE;
	}
	
	glog(ZQ::common::Log::L_INFO, "DODContentStoreServ::OnStart() leave");
	
	return S_OK;
}

HRESULT DODContentStoreServ::OnStop(void)
{
	glog(ZQ::common::Log::L_INFO, "DODContentStoreServ::OnStop() enter");

	/* Ice object are not required to release, coz they are smart pointer object */

	// free graph, _dodGraphFactory is released by GraphPool internally
	if(_graphPool != NULL)
	{
		delete _graphPool;
	}
	_graphPool = NULL;

	BaseZQServiceApplication::OnStop();
	
	glog(ZQ::common::Log::L_INFO, "DODContentStoreServ::OnStop() leave");

	return S_OK;

}

HRESULT DODContentStoreServ::OnUnInit(void)
{
	try
	{
		if(_adapter != NULL)
		{
			_adapter->deactivate();
		}
		
		if(_communicator != NULL)
		{
			_communicator->destroy();
		}
		_properties = NULL;
		_adapter = NULL;
		_communicator = NULL;
		
		if(_dodStoreI != NULL)
		{
			_dodStoreI = NULL;
		}
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, "Ice destroy met exception with error: %s", 
						ex.ice_name().c_str());
		return E_HANDLE;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, "Ice destroy met unknown exception");
	}

	BaseZQServiceApplication::OnUnInit();

	return S_OK;	
}

bool DODContentStoreServ::isHealth(void)
{
	return true;
}

void DODContentStoreServ::exitProcess(void)
{
	exit(1);
}

void WINAPI DODContentStoreServ::MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();
	
	glog(ZQ::common::Log::L_ERROR,  "Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);

	glog.flush();
}



// DODContentStoreConfig::DODContentStoreConfig()
// {
// 	setConfigFileName("DODContentStore.xml");

	/* 
	* <Default> 
	*/
/*	memset(dbPath, 0x00, BUFSIZE*sizeof(char));*/

	/* <Adapter> */
/*	memset(endpoint, 0x00, BUFSIZE*sizeof(char));*/

	/* <Host> */
/*	memset(netId, 0x00, BUFSIZE*sizeof(char));*/

	/* <Service> */
// 	maxSessionCount = MAX_SESSION_COUNT;
// 	buffPoolSize = DEFAULT_BUFFER_POOL_SIZE; 
// 	buffSize = DEFAULT_POOL_BUFFER_SIZE;
// 	traceProvDetails = false;    
// 	progressReportInterval = (DEFAULT_PROV_PROGRESS_RPT_INTERVAL / 1000);  // turn to seconds
// 	memset(homeDirectory, 0x00, BUFSIZE*sizeof(char));
// 	memset(desURLPrefix, 0x00, BUFSIZE*sizeof(char)); 
// 	yieldTime = 0;
// 
// }
// 
// ZQ::common::ConfigLoader::ConfigSchemaItem* DODContentStoreConfig::getSchema()
// {
// 	static ConfigSchemaItem entry[] = {
		/*
		* <Default>
		*/
// 		{"Default/DatabaseFolder", 
// 				"path", dbPath, BUFSIZE*sizeof(char), true, ConfigSchemaItem::TYPE_STRING},
		/* 
		* <DODContentStore>
		*/

		/* <Adapter> */
// 		{"DODContentStore/Adapter", 
// 				"endpoint", endpoint, BUFSIZE*sizeof(char), true, ConfigSchemaItem::TYPE_STRING},

		/* <IceProperties> */
// 		{"DODContentStore/IceProperties",
// 				"prop", NULL, 0, true, ConfigSchemaItem::TYPE_ENUM},

		/* <Host> */
// 		{"DODContentStore/Host",
// 				"netId", netId, BUFSIZE*sizeof(char), true, ConfigSchemaItem::TYPE_STRING},
		
		/* <Service> */
// 		{"DODContentStore/Provision/maxSessionCount",
// 				"value", &maxSessionCount, sizeof(DWORD), true, ConfigSchemaItem::TYPE_INTEGER},
// 		{"DODContentStore/Provision/buffPoolSize",
// 				"value", &buffPoolSize, sizeof(DWORD), true, ConfigSchemaItem::TYPE_INTEGER},
// 		{"DODContentStore/Provision/buffSizeInPool",
// 				"value", &buffSize, sizeof(DWORD), true, ConfigSchemaItem::TYPE_INTEGER},
// 		{"DODContentStore/Provision/traceProvDetails",
// 				"value", &traceProvDetails, sizeof(DWORD), true, ConfigSchemaItem::TYPE_INTEGER},
// 		{"DODContentStore/Provision/progressReportInterval",
// 				"value", &progressReportInterval, sizeof(DWORD), true, ConfigSchemaItem::TYPE_INTEGER},
// 		{"DODContentStore/Provision/homeDirectory",
// 				"value", &homeDirectory, BUFSIZE*sizeof(char), true, ConfigSchemaItem::TYPE_STRING},
// 		{"DODContentStore/Provision/desURLPrefix",
// 				"value", &desURLPrefix, BUFSIZE*sizeof(char), true, ConfigSchemaItem::TYPE_STRING},
// 		{"DODContentStore/Provision/yieldTime",
// 				"value", &yieldTime, sizeof(DWORD), true, ConfigSchemaItem::TYPE_INTEGER},
		/*
		*	the end.
		*/
// 		{NULL, NULL, NULL, 0, true, ConfigSchemaItem::TYPE_STRING}
// 	};
// 	
// 	return entry;
// }
