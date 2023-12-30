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
// Name  : ini.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/9/2005
// Desc  : ini file reading and writing
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ini.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-12 5:28p Daniel.wang
// 
// 3     5/14/05 2:58p Daniel.wang
// 
// 1     05-03-29 21:37 Daniel.wang
// ===========================================================================

#ifndef _ISA_INI_H_
#define _ISA_INI_H_

#pragma warning (disable:4786)

#include <windows.h>
#include <string>
#include <vector>

void GetStringList(const char* strStringList, size_t zLen, std::vector<std::string>& arrLists);

#define MAX_INI_SECTION_LENGTH		256
#define MAX_INI_KEY_LENGTH			256
#define MAX_INI_SECTION_COUNT		256
#define MAX_INI_KEY_COUNT			256
#define MAX_INI_VALUE_LENGTH		1024
#define MAX_INI_SECTION_LIST_LENGTH	MAX_INI_SECTION_LENGTH*MAX_INI_SECTION_COUNT
#define MAX_INI_KEY_LIST_LENGTH		MAX_INI_KEY_LENGTH*MAX_INI_KEY_COUNT

/// -----------------------------
/// IniFile
/// -----------------------------
/// Desc : ini file reading and writing 
/// @template - 
/// -----------------------------
class IniFile
{
private:
	std::string	m_strFileName;

public:
	/// constructor
	/// @param strFileName - ini file name
	/// @return - 
	IniFile(const char* strFileName = NULL);

	/// distructor
	/// @param  - 
	/// @return - 
	~IniFile(){};


	/// set ini file name 
	/// @param strFileName - ini file name
	/// @return - 
	void SetFileName(const char* strFileName);

	/// get ini file name 
	/// @param  - 
	/// @return - return the ini file name
	std::string GetFileName(void);


	/// query all the sections in ini file
	/// @param arrLists - [out] sections list
	/// @return - return ture if ok
	bool EnumSection(std::vector<std::string>& arrLists);

	/// query all the keys in a section
	/// @param strSection - the section name
	/// @param arrLists - [out] keys list
	/// @return - return ture if ok
	bool EnumKey(const char* strSection, std::vector<std::string>& arrLists);


	/// write a key into ini file
	/// @param strSection - section name
	/// @param strKey - key name
	/// @param strValue - the value of the key
	/// @return - return ture if ok
	bool WriteKey(const char* strSection, const char* strKey, const char* strValue);
	
	/// read a key from ini file
	/// @param strSection - section name
	/// @param strKey - key name
	/// @return - return the key value
	std::string ReadKey(const char* strSection, const char* strKey);


	/// delete a section in ini file
	/// @param strSection - section name
	/// @return - return ture if ok
	bool DeleteSection(const char* strSection);

	/// delete a key in a section
	/// @param strSection - section name
	/// @param strKey - the key name
	/// @return - return ture if ok
	bool DeleteKey(const char* strSection, const char* strKey);
};


#endif//_ISA_INI_H_
