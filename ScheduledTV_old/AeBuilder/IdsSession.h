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
// Ident : $Id: AeBuidler.cpp
// Author: Kaliven Lee
// Desc  : IDS session manager
// IdsSession.h: interface for the IdsSession class.
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_old/AeBuilder/IdsSession.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 3     04-10-08 10:46 Kaliven.lee
// 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDSSESSION_H__25AD1AAD_1835_4905_A022_605B7ED960E0__INCLUDED_)
#define AFX_IDSSESSION_H__25AD1AAD_1835_4905_A022_605B7ED960E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "IdsConf.h"


class IdsSession  
{
public:
	/// Constructor
	IdsSession();
	/// Deconstructor
	virtual ~IdsSession();

public:
	/// initialize	IdsSession	
	/// @param	wszServer	IN	server name or ip address									
	/// @param	wszUser		IN	user name.any user name in lab environment						
	/// @param	dwTimeOut	IN	time out for IDS session defaultly ID_DEFAULT_WAIT_TIME_OUT
	/// @param	IDSVer		IN	version of IDS in the SeaChange system.Default  IDS_VERSION_1_0	
	/// @return					if the version does not match the version of IDS in your system 
	///							return false. otherwise true.
	/// @note					in lab environment, wszuser can be set as any string.
	bool initialize(wchar_t* wszServer,wchar_t*wszUser,DWORD dwTimeOut = IDS_DEFAULT_WAIT_TIME_OUT,WORD IDSVer = IDS_VERSION_1_0);

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
	/// get the meta data of specified application
	/// @param	appUid			IN		the uid of specified application
	/// @param	metaDataName	IN		the name of metaDataName u wana get
	/// @param	MeataData		OUT		the metaData retrieved
	/// @return							Error code. if success return S_OK,otherwise return error code define 
	///									in ITV_ERR.h
	HRESULT GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, METADATA& metaData);
	/// get the meta data of specified application
	/// @param	appUid			IN		the uid of specified application
	/// @param	metaDataName	IN		the name of metaDataName u wana get
	/// @param	wstrVal			OUT		the metaData retrieved
	/// @return							Error code. if success return S_OK,otherwise return error code define 
	///									in ITV_ERR.h
	HRESULT GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, std::wstring& wstrVal);
	/// get the meta data of specified application
	/// @param	appUid			IN		the uid of specified application
	/// @param	metaDataName	IN		the name of metaDataName u wana get
	/// @param	dwtrVal			OUT		the metaData retrieved
	/// @return							Error code. if success return S_OK,otherwise return error code define 
	///									in ITV_ERR.h
	HRESULT GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, DWORD& dwVal);
	/// get all the applications in server 
	/// @param apps		IN/OUT	the list of returned application name
	/// @return					Error code. if success return S_OK,otherwise return error code define 
	///							in ITV_ERR.h 
	HRESULT ListApplications(APPNAMES &apps);
	/// unInitialize IdsSession
	void unInitialize(void);
	
	/// write log in AE build process
	/// @param	msg			IN	msg need to write into file 
	///	@param	logLevel	IN	Log level. defined in common log.h
	static void LogEvent(int logLevel,char* msg,...);
private:
	/// reConnect to the server retry twice.
	/// @return		if success return true,else return false
	bool reConnect(void);
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

#endif // !defined(AFX_IDSSESSION_H__25AD1AAD_1835_4905_A022_605B7ED960E0__INCLUDED_)
