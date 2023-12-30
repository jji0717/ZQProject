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
// Name  : registryex.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/9/2005
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/SNMPAlarm/registryex.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-12 2:16p Daniel.wang
// 
// 1     05-07-12 12:57p Daniel.wang
// 
// 1     05-07-11 3:33p Daniel.wang
// 
// 2     5/14/05 2:58p Daniel.wang
// 
// 1     05-03-30 14:57 Daniel.wang
// ===========================================================================

#include "registryex.h"


RegistryEx_T::RegistryEx_T(const char* strPath, RegistryEx_T::Root root)
		:m_hKey(NULL)
	{
		HKEY hRoot = (HKEY)root;
		if (ERROR_SUCCESS != RegOpenKeyEx(hRoot, strPath, 0, KEY_READ|KEY_WRITE, &m_hKey))
		{
			DWORD dwTemp;
			RegCreateKeyEx(hRoot, strPath, 0, "", 0, KEY_READ|KEY_WRITE, NULL, &m_hKey, &dwTemp);
		}
	}

	RegistryEx_T::~RegistryEx_T()
	{
		if (!IsError())
		{
			RegCloseKey(m_hKey);
		}
	}

	bool RegistryEx_T::IsError(void)
	{
		return NULL != m_hKey;
	}

	bool RegistryEx_T::Load(const char* strNode, RegistryEx_T::Type& tp,BYTE* pData, size_t zLen)
	{
		assert(strNode&&pData&&m_hKey);

		return ERROR_SUCCESS == RegQueryValueEx(m_hKey, strNode, NULL, (DWORD*)&tp, pData, (DWORD*)&zLen);
	}

	bool RegistryEx_T::LoadNoType(const char* strNode, BYTE* pData, size_t zLen)
	{
		assert(strNode&&pData&&m_hKey);

		return ERROR_SUCCESS == RegQueryValueEx(m_hKey, strNode, NULL, NULL, pData, (DWORD*)&zLen);
	}

	bool RegistryEx_T::Save(const char* strNode, RegistryEx_T::Type tp, const BYTE* pData, size_t zLen)
	{
		assert(strNode&&pData&&m_hKey);

		return ERROR_SUCCESS == RegSetValueEx(m_hKey, strNode, 0, (DWORD)tp, pData, zLen);
	}

	bool RegistryEx_T::LoadStr(const char* strNode, char* strData, size_t zLen)
	{
		return LoadNoType(strNode, (BYTE*)strData, zLen);
	}

	bool RegistryEx_T::LoadDword(const char* strNode, DWORD& dwData)
	{
		return LoadNoType(strNode, (BYTE*)&dwData, sizeof(DWORD));
	}

	bool RegistryEx_T::SaveString(const char* strNode, const char* strData)
	{
		return Save(strNode, TP_SZ, (const BYTE*)strData, strlen(strData));
	}

	bool RegistryEx_T::SaveDword(const char* strNode, DWORD dwData)
	{
		return Save(strNode, TP_DWORD_LITTLE_ENDIAN, (const BYTE*)&dwData, sizeof(DWORD));
	}

	bool RegistryEx_T::Delete(const char* strNode)
	{
		assert(strNode&&m_hKey);

		return ERROR_SUCCESS == RegDeleteValue(m_hKey, strNode);
	}

	bool RegistryEx_T::EnumSubkey(size_t szPos, char* strSubKey, size_t szLen)
	{
		assert(strSubKey);
		memset(strSubKey, 0,szLen*sizeof(char));
		return ERROR_SUCCESS == RegEnumKeyEx(m_hKey, szPos, strSubKey, (DWORD*)&szLen, 0, 0, 0, 0);
	}

	bool RegistryEx_T::EnumValue(size_t szPos, char* strKey, size_t szKeyLen,
						char* strValue, size_t szValueLen)
	{
		assert(strKey && strValue);
		memset(strKey, 0, szKeyLen*sizeof(char));
		memset(strValue, 0,szValueLen*sizeof(char));
		return ERROR_SUCCESS == RegEnumValue(m_hKey, szPos, strKey, (DWORD*)&szKeyLen, 0, 0, (BYTE*)strValue, (DWORD*)&szValueLen);
	}