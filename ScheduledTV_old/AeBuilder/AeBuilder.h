
// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: AeBuilder.h
// Author: Kaliven Lee
// Desc  : Build a asset element list
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_old/AeBuilder/AeBuilder.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 9     04-10-08 10:43 Kaliven.lee
// 
// 8     04-09-29 9:40 Kaliven.lee
// compatible with filler
// 
// 7     04-09-23 12:22 Kaliven.lee
// get the inform once
// 
// 6     04-09-22 18:51 Kaliven.lee
// get detail information of element
// 
// 5     04-09-20 18:15 Kaliven.lee
// 
// 4     04-09-20 10:38 Kaliven.lee
// modify the interface of getElementList 
// 
// 3     04-09-16 17:16 Kaliven.lee
#pragma  once

#include "AeBuilderConf.h"
#include "log.h"

using ZQ::common::Log;



class AeBuilder
{
public:
	/// constructor function
	AeBuilder();
	~AeBuilder();
	/// initialize	AeBuilder
	/// @param	wszServer	IN	server name or ip address									
	/// @param	wszUser		IN	user name.any user name in lab environment						
	/// @param	dwTimeOut	IN	time out for IDS session defaultly ID_DEFAULT_WAIT_TIME_OUT
	/// @param	IDSVer		IN	version of IDS in the SeaChange system.Default  IDS_VERSION_1_0	
	/// @return					if the version does not match the version of IDS in your system or the
	///							process of bind IDS, returned false. otherwise true.
	/// @note					in lab environment, wszuser can be set as any string.
	bool initialize(wchar_t* wszServer,wchar_t* wszUser,DWORD dwTimeOut = IDS_DEFAULT_WAIT_TIME_OUT,WORD IDSVer = IDS_VERSION_1_0);
	/// get ElementList	according AssetID
	/// @param dwAssetID	IN		ID of Asset you want to query 							
	/// @param pElementList	IN/OUT	pointer of element list of Asset allocate by caller,input 
	///								the list allocates by caller		
	/// @return						Error code. if success return S_OK,otherwise return error code define 
	///								in ITV_ERR.h
	/// @note						caller must allocate the memory ofelementList 
	HRESULT getElementList(DWORD dwAssetID,PASSETELEMENTS pElementList);
	/// get ElementList	according AssetList							
	/// @param pAssetList	IN/OUT	pointer of Asset Element list of Asset allocate by caller,input 
	///								the list allocates by caller.Caller only need to fill the UID in the 
	///								struct of Asset.		
	/// @return						Error code. if success return S_OK,otherwise return error code define 
	///								in ITV_ERR.h
	/// @note						call must allocate the memory to storage the element list
	HRESULT getElementList(PASSETS pAssetList);
	/// unInitialize AeBuilder
	void unInitialize(void);
	
	/// write log in AE build process
	/// @param	msg			IN	msg need to write into file 
	///	@param	logLevel	IN	Log level. defined in common log.h
	static void LogEvent(int logLevel,char* msg,...);
private:
	bool reConnect(void);
public:
	static bool m_isConnected;
private:
	/// IDS servername 
	wchar_t m_wszServer[MAX_SQL_TNAME + 1];
	///	username of this session 
	wchar_t m_wszUser[MAX_SQL_TNAME + 1];
	/// IDS wait for time out
	DWORD m_dwTimeOut;	
	/// instance of IDS session
	IDSSESS m_IdsSess;
	/// version of ids
	WORD m_wVersion;

	bool m_isUnIntialized;
};