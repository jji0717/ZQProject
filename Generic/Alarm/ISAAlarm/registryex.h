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
// Name  : registryex.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/9/2005
// Desc  : registry reading and writing
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/registryex.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-12 12:57p Daniel.wang
// 
// 1     05-07-11 3:33p Daniel.wang
// 
// 2     5/14/05 2:58p Daniel.wang
// 
// 1     05-03-30 14:57 Daniel.wang
// ===========================================================================

#ifndef _DANIEL_REGISTRYEX_H_
#define _DANIEL_REGISTRYEX_H_

#ifdef UNICODE
#undef _UNICODE
#undef UNICODE
#define USE_UNICODE
#endif

#include <windows.h>
#include <assert.h>


/// -----------------------------
/// RegistryEx_T
/// -----------------------------
/// Desc : registry reading and writing
/// @template - 
/// -----------------------------
class RegistryEx_T
{
public:
	enum Type
	{
		TP_NONE					= REG_NONE,
		TP_BINARY				= REG_BINARY,
		TP_DWORD_LITTLE_ENDIAN	= REG_DWORD_LITTLE_ENDIAN,
		TP_DWORD_BIG_ENDIAN		= REG_DWORD_BIG_ENDIAN,
		TP_EXPAND_SZ			= REG_EXPAND_SZ,
		TP_LINK					= REG_LINK,
		TP_MULTI_SZ				= REG_MULTI_SZ,
		TP_RESOURCE_LIST		= REG_RESOURCE_LIST,
		TP_SZ					= REG_SZ
	};

	enum Root
	{
		RT_HKEY_CLASSES_ROOT		= HKEY_CLASSES_ROOT,
		RT_HKEY_CURRENT_CONFIG		= HKEY_CURRENT_CONFIG,
		RT_HKEY_CURRENT_USER		= HKEY_CURRENT_USER,
		RT_HKEY_LOCAL_MACHINE		= HKEY_LOCAL_MACHINE,
		RT_HKEY_USERS				= HKEY_USERS,
		RT_HKEY_PERFORMANCE_DATA	= HKEY_PERFORMANCE_DATA,
		RT_HKEY_DYN_DATA			= HKEY_DYN_DATA
	};
private:
	HKEY	m_hKey;
public:

	/// constructor
	/// @param strPath - registry key path
	/// @param root - registry root
	/// @return - 
	RegistryEx_T(const char* strPath, Root root = RT_HKEY_LOCAL_MACHINE);

	/// desctuctor
	/// @param  - 
	/// @return - 
	~RegistryEx_T();


	/// judge if get an error ocupy
	/// @param  - 
	/// @return - return true if error
	bool IsError(void);

	/// get a registry item from registry [with type]
	/// @param strNode - node name
	/// @param tp - [out]registry type
	/// @param pData - [out]node data
	/// @param zLen - node data length limit
	/// @return - return true if ok
	bool Load(const char* strNode, RegistryEx_T::Type& tp,BYTE* pData, size_t zLen);

	/// get a registry item from registry [without type]
	/// @param strNode - node name
	/// @param pData - [out]node data
	/// @param zLen - node data length limit
	/// @return - return true if ok
	bool LoadNoType(const char* strNode, BYTE* pData, size_t zLen);

	/// save data into registry
	/// @param strNode - node name
	/// @param tp - registry type
	/// @param pData - node data
	/// @param zLen - data length
	/// @return - return true if ok
	bool Save(const char* strNode, RegistryEx_T::Type tp, const BYTE* pData, size_t zLen);

	/// load string from registry
	/// @param strNode - node name
	/// @param strData - [out]node data string
	/// @param zLen - string length limit
	/// @return - return true if ok
	bool LoadStr(const char* strNode, char* strData, size_t zLen);

	/// load dword data from registry
	/// @param strNode - node name
	/// @param dwData - [out]node data
	/// @return - return true if ok
	bool LoadDword(const char* strNode, DWORD& dwData);

	/// save a string into registry
	/// @param strNode - node name
	/// @param strData - node data string
	/// @return - return true if ok
	bool SaveString(const char* strNode, const char* strData);

	/// save a dword data into registry
	/// @param strNode - node name
	/// @param dwData - node value
	/// @return - return true if ok
	bool SaveDword(const char* strNode, DWORD dwData);

	/// delete a node in registry
	/// @param strNode - node name
	/// @return - return true if ok
	bool Delete(const char* strNode);

	/// query sub keys from registry
	/// @param szPos - key position
	/// @param strSubKey - [out]sub key name
	/// @param szLen - sub key name limit
	/// @return - return ture if queried
	bool EnumSubkey(size_t szPos, char* strSubKey, size_t szLen);

	/// query value from registry
	/// @param szPos - value position
	/// @param strKey - [out]key name
	/// @param szKeyLen - key name limit
	/// @param strValue - [out]value name
	/// @param szValueLen - value name limit
	/// @return - return true if queried
	bool EnumValue(size_t szPos, char* strKey, size_t szKeyLen,
						char* strValue, size_t szValueLen);
};

#ifdef USE_UNICODE
#undef USE_UNICODE
#define _UNICODE
#define UNICODE
#endif

#endif//_DANIEL_REGISTRYEX_H_