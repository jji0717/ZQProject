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


#include "FTPPropSource.h"
#include "Log.h"


#define MOLOG				(*_pLog)
#define FTPPropSrc	"FTPPropSrc"


using namespace ZQ::common;



namespace ZQTianShan {
	namespace ContentProvision {


FTPPropSource::FTPPropSource( FTPClientFactory* pFTPClientFactory, FileIoFactory* pFileIoFactory )
	:FTPPropagation(pFTPClientFactory, pFileIoFactory)
{
	_benbleResume = false;
}

void FTPPropSource::OnProgress( int64 nProcessBytes )
{
	GetGraph()->OnProgress(nProcessBytes);
}

bool FTPPropSource::Init()
{
	bool result = true;
	//call prepare
	result = FTPPropagation::prepare();
	if (!result)
	{
		std::string err;
		int errcode;
		FTPPropagation::getLastError(err,errcode);
		SetLastError(err,errcode);
	}
    GetGraph()->setTotalBytes(_totalBytes);
	return result;
}

void FTPPropSource::Stop()
{
	FTPPropagation::stop();
}

bool FTPPropSource::Run()
{
	bool result = true;
	// call propagate
	MediaInfo mInfo;
	FTPPropagation::getMediaInfo(mInfo);
	GetGraph()->OnMediaInfoParsed(mInfo);

	if (_benbleResume)
	{
		result = FTPPropagation::resumePropagate();
	}
	else
	{
		result =  FTPPropagation::propagate();
	}
	if (!result)
	{
		std::string err;
		int errcode;
		FTPPropagation::getLastError(err,errcode);
		SetLastError(err,errcode);
	}

	GetGraph()->setTotalBytes(_totalBytes);
	return result;
}

void FTPPropSource::Close()
{
	FTPPropagation::close();
}

void FTPPropSource::endOfStream()
{
}

const char* FTPPropSource::GetName()
{
	return SOURCE_TYPE_FTPPropagation;
}

bool FTPPropSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	return false;
}
bool FTPPropSource::seek(int64 offset, int pos)
{
	return false;
}

	
}}

