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

#ifndef ZQTS_CPE_RTFVVXHELPER_H
#define ZQTS_CPE_RTFVVXHELPER_H

#include <string>
#include "ErrorCode.h"

namespace ZQTianShan 
{
namespace ContentProvision
{


class VvxWriteCB
{
public:
	virtual ~VvxWriteCB(){};
	virtual void writeVvx(char* pBuf, unsigned int nLen) = 0;
};

class RTFVVXHelper
{
public:
	RTFVVXHelper();
	~RTFVVXHelper();

	void setVvxWriteCB(VvxWriteCB* pCB);
	void setCachePath(const char* szPath);

	///<key as an unique name for it
	bool open(const char* keyname);

	// since index file is small, so int is enough
	bool write(char* pBuf, unsigned int nLen, unsigned int uOffet);

	void close();
	void getLastError(std::string& strErr, int& errCode);
private:
	bool checkVvxData();

	void setLastError(const char* strLogHeader, int nErrorCode = ERRCODE_NTFS_WRITEFILE);
	void setLastError(const std::string& strLogHeader, int nErrorCode = ERRCODE_NTFS_WRITEFILE);

#ifdef ZQ_OS_MSWIN
	HANDLE          _cacheFile;			// for index file
#else
#endif

	unsigned int	_posCacheRead;		// used for pacing, read data
	unsigned int    _vvxByteRead;

	std::string		_strErr;
	int				_nErrorCode;

	VvxWriteCB*		_pVvxWrite;	
	std::string		_strCachePath;
	std::string		_strCacheFile;
};


}
}

#endif

