// ===========================================================================
// Copyright (c) 2008 by
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
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#ifndef ZQTS_CPE__FILE_IO_H
#define ZQTS_CPE__FILE_IO_H

#include "../PT_FtpServer/utils.h"
#include <string>
#include <memory>

namespace ZQ
{
	namespace common{
		class Log;
	}
}

namespace ZQTianShan 
{
namespace ContentProvision
{


class FileIoFactory;

class FileIo
{
public:

	enum Position
	{
		POS_BEGIN,
		POS_CURRENT,
		POS_END
	};

	enum AccessMode
	{
		ACCESS_READ		= 0x00000001, // (1<<0)
		ACCESS_WRITE	= 0x00000002, // (1<<1)
		ACCESS_EXECUTE	= 0x00000004, // (1<<2)
		ACCESS_ALL		= 0x00000008, // (1<<3)
		ACCESS_APPEND   = 0x00000010, // (1<<4)
	};

	enum ShareMode
	{
		SHARE_READ		= 0x00000001,
		SHARE_WRITE     = 0x00000002,
		SHARE_DELETE    = 0x00000004  
	};

	enum CreationWay
	{
		WAY_CREATE_NEW,
		WAY_CREATE_ALWAYS,
		WAY_OPEN_EXISTING,
		WAY_OPEN_ALWAYS,
		WAY_TRUNCATE_EXISTING
	};

	enum FileAttrib
	{
		ATTRIB_NONE,
		ATTRIB_INDEXFILE	= 0x00000001,
		ATTRIB_NPVR		= 0x00000002,
	};

	FileIo(FileIoFactory* pFileIoFactory):_pFileIoFactory(pFileIoFactory)
	{
	}
	
	virtual ~FileIo(){};
	virtual ZQ::common::Log* getLog() = 0;
	virtual void setLog(ZQ::common::Log* pLog) = 0;

	virtual bool openfile(const char* szFileName, AccessMode accessMode,ShareMode shareMode,CreationWay howToCreate, FileAttrib fileAttrib) = 0;
	virtual bool setOption() = 0;
	virtual void setMainFileFalg() = 0;
	virtual bool readfile(char* pBuf, unsigned int bufLen, unsigned int& rcvLen) = 0;
	virtual bool writefile(char* pBuf, unsigned int bufLen, unsigned int& writeLen) = 0;
	virtual bool seekfile(int64 offset, Position pos) = 0;
	virtual bool setEndOfFile(int64 offset) = 0;
	virtual bool closefile() = 0;
	virtual bool enableSparse() = 0;

	// get the current open file name
	virtual std::string getFileName() = 0;

	virtual std::string getFileExtension() = 0;

	virtual void setFileExtension(const char* szFileExt) = 0;

	//bandwidth management
	virtual bool reserveBandwidth(unsigned int dwBandwidth) = 0;
	virtual void releaseBandwidth() = 0;

	virtual void getLastError(std::string& strErr, int& errCode) = 0;

protected:
	FileIoFactory*		_pFileIoFactory;
};


class FileIoFactory
{
public:

	virtual ~FileIoFactory(){};
	virtual ZQ::common::Log* getLog() = 0;
	virtual void setLog(ZQ::common::Log* pLog) = 0;

	virtual FileIo*	create() = 0;

	virtual bool deleteFile(const char* szFileName) = 0;

	virtual bool moveFile(const char* szFileName, const char* szNewFileName) = 0;

	virtual int64 getFileSize(const char* szFileName) = 0;

	//find file functions
	// 	virtual HANDLE findFirstFile(const char* szFileName) = 0;
	// 	virtual HANDLE findNext(const char* szFileName) = 0;
	// 	virtual void findClose(HANDLE hFind) = 0;
	
		//find file functions
#ifdef ZQ_OS_MSWIN
	virtual void findClose(HANDLE hfile)=0;
	virtual bool findNextFile(HANDLE hfile,WIN32_FIND_DATAA& w)=0;
	virtual HANDLE findFirstFile(char* name, WIN32_FIND_DATAA& w)=0;
#endif
	virtual int getFileStats(char *filepath, FSUtils::fileInfo_t *infoptr)=0;

	
	virtual bool initialize() = 0;
	virtual void uninitialize() = 0;

	virtual void getLastError(std::string& strErr, int& errCode) = 0;

};

}
}

#endif

