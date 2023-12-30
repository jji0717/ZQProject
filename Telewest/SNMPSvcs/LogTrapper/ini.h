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
// ===========================================================================

#ifndef _ISA_INI_H_
#define _ISA_INI_H_

#pragma warning (disable:4786)

#include <windows.h>
#include <string>
#include <vector>

void GetStringList(const char* strStringList, size_t zLen, std::vector<std::string>& arrLists);

//i do not know how to set the limit
#define MAX_INI_LINE 1024
#define MAX_INI_SECTION 2048
#define MAX_INI_KEY 8192

/*
INI File instance
*/
class IniFile
{
private:
	std::string	m_strFileName;

public:
	//constructor and distructor
	IniFile(const char* strFileName = NULL);
	~IniFile(){};

	//set the ini file name
	void SetFileName(const char* strFileName);

	//read current ini file name
	std::string GetFileName(void);

	//[bad function name]
	//query sections' name in current ini file
	bool EnumSection(std::vector<std::string>& arrLists);

	//[bad function name]
	//query keys' name in strSection section
	bool EnumKey(const char* strSection, std::vector<std::string>& arrLists);

	//write key into ini file
	bool WriteKey(const char* strSection, const char* strKey, const char* strValue);

	//read key from ini file
	std::string ReadKey(const char* strSection, const char* strKey);

	//delete a section in ini file
	bool DeleteSection(const char* strSection);

	//delete a key in ini file
	bool DeleteKey(const char* strSection, const char* strKey);
};


#endif//_ISA_INI_H_
