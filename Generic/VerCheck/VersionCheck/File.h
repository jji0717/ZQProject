//////////////////////////////////////////////////////////////////////////
// Module: File.h
// Author: Han Guan
//////////////////////////////////////////////////////////////////////////

#ifndef _FILEINFO_H_
#define _FILEINFO_H_

#include "Directory.h"
#include <string>

class File
{
public:
	File(Directory* pDirectory,LPWIN32_FIND_DATA pFindData);
	virtual ~File();
	std::string getName();
	std::string getFullName();
	unsigned int getSize();
	std::string getCreateTime();
	std::string getVersion();

	// 功能：判断文件是否存在
	static bool isFileExist(std::string name);
protected:
	std::string m_name;
	WIN32_FIND_DATA	m_findData;
};

#endif // _FILEINFO_H_