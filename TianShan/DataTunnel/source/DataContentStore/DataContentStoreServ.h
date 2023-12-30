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
// $Header: /ZQProjs/TianShan/DataTunnel/source/DataContentStore/DataContentStoreServ.h 1     10-11-12 16:05 Admin $
// $Log: /ZQProjs/TianShan/DataTunnel/source/DataContentStore/DataContentStoreServ.h $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 1     09-03-09 10:33 Li.huang
// 
// 1     08-12-08 11:11 Li.huang
// 
// 5     08-10-29 15:24 Li.huang
// 
// 4     07-04-26 18:11 Ken.qian
// 
// 3     07-04-20 16:32 Ken.qian
// 
// 2     07-04-20 10:37 Ken.qian
// 
// 1     07-04-11 20:10 Ken.qian

#ifndef __ZQ_DODContentStoreServ_h__
#define __ZQ_DODContentStoreServ_h__


#define BUFSIZE 256

#include "BaseZQServiceApplication.h"
#include "ConfigLoader.h"
#include "MiniDump.h"

#include "DataTunnelGraphFactory.h"
#include "GraphPool.h"

#include "DataContentStoreImpl.h"
#include "TianShanDefines.h"

class DODContentStoreServ : public ZQ::common::BaseZQServiceApplication
{
	friend class TianShanIce::Storage::DataOnDemand::DODContentStoreI;
	friend class TianShanIce::Storage::DataOnDemand::DODContentI;
public:
	DODContentStoreServ();
	virtual ~DODContentStoreServ();
public:
	HRESULT OnInit(void);
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnUnInit(void);

	bool isHealth(void);
	void exitProcess(void);	

public:
	ZQ::Content::Process::GraphPool& getGraphPool() { return *_graphPool; }

protected:
	Ice::PropertiesPtr                 _properties;
	Ice::CommunicatorPtr			   _communicator;
	ZQADAPTER_DECLTYPE              _adapter;
	TianShanIce::Storage::DataOnDemand::DODContentStoreIPtr                _dodStoreI;

protected:
	DODGraphFactory*                   _dodGraphFactory;
	ZQ::Content::Process::GraphPool*   _graphPool;

private:
	ZQ::common::MiniDump _minidump;

	static void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress);

};


/*class DODContentStoreConfig : public ZQ::common::ConfigLoader
{
public:
	DODContentStoreConfig();
	virtual ~DODContentStoreConfig(){}
public:
	virtual ZQ::common::ConfigLoader::ConfigSchemaItem* getSchema();

public:
	
	 <Default> 
	
	char dbPath[BUFSIZE];
	char logPath[BUFSIZE];
	
	<IceProperties> 
	
	
	<ContentStore>
	

	<Adapter> 
	char endpoint[BUFSIZE];
	
    <Host> 
	char netId[BUFSIZE];

	 <Service> 
	DWORD maxSessionCount;          // Graph count in GraphPool
	DWORD buffPoolSize;             // Buffer Pool size in each Graph
	DWORD buffSize;                 // Buffer size of Graph's buff
	DWORD traceProvDetails;         // boolean, trace provision details
	DWORD progressReportInterval;   // progress report interval in seconds
	char  homeDirectory[BUFSIZE];   // the Home directory for output of wrapped ts files
	char  desURLPrefix[BUFSIZE];    // the prefix of destination URL
	DWORD yieldTime;                // yield time for datawrapping

	<Log> 
	char  logFile[BUFSIZE];
	DWORD logLevel;
	DWORD logBuffer;
	DWORD logSize;
	DWORD logTimeout;	
};*/
#endif